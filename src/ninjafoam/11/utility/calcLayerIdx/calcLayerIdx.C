#include "argList.H"
#include "Time.H"
#include "fvMesh.H"
#include "volFields.H"
#include "FaceCellWave.H"

using namespace Foam;

// Advanced OpenFOAM 11 Token tracking both step distance and its assigned Z-layer lane
class TopoToken
{
    label distance_;
    label zLayer_; // The vertical layer this token is restricted to (-1 means unrestricted)

public:
    // Core template constructors
    TopoToken() : distance_(-1), zLayer_(-1) {}
    TopoToken(const label dist, const label zLayer = -1)
        : distance_(dist), zLayer_(zLayer) {}

    label distance() const { return distance_; }
    label zLayer() const { return zLayer_; }

    bool valid(const labelList& zLayers) const { return distance_ >= 0; }

    // Serialization for parallel MPI communication pipelines
    friend Ostream& operator<<(Ostream& os, const TopoToken& t)
    {
        os << t.distance_ << token::SPACE << t.zLayer_;
        return os;
    }
    friend Istream& operator>>(Istream& is, TopoToken& t)
    {
        is >> t.distance_ >> t.zLayer_;
        return is;
    }

    // Matching functions used by the template mechanics
    bool equal(const TopoToken& rhs, const labelList& zLayers) const
    {
        return distance_ == rhs.distance_ && zLayer_ == rhs.zLayer_;
    }

    bool operator==(const TopoToken& rhs) const
    {
        return distance_ == rhs.distance_ && zLayer_ == rhs.zLayer_;
    }

    bool operator!=(const TopoToken& rhs) const
    {
        return distance_ != rhs.distance_ || zLayer_ != rhs.zLayer_;
    }

    // signature for geometric matching over cyclic boundaries
    bool sameGeometry(const polyMesh& mesh, const TopoToken& other, const scalar tol, const labelList& zLayers) const
    {
        return distance_ == other.distance_;
    }

    // Face-to-Cell Rule with TrackingData mapped to the pre-computed zLayers list
    // this is the one that matters for updating/upgrading the walk-through methods
    // Pure topological step rule: any valid face crossed adds a single unit step
    bool updateCell
    (
        const polyMesh& mesh,
        const label celli,
        const label facei,
        const TopoToken& faceInfo,
        const scalar tol,
        const labelList& zLayers
    )
    {
        // LAYER-LOCK RULE: If this is an X/Y wave (zLayer >= 0), check if the target cell
        // belongs to a different Z floor. If it does, block the wave immediately!
        if(faceInfo.zLayer_ >= 0 && zLayers[celli] != faceInfo.zLayer_)
        {
            return false;
        }

        if(distance_ < 0 || faceInfo.distance_ + 1 < distance_)
        {
            distance_ = faceInfo.distance_ + 1;
            zLayer_ = faceInfo.zLayer_;  // Keep carrying the lane configuration forward
            return true;
        }
        return false;
    }

    // Cell-to-Face Rule (5-argument variant)
    bool updateFace
    (
        const polyMesh& mesh,
        const label facei,
        const TopoToken& cellInfo,
        const scalar tol,
        const labelList& zLayers
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
            zLayer_ = cellInfo.zLayer_;  // Keep carrying the lane configuration forward
            return true;
        }
        return false;
    }

    // Cell-to-Face Rule (6-argument variant)
    bool updateFace
    (
        const polyMesh& mesh,
        const label facei,
        const label celli,
        const TopoToken& cellInfo,
        const scalar tol,
        const labelList& zLayers
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
            zLayer_ = cellInfo.zLayer_;  // Keep carrying the lane configuration forward
            return true;
        }
        return false;
    }

    // parallel transformation signature
    void transform
    (
        const polyPatch& patch,
        const label facei,
        const transformer& transform,
        const labelList& zLayers
    ) {}
};

// Reusable calculation wrapper to compute scalar topology layers along a boundary seed lane, configured to accept layer restrictions
void calculateDirectionalWave
(
    const fvMesh& mesh,
    const Time& runTime,
    const word& seedPatchName,
    const word& scalarFieldName,
    labelList& zLayers,
    const bool lockToLayers
)
{
    Info << "-> Calculating wave starting from patch: " << seedPatchName << " (Layer Locked: " << (lockToLayers ? "true" : "false") << ")" << endl;

    label patchi = mesh.boundaryMesh().findPatchID(seedPatchName);
    if(patchi < 0)
    {
        WarningInFunction
            << "Patch '" << seedPatchName << "' not found. Skipping field generation for "
            << scalarFieldName << "!" << endl;
        return;
    }

    const polyPatch& patch = mesh.boundaryMesh()[patchi];
    label totalSeedFaces = patch.size();

    // Size our seed lists to fit all faces found
    labelList changedFaces(totalSeedFaces);
    List<TopoToken> faceData(totalSeedFaces);

    // Loop back through and seed every wall face as Layer 0
    // but also to add the zLayer floorID, as either unrestricted, or to the specific zLayerIdx
    forAll(patch, facei)
    {
        label globalFacei = patch.start() + facei;
        changedFaces[facei] = globalFacei;  // Global face ID

        if(lockToLayers)
        {
            // For horizontal boundaries, identify the exact vertical layer of the interior cell it touches
            label celli = mesh.faceOwner()[globalFacei];
            label localZ = zLayers[celli];
            faceData[facei] = TopoToken(0, localZ); // Seed layer 0 , also tag this token with its specific floor ID
        }
        else
        {
            faceData[facei] = TopoToken(0, -1);    // Seed layer 0, also Z-wave runs completely unrestricted
        }
    }

    // Allocate tracking structures across the grid size
    List<TopoToken> allFaceInfo(mesh.nFaces());
    List<TopoToken> allCellInfo(mesh.nCells());

    // Fire the FaceCellWave pipeline for the chosen grid alignment
    FaceCellWave<TopoToken, labelList> wave
    (
        mesh,
        changedFaces,
        faceData,
        allFaceInfo,
        allCellInfo,
        mesh.nCells(),
        zLayers
    );

    // Initialize/Declare the output volume scalar field
    volScalarField layerIdx
    (
        IOobject
        (
            scalarFieldName,
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("default", dimless, -1.0)
    );

    // Populate cell values (Internal Field)
    forAll(allCellInfo, celli)
    {
        if(allCellInfo[celli].valid(zLayers))
        {
            layerIdx[celli] = allCellInfo[celli].distance();
        }
    }

    // Explicitly map topological steps onto the boundary patch faces
    forAll(mesh.boundaryMesh(), patchi)
    {
        const fvPatch& patch = mesh.boundary()[patchi];
        fvPatchScalarField& boundaryField = layerIdx.boundaryFieldRef()[patchi];

        forAll(patch, facei)
        {
            label globalFacei = patch.start() + facei;
            if(allFaceInfo[globalFacei].valid(zLayers))
            {
                boundaryField[facei] = allFaceInfo[globalFacei].distance();
            }
        }
    }

    // Write file to disk
    layerIdx.write();
    Info << "   Successfully calculated and wrote '" << scalarFieldName << "'." << endl;
}

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    Info << "\n--- Running Multi-Directional Layer-Isolated Topological FaceCellWave ---" << endl;

    // Allocate an empty placeholder list for the first vertical wave pass
    labelList dummyLayers(mesh.nCells(), -1);

    // 1. Z-Direction pass: Generates the foundational layout field
    calculateDirectionalWave(mesh, runTime, "minZ", "zLayerIdx", dummyLayers, false);

    // Re-read or extract the fresh zLayerIdx field to use as our layout restrictions blueprint
    Info << "-> Caching topological Z-Layers blueprint for horizontal routing constraints..." << endl;
    volScalarField zLayerIdx(IOobject("zLayerIdx", runTime.timeName(), mesh, IOobject::MUST_READ, IOobject::NO_WRITE), mesh);

    labelList activeCellZLayers(mesh.nCells());
    forAll(activeCellZLayers, celli)
    {
        activeCellZLayers[celli] = std::round(zLayerIdx[celli]);
    }

    // 2. X-Direction pass: Completely locked inside its respective horizontal z-levels
    calculateDirectionalWave(mesh, runTime, "west_face", "xLayerIdx", activeCellZLayers, true);

    // 3. Y-Direction pass: Completely locked inside its respective horizontal z-levels
    calculateDirectionalWave(mesh, runTime, "south_face", "yLayerIdx", activeCellZLayers, true);

    Info << "\nExecution complete. Isolated horizontal coordinates mapped successfully.\n" << endl;
    return 0;
}
