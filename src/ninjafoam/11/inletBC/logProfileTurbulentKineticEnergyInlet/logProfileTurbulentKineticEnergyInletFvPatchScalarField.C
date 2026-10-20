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

    // PARALLEL SAFEGUARD: If this specific processor subdomain
    // does not own any faces on this patch, exit early.
    if(faceCenters.empty())
    {
        return;
    }

    label minZPatchID = patch().boundaryMesh().findPatchID("minZ");
    if(minZPatchID == -1)
    {
        FatalErrorIn("logProfileVelocityInletFvPatchVectorField")
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
        FatalErrorIn("logProfileVelocityInletFvPatchVectorField")
            << "Boundary condition applied on incorrect patch"
            << abort(FatalError);
    }

    scalar patchNormPos = faceCenters[0].component(normDir);  // the x or y common position, of the patch, to compare against
    scalar tolerance = 0.1;  // search distance away from the inlet patch, of the minZ patch
    std::map<scalar, scalar> groundProfileMap;
    forAll(minZ_points, pointI)
    {
        // search by the normal coordinate
        if(abs(minZ_points[pointI].component(normDir) - patchNormPos) < tolerance)
        {
            // Map automatically handles sorting by the tangential coordinate
            groundProfileMap[minZ_points[pointI].component(tanDir)] = minZ_points[pointI].z();
        }
    }
    if(groundProfileMap.empty())
    {
        FatalErrorIn("logProfileVelocityInletFvPatchVectorField")
            << "No matching minZ faces found near the inlet patch"
            << abort(FatalError);
    }

    List<Tuple2<scalar, scalar>> tableEntries(groundProfileMap.size());
    label idx = 0;
    for(const std::pair<const scalar, scalar>& pair : groundProfileMap)
    {
        tableEntries[idx] = Tuple2<scalar, scalar>(pair.first, pair.second);
        idx++;
    }

    // Set out-of-bounds behavior to CLAMP (retains edge values if inlet exceeds minZ bounds)
    dictionary tableDict;
    tableDict.add("values", tableEntries);
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
    setTerrainHeights();

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
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}

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
