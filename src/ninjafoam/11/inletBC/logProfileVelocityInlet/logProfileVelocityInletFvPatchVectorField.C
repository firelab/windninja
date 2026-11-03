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

#include "logProfileVelocityInletFvPatchVectorField.H"
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

//- Construct from patch and internal field
Foam::logProfileVelocityInletFvPatchVectorField::
logProfileVelocityInletFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF),
    UfreeStream_(0.0),
    uDirection_(0.0,0.0,0.0),
    inputWindHeight_Veg_(0),
    z0_(0),
    Rd_(0),
    relativeHeight_(0),
    firstCellHeight_(0)
{}

//- Construct from patch, internal field and dictionary
Foam::logProfileVelocityInletFvPatchVectorField::
logProfileVelocityInletFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF, dict, false),
    UfreeStream_(readScalar(dict.lookup("UfreeStream"))),
    uDirection_(dict.lookup("uDirection")),
    inputWindHeight_Veg_(readScalar(dict.lookup("inputWindHeight_Veg"))),
    z0_(readScalar(dict.lookup("z0"))),
    Rd_(readScalar(dict.lookup("Rd"))),
    relativeHeight_(0),
    firstCellHeight_(readScalar(dict.lookup("firstCellHeight")))
{
    evaluate();
}

//- Construct by mapping given logProfileVelocityInletFvPatchVectorField
//  onto a new patch
Foam::logProfileVelocityInletFvPatchVectorField::
logProfileVelocityInletFvPatchVectorField
(
    const logProfileVelocityInletFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(mapper(ptf.relativeHeight_)),
    firstCellHeight_(ptf.firstCellHeight_)
{}

void Foam::logProfileVelocityInletFvPatchVectorField::map
(
    const fvPatchField<vector>& ptf,
    const fvPatchFieldMapper& mapper
)
{
    fixedValueFvPatchVectorField::map(ptf, mapper);

    const logProfileVelocityInletFvPatchVectorField& logBC =
        refCast<const logProfileVelocityInletFvPatchVectorField>(ptf);

    mapper(relativeHeight_, logBC.relativeHeight_);
}

/*//- Copy constructor
Foam::logProfileVelocityInletFvPatchVectorField::
logProfileVelocityInletFvPatchVectorField
(
    const logProfileVelocityInletFvPatchVectorField& ptf
)
:
    fixedValueFvPatchVectorField(ptf),
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
Foam::logProfileVelocityInletFvPatchVectorField::
logProfileVelocityInletFvPatchVectorField
(
    const logProfileVelocityInletFvPatchVectorField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(ptf, iF),
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
void Foam::logProfileVelocityInletFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    if(relativeHeight_.empty())
    {
        label patchID = patch().index();
        const fvMesh& mesh = patch().boundaryMesh().mesh();

        // try to look up zAGL from the object registry (simpleFoam controlDict mode)
        // if that fails, try reading zAGL from disk (foamPostProcess mode)
        // warn the user if neither of these methods work, zAGL is REQUIRED for this custom BC
        if(db().foundObject<volScalarField>("zAGL"))
        {
            const volScalarField& zAGL = db().lookupObject<volScalarField>("zAGL");
            relativeHeight_ = zAGL.boundaryField()[patchID];
        }
        else
        {
            IOobject zAGLHeader
            (
                "zAGL",
                db().time().timeName(),
                mesh,
                IOobject::READ_IF_PRESENT,
                IOobject::NO_WRITE
            );
            if(zAGLHeader.headerOk())
            {
                volScalarField zAGL(zAGLHeader, mesh);
                relativeHeight_ = zAGL.boundaryField()[patchID];
            }
            else
            {
                FatalErrorIn("logProfileVelocityInletFvPatchVectorField")
                    << "The zAGL field could not be found in the database or as a file on disk.\n"
                    << "This boundary condition requires zAGL to calculate valid physical profiles.\n\n"
                    << "Please pre-calculate the field before running this utility by executing:\n"
                    << "    foamPostProcess -func computeTerrainHeights -latestTime\n\n"
                    << exit(FatalError);
            }
        }
    }

    scalar ustar = UfreeStream_*0.41/Foam::log((inputWindHeight_Veg_)/z0_);
    scalar ucalc(0.0);
    vectorField Up(patch().Cf().size(), vector(0.0, 0.0, 0.0));

    // Loop over all the faces in that patch
    forAll(Up, faceI)
    {
        // relative height from ground for face lists
        scalar& AGL = relativeHeight_[faceI];

        //Apply the log profile
        ucalc = ustar/0.41*Foam::log((AGL)/z0_);
        Up[faceI] = ucalc*uDirection_;
    }

    operator==(Up);

    fixedValueFvPatchVectorField::updateCoeffs();
}

//- Write
void Foam::logProfileVelocityInletFvPatchVectorField::write(Ostream& os) const
{
    fvPatchVectorField::write(os);
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
        fvPatchVectorField,
        logProfileVelocityInletFvPatchVectorField
    );
}

// ************************************************************************* //
