/*---------------------------------------------------------------------------* \
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

#include "nutNonEquiWallFunctionFvPatchScalarField.H"
#include "incompressible/turbulenceModel/turbulenceModel.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "wallFvPatch.H"
#include "addToRunTimeSelectionTable.H"
#include "fvcGrad.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace incompressible
{

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

tmp<scalarField> nutNonEquiWallFunctionFvPatchScalarField::calcNut() const
{
    const label patchi = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            turbModel.U().mesh().time().constant(),
            turbModel.U().mesh(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );
    dimensionedScalar rho(transportProperties.lookup("rho"));


    const scalarField& y = turbModel.y()[patchi];
    const tmp<volScalarField> tk = turbModel.k();
    const volScalarField& k = tk();
    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchi];
    const scalarField& kw = k.boundaryField()[patchi];

    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchi];
    const vectorField Uc(Uw.patchInternalField());
    const volScalarField & pTemp = db().lookupObject<volScalarField>("p");
    const volVectorField pGrad(fvc::grad(pTemp));

    const vectorField pGradW(pGrad.boundaryField()[patchi]);
    const scalarField pSnGrad(pTemp.boundaryField()[patchi].snGrad());
    const scalarField pTang(mag(pGradW)-pSnGrad);
    const scalarField UMag(mag(Uc-Uw));
    scalarField yStarv(kw.size(),11.24);
    const scalar rhow(rho.value());
    const scalarField muw(nuw*rhow);

    const scalar Cmu25 = pow025(Cmu_);

    tmp<scalarField> tnutw(new scalarField(patch().size(), 0.0));
    scalarField& nutw = tnutw();

    const scalarField yv=nuw*yStarv/(Cmu25*pow(kw,0.5));

    scalarField UTilda= mag(Uc)-0.5*pTang*(yv/(rhow*kappa_*pow(kw,0.5))*log(y/yv)+ (y-yv)/(rhow*kappa_*pow(kw,0.5))+pow(yv,2)/muw);
    forAll(nutw, faceI)
    {
        label faceCellI = patch().faceCells()[faceI];

        scalar yPlus = Cmu25*y[faceI]*sqrt(k[faceCellI])/nuw[faceI];

        if ((yPlus > yPlusLam_) && (UMag[faceI] > SMALL))
        {
            scalar C(Cmu25*pow(kw[faceI],0.5));
            nutw[faceI] = max(UTilda[faceI]*C*rhow*y[faceI]/(UMag[faceI]*log(E_*rhow*C*y[faceI]/muw[faceI])/kappa_)-muw[faceI],0.0);
            nutw[faceI]/=rhow;
        }
    }

    return tnutw;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

nutNonEquiWallFunctionFvPatchScalarField::nutNonEquiWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    nutWallFunctionFvPatchScalarField(p, iF)
{}


nutNonEquiWallFunctionFvPatchScalarField::nutNonEquiWallFunctionFvPatchScalarField
(
    const nutNonEquiWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    nutWallFunctionFvPatchScalarField(ptf, p, iF, mapper)
{}


nutNonEquiWallFunctionFvPatchScalarField::nutNonEquiWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    nutWallFunctionFvPatchScalarField(p, iF, dict)
{}


nutNonEquiWallFunctionFvPatchScalarField::nutNonEquiWallFunctionFvPatchScalarField
(
    const nutNonEquiWallFunctionFvPatchScalarField& wfpsf
)
:
    nutWallFunctionFvPatchScalarField(wfpsf)
{}


nutNonEquiWallFunctionFvPatchScalarField::nutNonEquiWallFunctionFvPatchScalarField
(
    const nutNonEquiWallFunctionFvPatchScalarField& wfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    nutWallFunctionFvPatchScalarField(wfpsf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

tmp<scalarField> nutNonEquiWallFunctionFvPatchScalarField::yPlus() const
{
    const label patchi = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchi];

    const tmp<volScalarField> tk = turbModel.k();
    const volScalarField& k = tk();
    tmp<scalarField> kwc = k.boundaryField()[patchi].patchInternalField();
    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchi];

    return pow025(Cmu_)*y*sqrt(kwc)/nuw;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    nutNonEquiWallFunctionFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace Foam

// ************************************************************************* //
