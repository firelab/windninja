#include "argList.H"
#include "Time.H"
#include "fvMesh.H"
#include "volFields.H"
#include "FaceCellWave.H"

using namespace Foam;

// Advanced OpenFOAM 11 FaceCellWave tracking token that tracks step distance, using the faceNormal of the last faceToCell wave that successfully passed through, seeded with the faceNormal of the starting patch faces
class TopoToken
{
    //- Topological step distance from seed patch (-1 = unvisited)
    label distance_;

    //- Unit normal of the last valid face crossed (pointing in wave direction)
    vector waveNormal_; // Tracked normal passed from seed face -> cell -> next cell

public:
    // Core template constructors
    TopoToken() : distance_(-1), waveNormal_(vector::zero) {}
    TopoToken(const label dist, const vector& norm = vector::zero)
        : distance_(dist), waveNormal_(norm) {}

    label distance() const { return distance_; }
    const vector& waveNormal() const { return waveNormal_; }

    bool valid(int& td) const { return distance_ >= 0; }

    // Serialization for parallel MPI communication pipelines
    friend Ostream& operator<<(Ostream& os, const TopoToken& t)
    {
        os << t.distance_ << token::SPACE << t.waveNormal_;
        return os;
    }
    friend Istream& operator>>(Istream& is, TopoToken& t)
    {
        is >> t.distance_ >> t.waveNormal_;
        return is;
    }

    // Matching functions used by the template mechanics
    bool equal(const TopoToken& rhs, int& td) const
    {
        return distance_ == rhs.distance_  && waveNormal_ == rhs.waveNormal_;
    }

    bool operator==(const TopoToken& rhs) const
    {
        return distance_ == rhs.distance_  && waveNormal_ == rhs.waveNormal_;
    }

    bool operator!=(const TopoToken& rhs) const
    {
        return distance_ != rhs.distance_ || waveNormal_ != rhs.waveNormal_;
    }

    // signature for geometric matching over cyclic boundaries
    bool sameGeometry(const polyMesh& mesh, const TopoToken& other, const scalar tol, int& td) const
    {
        return distance_ == other.distance_;
    }

    // Face-to-Cell Rule
    // this is the one that matters for updating/upgrading the walk-through methods
    // check alignment of the incoming waveNormal_/faceNormal wave path, with the waveNormal_/faceNormal of the last valid wave path,
    // using the seeded faceNormal values of the starting patch faces as the starting position and direction of the wave path.
    bool updateCell
    (
        const polyMesh& mesh,
        const label celli,
        const label facei,
        const TopoToken& faceInfo,
        const scalar tol,
        int& td
    )
    {
        // the face normal, of the face the wave just passed through
        // get unit normal of facei
        vector faceNorm = mesh.faceAreas()[facei];
        const scalar area = mag(faceNorm);

        // guard against zero-sized/small face areas
        if(area < VSMALL)
        {
            return false;
        }

        // normalize the face normal vector to get the true unit directional normal
        faceNorm /= area;

        // OpenFOAM face normals point OUT of the owner cell and INTO the neighbour cell.
        // flip sign if celli is the owner so faceNormal always points INWARD into celli (matching -faceNormal seed)
        if(mesh.faceOwner()[facei] == celli)
        {
            faceNorm = -faceNorm;
        }

        // check to make sure that the past waveNormal_/faceNorm matches the current one
        // not aligned close enough, represents a perpendicular side wall, ignore
        if((faceNorm & faceInfo.waveNormal_) <= 0.5)
        {
            return false;
        }

        // standard topological distance update
        // reached only if it clears the waveNormal_ alignment check
        // also updates waveNormal_ with faceNormal for the next cell
        if(distance_ < 0 || faceInfo.distance_ + 1 < distance_)
        {
            distance_ = faceInfo.distance_ + 1;
            waveNormal_ = faceNorm;
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
        int& td
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
            waveNormal_ = cellInfo.waveNormal_;
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
        int& td
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
            waveNormal_ = cellInfo.waveNormal_;
            return true;
        }
        return false;
    }

    // parallel transformation signature
    // Rotate directional vector across coupled/cyclic patches
    void transform
    (
        const polyPatch& patch,
        const label facei,
        const transformer& transform,
        int& td
    )
    {
        waveNormal_ = transform.transform(waveNormal_);
    }
};

// Reusable calculation wrapper to compute scalar topology layers along a boundary seed lane
void calculateDirectionalWave
(
    const fvMesh& mesh,
    const Time& runTime,
    const word& seedPatchName,
    const word& scalarFieldName
)
{
    Info << "-> Calculating wave starting from patch: " << seedPatchName << endl;

    label patchi = mesh.boundaryMesh().findPatchID(seedPatchName);
    if(patchi < 0)
    {
        WarningInFunction
            << "Patch '" << seedPatchName << "' not found. Skipping field generation for "
            << scalarFieldName << "!" << endl;
        return;
    }

    const polyPatch& patch = mesh.boundaryMesh()[patchi];

    // Size our seed lists to fit all faces found
    labelList changedFaces(patch.size());
    List<TopoToken> faceData(patch.size());

    // Loop back through and seed every face of the starting patch as LayerIdx 0
    // also seed the waveNormal_/faceNormal value for the given face
    forAll(patch, facei)
    {
        const label globalFacei = patch.start() + facei;  // Global face index

        vector faceNorm = mesh.faceAreas()[globalFacei];  // the faceNormal of the given face
        faceNorm /= (mag(faceNorm) + VSMALL);  // make sure it is normalized

        // Boundary face normals point OUT of the domain; negate (-faceNorm) so waveNormal points INWARD
        faceNorm = -faceNorm;

        changedFaces[facei] = globalFacei;
        faceData[facei] = TopoToken(0, faceNorm);  // Seed LayerIdx 0, and the waveNormal_/faceNormal of the face
    }

    // Allocate tracking structures across the grid size
    List<TopoToken> allFaceInfo(mesh.nFaces());
    List<TopoToken> allCellInfo(mesh.nCells());

    // Tracking data default primitive
    int td = 0;

    // Fire the FaceCellWave pipeline for the chosen grid alignment
    FaceCellWave<TopoToken> wave
    (
        mesh,
        changedFaces,
        faceData,
        allFaceInfo,
        allCellInfo,
        mesh.nCells(),
        td
    );

    // Initialize the output volume scalar field
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

    // Map the internalField cell distances
    forAll(allCellInfo, celli)
    {
        if(allCellInfo[celli].valid(td))
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
            if(allFaceInfo[globalFacei].valid(td))
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

    Info << "\n--- Running Multi-Directional Topological FaceCellWave ---" << endl;

    // run each directional wave
    calculateDirectionalWave(mesh, runTime, "minZ", "zLayerIdx");
    calculateDirectionalWave(mesh, runTime, "west_face", "xLayerIdx");
    calculateDirectionalWave(mesh, runTime, "south_face", "yLayerIdx");

    Info << "\nExecution complete. All scalar grid coordinates calculated successfully.\n" << endl;
    return 0;
}
