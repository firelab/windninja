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

#include "myTurbulentHeatFluxTemperatureFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "incompressible/turbulenceModel/turbulenceModel.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    // declare specialization within 'Foam' namespace
    template<>
    const char* NamedEnum
    <
        Foam::incompressible::
        myTurbulentHeatFluxTemperatureFvPatchScalarField::heatSourceType,
        2
    >::names[] =
    {
        "power",
        "flux"
    };
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


namespace Foam
{

namespace incompressible
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const NamedEnum
<
    myTurbulentHeatFluxTemperatureFvPatchScalarField::heatSourceType,
    2
> myTurbulentHeatFluxTemperatureFvPatchScalarField::heatSourceTypeNames_;


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

myTurbulentHeatFluxTemperatureFvPatchScalarField::
myTurbulentHeatFluxTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(p, iF),
    heatSource_(hsPower),
    q_(p.size(), 0.0),
    alphaEffName_("undefinedAlphaEff")
{}


myTurbulentHeatFluxTemperatureFvPatchScalarField::
myTurbulentHeatFluxTemperatureFvPatchScalarField
(
    const myTurbulentHeatFluxTemperatureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchScalarField(ptf, p, iF, mapper),
    heatSource_(ptf.heatSource_),
    q_(ptf.q_, mapper),
    alphaEffName_(ptf.alphaEffName_)
{}


myTurbulentHeatFluxTemperatureFvPatchScalarField::
myTurbulentHeatFluxTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedGradientFvPatchScalarField(p, iF),
    heatSource_(heatSourceTypeNames_.read(dict.lookup("heatSource"))),
    q_("q", dict, p.size()),
    alphaEffName_(dict.lookup("alphaEff"))
{
    fvPatchField<scalar>::operator=(patchInternalField());
    gradient() = 0.0;
}


myTurbulentHeatFluxTemperatureFvPatchScalarField::
myTurbulentHeatFluxTemperatureFvPatchScalarField
(
    const myTurbulentHeatFluxTemperatureFvPatchScalarField& thftpsf
)
:
    fixedGradientFvPatchScalarField(thftpsf),
    heatSource_(thftpsf.heatSource_),
    q_(thftpsf.q_),
    alphaEffName_(thftpsf.alphaEffName_)
{}


myTurbulentHeatFluxTemperatureFvPatchScalarField::
myTurbulentHeatFluxTemperatureFvPatchScalarField
(
    const myTurbulentHeatFluxTemperatureFvPatchScalarField& thftpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(thftpsf, iF),
    heatSource_(thftpsf.heatSource_),
    q_(thftpsf.q_),
    alphaEffName_(thftpsf.alphaEffName_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void myTurbulentHeatFluxTemperatureFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    scalarField::autoMap(m);
    q_.autoMap(m);
}


void myTurbulentHeatFluxTemperatureFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    fixedGradientFvPatchScalarField::rmap(ptf, addr);

    const myTurbulentHeatFluxTemperatureFvPatchScalarField& thftptf =
        refCast<const myTurbulentHeatFluxTemperatureFvPatchScalarField>
        (
            ptf
        );

    q_.rmap(thftptf.q_, addr);
}


void myTurbulentHeatFluxTemperatureFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

	//std::cout << "Constructing scalarField alphaEffp from alphaEffName_ \"" << alphaEffName_ << "\"\n";
    scalarField alphaEffp =
        patch().lookupPatchField<volScalarField, scalar>(alphaEffName_);

	//std::cout << "setting up turbulence model\n";
    // retrieve (constant) specific heat capacity from transport dictionary
    const turbulenceModel& turbulence =
        db().lookupObject<turbulenceModel>("turbulenceModel");
	//std::cout << "reading scalar CpWall from transport properties\n";
    const dimensionedScalar foundCpWall(turbulence.transport().lookup("Cp0"));
	scalar CpWall = foundCpWall.value();
	//scalar CpWall(readScalar(turbulence.transport().lookup("CpWall")));

	//std::cout << "checking for zero values\n";
	bool foundZero = false;
	forAll(alphaEffp,cellI)
	{
		if(alphaEffp[cellI] == 0)
		{
			alphaEffp[cellI] = 1;
			foundZero = true;
		}
	}
	if(foundZero == true)
	{
		std::cout << "WARNING in myTurbulentHeatFluxTemperatureFvPatchScalarField.updateCoeffs()! Found at least one alphaEffp value that was 0. To avoid dividing by zero in gradient calculation, set all 0 values to 1\n";
	}
	if(CpWall == 0)
	{
		std::cout << "WARNING in myTurbulentHeatFluxTemperatureFvPatchScalarField.updateCoeffs()! CpWall = 0, set value to 1000\n";
		CpWall = 1000;
	}

	//std::cout << "figuring out heat source type/units\n";
    switch (heatSource_)
    {
        case hsPower:
        {
			//std::cout << "heat source is type power W, calculating scalar Ap\n";
            const scalar Ap = gSum(patch().magSf());
			//std::cout << "calculating gradient\n";
            gradient() = q_/(Ap*CpWall*alphaEffp);
            break;
        }
        case hsFlux:
        {
			//std::cout << "heat source is type flux W/m^2. Calculating gradient\n";
            gradient() = q_/(CpWall*alphaEffp);	// hm, I bet alphaEffp is somehow 0.
            break;
        }
        default:
        {
            FatalErrorIn
            (
                "myTurbulentHeatFluxTemperatureFvPatchScalarField"
                "("
                    "const fvPatch&, "
                    "const DimensionedField<scalar, volMesh>&, "
                    "const dictionary&"
                ")"
            )   << "Unknown heat source type. Valid types are: "
                << heatSourceTypeNames_ << nl << exit(FatalError);
        }
    }

	//std::cout << "updating coefficients using fixedGradientFvPatchScalarField class updateCoeffs()\n\n";
    fixedGradientFvPatchScalarField::updateCoeffs();
}


void myTurbulentHeatFluxTemperatureFvPatchScalarField::write(Ostream& os) const
{
    fixedGradientFvPatchScalarField::write(os);
    os.writeKeyword("heatSource") << heatSourceTypeNames_[heatSource_]
        << token::END_STATEMENT << nl;
    q_.writeEntry("q", os);
    os.writeKeyword("alphaEff") << alphaEffName_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    myTurbulentHeatFluxTemperatureFvPatchScalarField
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace Foam


// ************************************************************************* //

