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
    const volVectorField& cellTilt;

    WaveTrackingData(const WaveDir d, const volVectorField& cellTiltField)
    :
        dir(d),
        cellTilt(cellTiltField)
    {}
};

// Advanced OpenFOAM 11 FaceCellWave tracking token both that tracks step distance, using a struct of additional information like the wave-direction and the computed cellTilt values
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
    // where the travelVector is tilt corrected using the cellTilt of the minZ face of the cell
    //// worked for missoula_valley.tif, flat_test.tif, and big_butte_small.tif, but breaks down for bell_steep.tif where cells tilt more than 45 degrees, or have extra strong )) and (( style distortions. Still breaks for bell_steep.tif even with minZ cellTilt, whatever cellTilt form is used, seems cross cell tilt information might be needed too, or cell distortion information, or it just isn't quite enough to pull out the travelVect vs the waveDirection.
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

        /*//// first attempt, an attempt at building a tilted 3D rotation matrix, for a projection style calculation. this form of the calculations seems a bit weird and quirky
        // build local terrain-aligned axes from cellTilt
        const vector& localZ = td.cellTilt[celli];
        vector refX(1, 0, 0);
        if(mag(localZ & refX) > 0.9)
        {
            refX = vector(0, 1, 0);
        }
        const vector localY = normalised(localZ ^ refX);
        const vector localX = localY ^ localZ;

        // project travelVec onto local terrain axes
        const scalar dX = travelVec & localX;
        const scalar dY = travelVec & localY;
        const scalar dZ = travelVec & localZ;

        // get the magnitude along each axis
        const scalar magX = mag(dX);
        const scalar magY = mag(dY);
        const scalar magZ = mag(dZ);*/

        // totate travelVec by the exact 3D rotation that aligns cellTilt -> (0, 0, 1)
        // (Rodrigues' rotation formula: rotates "down and over" with zero azimuthal twist)
        const vector& n = td.cellTilt[celli];
        const vector up(0, 0, 1);

        const vector rotAxis = n ^ up;          // Cross product gives rotation axis & sin(theta)
        const scalar cosTheta = n & up;         // Dot product gives cos(theta)
        const scalar sinThetaSq = magSqr(rotAxis);

        vector flatTravelVec = travelVec;
        if(sinThetaSq > SMALL)
        {
            // Standard 3D vector rotation from n to (0, 0, 1)
            flatTravelVec =
                travelVec * cosTheta
              + (rotAxis ^ travelVec)
              + rotAxis * ((rotAxis & travelVec) / (1.0 + cosTheta));
        }

        // get the magnitude along each axis
        const scalar magX = mag(flatTravelVec.x());
        const scalar magY = mag(flatTravelVec.y());
        const scalar magZ = mag(flatTravelVec.z());

        /*//// the full rotation tensor form, let OpenFOAM make the 3D tilted coordinate axes needed to rotate to the ground, instead of building it manually
        //// hrm, all three forms ended up looking the same, I suspect that cellTilt of just minZ is not enough of a tilt correction, or something else is missing.
        //// Hrm. Lots of )( style cells in bell_steep.tif, could just be that it needs more info about the sides, some kind of cross tilt or cell distortion as well. Hrm.
        // normalize the cellTilt, and setup the unit cartesian coordinate ground vector to rotate towards
        const vector n = normalised(td.cellTilt[celli]);
        const vector up(0, 0, 1);

        // build the exact rotation that tips 'n' straight up to (0, 0, 1)
        const tensor unTilt = rotationTensor(n, up);

        // rotate travelVec onto flat ground
        vector flatVec = unTilt & travelVec;

        // get the magnitude along each axis
        const scalar magX = mag(flatVec.x());
        const scalar magY = mag(flatVec.y());
        const scalar magZ = mag(flatVec.z());*/

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

// Reusable calculation wrapper to compute scalar topology layers along a boundary seed lane, configured to pass in a struct of tracking data
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

volVectorField calcCellTilt(const fvMesh& mesh, const Time& runTime)
{
    volVectorField cellTilt
    (
        IOobject
        (
            "cellTilt",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector("up", dimless, vector(0, 0, 1))
    );

    const vectorField& Sf = mesh.faceAreas();
    const labelList& own = mesh.faceOwner();

    forAll(mesh.cells(), celli)
    {
        const cell& c = mesh.cells()[celli];

        // Find the face pointing most downward (-Z) for this cell
        scalar mostDownwardRatio = 0.0;
        vector bottomUpNormal = vector(0, 0, 1);

        forAll(c, fI)
        {
            label facei = c[fI];
            vector outwardSf = Sf[facei];
            if (own[facei] != celli)
            {
                outwardSf = -outwardSf;
            }

            scalar area = mag(outwardSf);
            if (area < SMALL) continue;

            // Cosine of angle from +Z (will be negative for bottom face, e.g. -0.58 at 54 deg)
            scalar zCos = outwardSf.z() / area;

            if (zCos < mostDownwardRatio)
            {
                mostDownwardRatio = zCos;
                bottomUpNormal = -outwardSf / area; // Flip outward (-Z) to upward (+Z) unit normal
            }
        }

        cellTilt[celli] = bottomUpNormal;
    }

    return cellTilt;
}

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    Info << "\n--- Running Multi-Directional Topological FaceCellWave ---" << endl;

    // pre-calculate cellTilt once for the entire mesh
    volVectorField cellTilt = calcCellTilt(mesh, runTime);

    // build lightweight TrackingData structs for each directional wave (zero-copy reference to cellGradients)
    WaveTrackingData td_Z(WaveDir::Z, cellTilt);
    WaveTrackingData td_X(WaveDir::X, cellTilt);
    WaveTrackingData td_Y(WaveDir::Y, cellTilt);

    // run each directional wave
    calculateDirectionalWave(mesh, runTime, "minZ", "zLayerIdx", td_Z);
    calculateDirectionalWave(mesh, runTime, "west_face", "xLayerIdx", td_X);
    calculateDirectionalWave(mesh, runTime, "south_face", "yLayerIdx", td_Y);

    Info << "\nExecution complete. All scalar grid coordinates calculated successfully.\n" << endl;
    return 0;
}
