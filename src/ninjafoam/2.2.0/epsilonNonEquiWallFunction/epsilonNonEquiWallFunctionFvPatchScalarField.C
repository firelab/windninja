/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2012 OpenFOAM Foundation
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

#include "epsilonNonEquiWallFunctionFvPatchScalarField.H"
#include "incompressible/turbulenceModel/turbulenceModel.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "addToRunTimeSelectionTable.H"
#include "wallFvPatch.H"
#include "nutkWallFunctionFvPatchScalarField.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace incompressible
{

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void epsilonNonEquiWallFunctionFvPatchScalarField::checkType()
{
    if (!isA<wallFvPatch>(patch()))
    {
        FatalErrorIn("epsilonNonEquiWallFunctionFvPatchScalarField::checkType()")
            << "Invalid wall function specification" << nl
            << "    Patch type for patch " << patch().name()
            << " must be wall" << nl
            << "    Current patch type is " << patch().type() << nl << endl
            << abort(FatalError);
    }
}


void epsilonNonEquiWallFunctionFvPatchScalarField::writeLocalEntries(Ostream& os) const
{
    os.writeKeyword("Cmu") << Cmu_ << token::END_STATEMENT << nl;
    os.writeKeyword("kappa") << kappa_ << token::END_STATEMENT << nl;
    os.writeKeyword("E") << E_ << token::END_STATEMENT << nl;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

epsilonNonEquiWallFunctionFvPatchScalarField::epsilonNonEquiWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedInternalValueFvPatchField<scalar>(p, iF),
    Cmu_(0.09),
    kappa_(0.41),
    E_(9.8)
{
    checkType();
}


epsilonNonEquiWallFunctionFvPatchScalarField::epsilonNonEquiWallFunctionFvPatchScalarField
(
    const epsilonNonEquiWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedInternalValueFvPatchField<scalar>(ptf, p, iF, mapper),
    Cmu_(ptf.Cmu_),
    kappa_(ptf.kappa_),
    E_(ptf.E_)
{
    checkType();
}


epsilonNonEquiWallFunctionFvPatchScalarField::epsilonNonEquiWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedInternalValueFvPatchField<scalar>(p, iF, dict),
    Cmu_(dict.lookupOrDefault<scalar>("Cmu", 0.09)),
    kappa_(dict.lookupOrDefault<scalar>("kappa", 0.41)),
    E_(dict.lookupOrDefault<scalar>("E", 9.8))
{
    checkType();
}


epsilonNonEquiWallFunctionFvPatchScalarField::epsilonNonEquiWallFunctionFvPatchScalarField
(
    const epsilonNonEquiWallFunctionFvPatchScalarField& ewfpsf
)
:
    fixedInternalValueFvPatchField<scalar>(ewfpsf),
    Cmu_(ewfpsf.Cmu_),
    kappa_(ewfpsf.kappa_),
    E_(ewfpsf.E_)
{
    checkType();
}


epsilonNonEquiWallFunctionFvPatchScalarField::epsilonNonEquiWallFunctionFvPatchScalarField
(
    const epsilonNonEquiWallFunctionFvPatchScalarField& ewfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedInternalValueFvPatchField<scalar>(ewfpsf, iF),
    Cmu_(ewfpsf.Cmu_),
    kappa_(ewfpsf.kappa_),
    E_(ewfpsf.E_)
{
    checkType();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void epsilonNonEquiWallFunctionFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const label patchI = patch().index();

    const turbulenceModel& turbulence =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbulence.y()[patchI];

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            turbulence.U().mesh().time().constant(),
            turbulence.U().mesh(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );
    dimensionedScalar rho(transportProperties.lookup("rho"));

    const scalar Cmu25 = pow025(Cmu_);
    const scalar Cmu75 = pow(Cmu_, 0.75);

    volScalarField& G =
        const_cast<volScalarField&>
        (
            db().lookupObject<volScalarField>
            (
                turbulence.type() + ".G"
            )
        );

    DimensionedField<scalar, volMesh>& epsilon =
        const_cast<DimensionedField<scalar, volMesh>&>
        (
            dimensionedInternalField()
        );

    const tmp<volScalarField> tk = turbulence.k();
    const volScalarField& k = tk();
    const scalarField& kw = k.boundaryField()[patchI];

    const tmp<volScalarField> tnu = turbulence.nu();
    const scalarField& nuw = tnu().boundaryField()[patchI];

    const tmp<volScalarField> tnut = turbulence.nut();
    const volScalarField& nut = tnut();
    const scalarField& nutw = nut.boundaryField()[patchI];

    const fvPatchVectorField& Uw = turbulence.U().boundaryField()[patchI];

    const scalarField magGradUw(mag(Uw.snGrad()));

    const scalar rhow(rho.value());
    const scalarField muw(nuw*rhow);
    scalarField yStarv(muw.size(),11.24);
    const scalarField yv(nuw*yStarv/(Cmu25*pow(kw,0.5)));


    // Set epsilon and G
    forAll(nutw, faceI)
    {
        label faceCellI = patch().faceCells()[faceI];

        epsilon[faceCellI] =k[faceCellI] *(2*muw[faceI]/(rhow*yv[faceI])
            +pow(k[faceCellI],0.5)*log(2.0*y[faceI]/yv[faceI])/(kappa_*pow(Cmu_,-0.75)))/(2.0*y[faceI]);
        scalar tw=rhow*(nutw[faceI] + nuw[faceI])*magGradUw[faceI];

        G[faceCellI] = pow(tw,2)*log(2*y[faceI]/yv[faceI])/(kappa_*2.0*y[faceI]*rhow*Cmu25*pow(k[faceCellI],0.5));
    }

    fixedInternalValueFvPatchField<scalar>::updateCoeffs();

    // TODO: perform averaging for cells sharing more than one boundary face
}


void epsilonNonEquiWallFunctionFvPatchScalarField::evaluate
(
    const Pstream::commsTypes commsType
)
{
    fixedInternalValueFvPatchField<scalar>::evaluate(commsType);
}


void epsilonNonEquiWallFunctionFvPatchScalarField::write(Ostream& os) const
{
    fixedInternalValueFvPatchField<scalar>::write(os);
    writeLocalEntries(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    epsilonNonEquiWallFunctionFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace Foam

// ************************************************************************* //
