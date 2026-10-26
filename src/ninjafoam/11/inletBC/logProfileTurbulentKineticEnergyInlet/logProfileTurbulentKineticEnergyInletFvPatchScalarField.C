/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | 
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "logProfileTurbulentKineticEnergyInletFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"
#include "surfaceFields.H"
#include <map>
#include "Function1.H"
#include "Table.H"

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

//- set all the various height values specific for each cell of the domain
void Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::setTerrainHeights()
{
    const pointField& faceCenters = patch().Cf();
    relativeHeight_.setSize(faceCenters.size(), 0.0);

    label minZPatchID = patch().boundaryMesh().findPatchID("minZ");
    if(minZPatchID == -1)
    {
        FatalErrorIn("logProfileTurbulentKineticEnergyInletFvPatchScalarField")
            << "Could not find a patch named 'minZ' in the mesh"
            << abort(FatalError);
    }
    const fvPatch& minZPatch = patch().boundaryMesh()[minZPatchID];
    const pointField& minZ_points = minZPatch.patch().localPoints();

    const word& curPatchName = patch().name();
    direction normDir = vector::X;
    direction tanDir = vector::Y;
    if(curPatchName == "west_face" || curPatchName == "east_face")
    {
        // inlet points in the x direction
        normDir = vector::X;
        tanDir = vector::Y;
    }
    else if(curPatchName == "north_face" || curPatchName == "south_face")
    {
        // inlet points in the y direction
        normDir = vector::Y;
        tanDir = vector::X;
    }
    else
    {
        FatalErrorIn("logProfileTurbulentKineticEnergyInletFvPatchScalarField")
            << "Boundary condition applied on incorrect patch"
            << abort(FatalError);
    }

    scalar patchNormPos = -GREAT;
    if(faceCenters.size() > 0)
    {
        patchNormPos = faceCenters[0].component(normDir);  // the x or y common position, of the patch, to compare against
    }
    if(Pstream::parRun())
    {
        // safely get the value from the other processors, for those with no faceCenters,
        // because valid coords will be always be greater than -GREAT
        reduce(patchNormPos, maxOp<scalar>());
    }

    scalar tolerance = 0.1;  // search distance away from the inlet patch, of the minZ patch
    scalarField localPoints_tanPos(minZ_points.size());
    scalarField localPoints_zPos(minZ_points.size());
    label localPoints_count = 0;
    forAll(minZ_points, pointI)
    {
        // search by the normal coordinate
        if(abs(minZ_points[pointI].component(normDir) - patchNormPos) < tolerance)
        {
            localPoints_tanPos[localPoints_count] = minZ_points[pointI].component(tanDir);
            localPoints_zPos[localPoints_count] = minZ_points[pointI].z();
            localPoints_count++;
        }
    }
    localPoints_tanPos.setSize(localPoints_count);
    localPoints_zPos.setSize(localPoints_count);

    // gather (parallel communication) each local processor minZ 1D line list of points into one big list of lists
    List<scalarField> gatheredPoints_tanPos;
    List<scalarField> gatheredPoints_zPos;
    if(Pstream::parRun())
    {
        gatheredPoints_tanPos.setSize(Pstream::nProcs());
        gatheredPoints_zPos.setSize(Pstream::nProcs());

        gatheredPoints_tanPos[Pstream::myProcNo()] = localPoints_tanPos;
        gatheredPoints_zPos[Pstream::myProcNo()] = localPoints_zPos;

        Pstream::gatherList(gatheredPoints_tanPos);
        Pstream::gatherList(gatheredPoints_zPos);
    }
    else
    {
        // sets up a single full list of points for when not a parallel run
        gatheredPoints_tanPos.setSize(1);
        gatheredPoints_zPos.setSize(1);
        gatheredPoints_tanPos[0] = localPoints_tanPos;
        gatheredPoints_zPos[0] = localPoints_zPos;
    }

    // master node sorts and compiles the global minZ 1D line list of points
    // into a profile map (std::map also sorts the points)
    // notice this also is setup such that it works for non-parallel processes,
    // just moves around the single full list, sorting the minZ 1D line list of points
    scalarField globalFlat_tanPos;
    scalarField globalFlat_zPos;
    if(Pstream::master())
    {
        std::map<scalar, scalar> globalGroundProfileMap;

        forAll(gatheredPoints_tanPos, procI)
        {
            forAll(gatheredPoints_tanPos[procI], itemI)
            {
                // Map automatically handles sorting by the tangential coordinate
                const scalar current_tanPos = gatheredPoints_tanPos[procI][itemI];
                const scalar current_zPos = gatheredPoints_zPos[procI][itemI];
                globalGroundProfileMap[current_tanPos] = current_zPos;
            }
        }

        if(globalGroundProfileMap.empty())
        {
            FatalErrorIn("logProfileTurbulentKineticEnergyInletFvPatchScalarField")
                << "No matching minZ faces found near the inlet patch across any processors"
                << abort(FatalError);
        }

        // Flatten the map back out into simple fields
        globalFlat_tanPos.setSize(globalGroundProfileMap.size());
        globalFlat_zPos.setSize(globalGroundProfileMap.size());
        label idx = 0;
        for(const std::pair<const scalar, scalar>& pair : globalGroundProfileMap)
        {
            globalFlat_tanPos[idx] = pair.first;
            globalFlat_zPos[idx] = pair.second;
            idx++;
        }
    }

    // scatter (parallel communication) the full sorted list of minZ 1D line points,
    // to each separate processor, for use in the final calculations
    // note the sizing stuff, the size was set above for Pstream::master() but not yet for the rest of the threads
    // Pstream::scatter() behaves like a fence, it takes the Pstream::master() values and spreads them to the rest of the threads
    if(Pstream::parRun())
    {
        // Broadcast the final unified size from Master so slaves can allocate memory
        label globalSize = globalFlat_tanPos.size();  // slaves get a value of 0, master gets the proper value
        Pstream::scatter(globalSize);  // spread the value from master to slaves, with a fence

        // Slaves must resize their local receivers to match the master's size
        if(!Pstream::master())
        {
            globalFlat_tanPos.setSize(globalSize);
            globalFlat_zPos.setSize(globalSize);
        }

        // Now send the actual data, with a fence
        Pstream::scatter(globalFlat_tanPos);
        Pstream::scatter(globalFlat_zPos);
    }

    // skip the calculations if no faceCenters to act on
    // still needed the above just in case no faceCenters, but still some minZ 1D line points on the given processor domain
    if(faceCenters.empty())
    {
        return;
    }

    // Repack into Tuple2 format for the interpolation Table constructor
    List<Tuple2<scalar, scalar>> globalTableEntries(globalFlat_tanPos.size());
    forAll(globalTableEntries, i)
    {
        globalTableEntries[i] = Tuple2<scalar, scalar>(globalFlat_tanPos[i], globalFlat_zPos[i]);
    }

    // Set out-of-bounds behavior to CLAMP (retains edge values if inlet exceeds minZ bounds)
    dictionary tableDict;
    tableDict.add("values", globalTableEntries);
    tableDict.add("outOfBounds", "clamp");
    Foam::Function1s::Table<scalar> zGroundTable("zGroundTable", tableDict);

    forAll(faceCenters, faceI)
    {
        // do the calculation in the tangential coordinate direction
        scalar current_coord = faceCenters[faceI].component(tanDir);
        scalar z_ground = zGroundTable.value(current_coord);

        scalar z_face = faceCenters[faceI].z();
        scalar z_AGL = z_face - z_ground;

        relativeHeight_[faceI] = z_AGL;
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

//- Construct from patch and internal field
Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::
logProfileTurbulentKineticEnergyInletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    UfreeStream_(0.0),
    uDirection_(0.0,0.0,0.0),
    inputWindHeight_Veg_(0),
    z0_(0),
    Rd_(0),
    relativeHeight_(0),
    firstCellHeight_(0)
{}

//- Construct from patch, internal field and dictionary
Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::
logProfileTurbulentKineticEnergyInletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF, dict, false),
    UfreeStream_(readScalar(dict.lookup("UfreeStream"))),
    uDirection_(dict.lookup("uDirection")),
    inputWindHeight_Veg_(readScalar(dict.lookup("inputWindHeight_Veg"))),
    z0_(readScalar(dict.lookup("z0"))),
    Rd_(readScalar(dict.lookup("Rd"))),
    relativeHeight_(0),
    firstCellHeight_(readScalar(dict.lookup("firstCellHeight")))
{
    // Get the name of the active database directory level (e.g., "processor1" or the root NINJAFOAM dir)
    word localDirName = this->db().time().path().name();
    // if inside a "processor" dir, but not running in parallel, we are guaranteed to be inside an isolated reconstructPar sub-mesh loop
    bool isReconstructParMode = (!Foam::Pstream::parRun() && std::string(localDirName).rfind("processor", 0) == 0);
    if(isReconstructParMode)
    {
        if(dict.found("relativeHeights"))
        {
            relativeHeight_ = scalarField("relativeHeights", dict, patch().size());
        }
        else
        {
            FatalErrorIn("logProfileTurbulentKineticEnergyInletFvPatchScalarField")
                << "cannot read relativeHeights entry, during reconstructPar"
                << abort(FatalError);
        }
    }
    else
    {
        setTerrainHeights();
    }

    evaluate();
}

//- Construct by mapping given logProfileTurbulentKineticEnergyInletFvPatchScalarField
//  onto a new patch
Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::
logProfileTurbulentKineticEnergyInletFvPatchScalarField
(
    const logProfileTurbulentKineticEnergyInletFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(mapper(ptf.relativeHeight_)),
    firstCellHeight_(ptf.firstCellHeight_)
{}

void Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::map
(
    const fvPatchField<scalar>& ptf,
    const fvPatchFieldMapper& mapper
)
{
    fixedValueFvPatchScalarField::map(ptf, mapper);

    const logProfileTurbulentKineticEnergyInletFvPatchScalarField& logBC =
        refCast<const logProfileTurbulentKineticEnergyInletFvPatchScalarField>(ptf);

    mapper(relativeHeight_, logBC.relativeHeight_);
}

/*//- Copy constructor
Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::
logProfileTurbulentKineticEnergyInletFvPatchScalarField
(
    const logProfileTurbulentKineticEnergyInletFvPatchScalarField& ptf
)
:
    fixedValueFvPatchScalarField(ptf),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}*/

//- Construct and return a clone
// done in the .H file

//- Copy constructor setting internal field reference
Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::
logProfileTurbulentKineticEnergyInletFvPatchScalarField
(
    const logProfileTurbulentKineticEnergyInletFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(ptf, iF),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}

//- Construct and return a clone setting internal field reference
// done in the .H file

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//- Update the coefficients associated with the patch field
void Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    // Caluculate Input Wind Height
    scalarField Kp(patch().Cf().size(), scalar(0));

    //const RASModel& rasModel = db().lookupObject<RASModel>("RASProperties");
    //const incompressible::RASModel& rasModel = db().lookupObject<incompressible::RASModel>("RASProperties");

    //const scalar Cmu = rasModel.coeffDict().lookupOrDefault<scalar>("Cmu", 0.09);
    //scalar Cmu = readScalar(rasModel.coeffDict().lookup("Cmu"));
    //scalar kappa = rasModel.kappa().value();
    //scalar Cmu = 0.09;  // original, this is for regular k epsilon models
    scalar Cmu = 0.085;  // for RNG k epsilon models

    scalar ustar = UfreeStream_*0.41/log((inputWindHeight_Veg_)/z0_);

    // Loop over all the faces in that patch
    forAll(Kp, faceI)
    {
        Kp[faceI] = pow(ustar,2)/pow(Cmu,0.5);
    }

    operator==(Kp);

    fixedValueFvPatchScalarField::updateCoeffs();
}

//- Write
void Foam::logProfileTurbulentKineticEnergyInletFvPatchScalarField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    writeEntry(os, "UfreeStream", UfreeStream_);
    writeEntry(os, "uDirection", uDirection_);
    writeEntry(os, "inputWindHeight_Veg", inputWindHeight_Veg_);
    writeEntry(os, "z0", z0_);
    writeEntry(os, "Rd", Rd_);
    writeEntry(os, "firstCellHeight", firstCellHeight_);
//if(Foam::Pstream::parRun())
//{
    writeEntry(os, "relativeHeights", relativeHeight_);
//}
    writeEntry(os, "value", *this);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        logProfileTurbulentKineticEnergyInletFvPatchScalarField
    );
}

// ************************************************************************* //
