/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Class
    logProfileTurbulentKineticEnergyInletFvPatchScalarField

Description
    Boundary condition specifies a log law profile on velocity inlet profile
    (fixed value), given free stream velocity, roughness parameters

SourceFiles
    logProfileTurbulentKineticEnergyInletFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef logProfileTurbulentKineticEnergyInletFvPatchScalarField_H
#define logProfileTurbulentKineticEnergyInletFvPatchScalarField_H

#include "fvPatchFields.H"
#include "fixedValueFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
              Class logProfileTurbulentKineticEnergyInletFvPatchField Declaration
\*---------------------------------------------------------------------------*/

class logProfileTurbulentKineticEnergyInletFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{
        // Private data

        //- Peak velocity magnitude
        scalar maxValue_;	 //Scalar Vmax

        scalar UfreeStream_; 	// Free Stream Velocity
        scalar inputWindHeight_Veg_; // Input wind Height on top of Vegetation
        scalar z0_;    // Roughness Parameters
        scalar Rd_; //Zero plane displacement height
        scalarField relativeHeight_;	// Relative distance from wall
        scalar firstCellHeight_; // First Cell Height

public:

    //- Runtime type information
    TypeName("logProfileTurbulentKineticEnergyInlet");


    // Constructors

        //- Construct from patch and internal field
        logProfileTurbulentKineticEnergyInletFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        logProfileTurbulentKineticEnergyInletFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given logProfileTurbulentKineticEnergyInletFvPatchScalarField
        //  onto a new patch
        logProfileTurbulentKineticEnergyInletFvPatchScalarField
        (
            const logProfileTurbulentKineticEnergyInletFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new logProfileTurbulentKineticEnergyInletFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        logProfileTurbulentKineticEnergyInletFvPatchScalarField
        (
            const logProfileTurbulentKineticEnergyInletFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchScalarField> clone
        (
            const DimensionedField<scalar, volMesh>& iF
        ) const
        {
            return tmp<fvPatchScalarField>
            (
                new logProfileTurbulentKineticEnergyInletFvPatchScalarField(*this, iF)
            );
        }


    // Member functions

        //- Return max value
	scalar& UfreeStream()
	{
            return UfreeStream_;
	}

	scalar& inputWindHeight_Veg()
	{
	    return inputWindHeight_Veg_;
	}


    scalar& z0()
    {
        return z0_;
    }

    scalar& Rd()
    {
        return Rd_;
    }

	scalar& firstCellHeight()
	{
	    return firstCellHeight_;
	}

        //- Update coefficients
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
