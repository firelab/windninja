#include "argList.H"
#include "Time.H"
#include "fvMesh.H"
#include "volFields.H"
#include "FaceCellWave.H"

using namespace Foam;

enum class WaveDir : int
{
    X = 0,
    Y = 1,
    Z = 2
};

// Lightweight struct passed as TrackingData (td)
struct WaveTrackingData
{
    WaveDir dir;

    WaveTrackingData(const WaveDir d)
    :
        dir(d)
    {}
};

// Advanced OpenFOAM 11 FaceCellWave tracking token both that tracks step distance, using a struct of additional information like the wave-direction
class TopoToken
{
    label distance_;

public:
    // Core template constructors
    TopoToken() : distance_(-1) {}
    TopoToken(const label dist) : distance_(dist) {}

    label distance() const { return distance_; }

    bool valid(const WaveTrackingData& td) const { return distance_ >= 0; }

    // Serialization for parallel MPI communication pipelines
    friend Ostream& operator<<(Ostream& os, const TopoToken& t)
    {
        os << t.distance_;
        return os;
    }
    friend Istream& operator>>(Istream& is, TopoToken& t)
    {
        is >> t.distance_;
        return is;
    }

    // Matching functions used by the template mechanics
    bool equal(const TopoToken& rhs, const WaveTrackingData& td) const
    {
        return distance_ == rhs.distance_;
    }

    bool operator==(const TopoToken& rhs) const
    {
        return distance_ == rhs.distance_;
    }

    bool operator!=(const TopoToken& rhs) const
    {
        return distance_ != rhs.distance_;
    }

    // signature for geometric matching over cyclic boundaries
    bool sameGeometry(const polyMesh& mesh, const TopoToken& other, const scalar tol, const WaveTrackingData& td) const
    {
        return distance_ == other.distance_;
    }

    // Face-to-Cell Rule
    // this is the one that matters for updating/upgrading the walk-through methods
    // simplest travelVector based step rule: only a valid step if the largest component of the travelVector is in the direction of the meshWave
    //// worked for missoula_valley.tif, flat_test.tif, and big_butte_small.tif, but breaks down for bell_steep.tif where cells tilt more than 45 degrees, or have extra strong )) and (( style distortions
    bool updateCell
    (
        const polyMesh& mesh,
        const label celli,
        const label facei,
        const TopoToken& faceInfo,
        const scalar tol,
        const WaveTrackingData& td
    )
    {
        const point& faceCtr = mesh.faceCentres()[facei];
        const point& cellCtr = mesh.cellCentres()[celli];

        // the physical path the wave just took, the step vector from face center to cell center
        vector travelVec = cellCtr - faceCtr;
        const scalar dist = mag(travelVec);

        // guard against zero-length vectors
        if(dist < SMALL)
        {
            return false;
        }

        // normalize to get the unit direction vector
        travelVec /= dist;

        // get the magnitude along each axis
        const scalar magX = mag(travelVec.x());
        const scalar magY = mag(travelVec.y());
        const scalar magZ = mag(travelVec.z());

        // check to make sure that the largest component is in the meshWave direction
        bool isAligned = false;
        if(td.dir == WaveDir::X)
        {
            isAligned = (magX > magY) && (magX > magZ);
        }
        else if(td.dir == WaveDir::Y)
        {
            isAligned = (magY > magX) && (magY > magZ);
        }
        else if(td.dir == WaveDir::Z)
        {
            isAligned = (magZ > magX) && (magZ > magY);
        }

        if(!isAligned)
        {
            return false;
        }

        // standard topological distance update
        // reached only if it clears the meshWave direction check
        if(distance_ < 0 || faceInfo.distance_ + 1 < distance_)
        {
            distance_ = faceInfo.distance_ + 1;
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
        const WaveTrackingData& td
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
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
        const WaveTrackingData& td
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
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
        const WaveTrackingData& td
    ) {}
};

// Reusable calculation wrapper to compute scalar topology layers along a boundary seed lane, configured to accept layer restrictions
void calculateDirectionalWave
(
    const fvMesh& mesh,
    const Time& runTime,
    const word& seedPatchName,
    const word& scalarFieldName,
    WaveTrackingData& td   // Passed by non-const reference into FaceCellWave
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
    forAll(patch, facei)
    {
        changedFaces[facei] = patch.start() + facei; // Global face index
        faceData[facei] = TopoToken(0);              // Seed LayerIdx 0
    }

    // Allocate tracking structures across the grid size
    List<TopoToken> allFaceInfo(mesh.nFaces());
    List<TopoToken> allCellInfo(mesh.nCells());

    // Fire the FaceCellWave pipeline for the chosen grid alignment
    FaceCellWave<TopoToken, WaveTrackingData> wave
    (
        mesh,
        changedFaces,
        faceData,
        allFaceInfo,
        allCellInfo,
        mesh.nCells(),
        td
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

    // build lightweight TrackingData structs for each directional wave
    WaveTrackingData td_Z(WaveDir::Z);
    WaveTrackingData td_X(WaveDir::X);
    WaveTrackingData td_Y(WaveDir::Y);

    // run each directional wave
    calculateDirectionalWave(mesh, runTime, "minZ", "zLayerIdx", td_Z);
    calculateDirectionalWave(mesh, runTime, "west_face", "xLayerIdx", td_X);
    calculateDirectionalWave(mesh, runTime, "south_face", "yLayerIdx", td_Y);

    Info << "\nExecution complete. All scalar grid coordinates mapped successfully.\n" << endl;
    return 0;
}
