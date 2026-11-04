#include "argList.H"
#include "Time.H"
#include "fvMesh.H"
#include "volFields.H"
#include "FaceCellWave.H"

using namespace Foam;

// Define an exact, structurally compliant OpenFOAM 11 FaceCellWave token
class TopoToken
{
    label distance_;

public:
    // Core template constructors
    TopoToken() : distance_(-1) {}
    TopoToken(const label dist) : distance_(dist) {}

    label distance() const { return distance_; }
    bool valid(const int td) const { return distance_ >= 0; }

    // Serialization for communication pipelines
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
    bool equal(const TopoToken& rhs, const int td) const
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
    bool sameGeometry(const polyMesh& mesh, const TopoToken& other, const scalar tol, const int td) const
    {
        return distance_ == other.distance_;
    }

    // Face-to-Cell Rule
    // this is the one that matters for updating/upgrading the walk-through methods
    bool updateCell
    (
        const polyMesh& mesh,
        const label celli,
        const label facei,
        const TopoToken& faceInfo,
        const scalar tol,
        const int td
    )
    {
        if(distance_ < 0 || faceInfo.distance_ + 1 < distance_)
        {
            distance_ = faceInfo.distance_ + 1;
            return true;
        }
        return false;
    }

    // Cell-to-Face Rule
    bool updateFace
    (
        const polyMesh& mesh,
        const label facei,
        const TopoToken& cellInfo,
        const scalar tol,
        const int td
    )
    {
        if(distance_ < 0 || cellInfo.distance_ < distance_)
        {
            distance_ = cellInfo.distance_;
            return true;
        }
        return false;
    }

    // Cell-to-Face Rule
    bool updateFace
    (
        const polyMesh& mesh,
        const label facei,
        const label celli,
        const TopoToken& cellInfo,
        const scalar tol,
        const int td
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
        const int td
    ) {}
};

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    Info << "\n--- Running Topological FaceCellWave ---" << endl;

    // Dynamically count how many faces belong to ALL wall patches combined
    label totalWallFaces = 0;
    forAll(mesh.boundaryMesh(), patchi)
    {
        const polyPatch& patch = mesh.boundaryMesh()[patchi];
        if(patch.type() == "wall")
        {
            totalWallFaces += patch.size();
        }
    }

    if(totalWallFaces == 0)
    {
        FatalErrorInFunction
            << "No patches of type 'wall' were found in polyMesh/boundary!"
            << abort(FatalError);
    }

    // Size our seed lists to fit all wall faces found
    labelList changedFaces(totalWallFaces);
    List<TopoToken> faceData(totalWallFaces);

    // Loop back through and seed every wall face as Layer 0
    label seedi = 0;
    forAll(mesh.boundaryMesh(), patchi)
    {
        const polyPatch& patch = mesh.boundaryMesh()[patchi];
        if(patch.type() == "wall")
        {
            Info << "  Seeding wall patch: " << patch.name() << " (" << patch.size() << " faces)" << endl;
            forAll(patch, facei)
            {
                changedFaces[seedi] = patch.start() + facei; // Global face ID
                faceData[seedi] = TopoToken(0);              // Layer 0
                seedi++;
            }
        }
    }

    // Allocate tracking structures across the grid size
    List<TopoToken> allFaceInfo(mesh.nFaces());
    List<TopoToken> allCellInfo(mesh.nCells());

    // Tracking data default primitive
    int td = 0;

    // Fire the FaceCellWave pipeline
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

    // Declare the output volume field
    volScalarField zLayerIdx
    (
        IOobject
        (
            "zLayerIdx",
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
            zLayerIdx[celli] = allCellInfo[celli].distance();
        }
    }

    // Explicitly map topological steps onto boundary faces
    forAll(mesh.boundaryMesh(), patchi)
    {
        const fvPatch& patch = mesh.boundary()[patchi];
        fvPatchScalarField& boundaryField = zLayerIdx.boundaryFieldRef()[patchi];

        forAll(patch, facei)
        {
            label globalFacei = patch.start() + facei;
            if(allFaceInfo[globalFacei].valid(td))
            {
                boundaryField[facei] = allFaceInfo[globalFacei].distance();
            }
        }
    }

    // Write results to your time steps
    zLayerIdx.write();
    Info << "Successfully calculated and wrote 'zLayerIdx' to time folder.\n" << endl;

    return 0;
}
