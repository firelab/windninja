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

Class
    foam::logProfileDissipationRateInletFvPatchScalarField

Description
    Boundary condition specifies a log law profile on velocity inlet profile
    (fixed value), given free stream velocity, roughness parameters

Usage
    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            	logProfileVelocityInlet;
        UfreeStream     	2.2351999999999999;			// velocity at reference height above ground. Default value is 5 mph
        uDirection      	(1.0000 0.0000 0.0000);		// reference wind direction. Default value is -x to x direction
        inputWindHeight_Veg 6.095999804928006;			// reference height above ground. Default value is 20 ft
        z0              	0.42999999999993194;		// ground roughness. Default is 0.43 for brush
        Rd              	1.7999999999996967;			// ground roughness parameter. Default is 1.8 for brush
        firstCellHeight 	0.0;						// height of first cell. Starts out at the blockMeshResolution, 
														//  which is meshResolution*2*nRoundsRefinement.
														//  This is divided by 2 for each level of mesh refinement.
														//  Default is flat terrain, a value of 0.
    }
    \endverbatim

Notes:
	This is similar to atmBoundaryLayerInletVelocityFvPatchVectorField.C/H, but
	it has much better control over the ground height correction, a method that
	is robust enough to support WindNinja's method of specifying terrain.

SourceFiles
    logProfileDissipationRateInletFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef logProfileDissipationRateInletFvPatchScalarField_H
#define logProfileDissipationRateInletFvPatchScalarField_H

#include "fvPatchFields.H"
#include "fixedValueFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
           Class logProfileDissipationRateInletFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class logProfileDissipationRateInletFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{
    // Private Data

        //- Peak velocity magnitude
        //scalar maxValue_;     //Scalar Vmax
		
		scalar UfreeStream_;            // Free Stream Velocity
		vector uDirection_;             // Direction of Free Stream Velocity
		scalar inputWindHeight_Veg_;    // Input wind Height on top of Vegetation
		scalar z0_;                     // Roughness Parameters
		scalar Rd_;                     // Zero plane displacement height
		scalarField relativeHeight_;    // Relative distance from wall
		scalar firstCellHeight_;        // First Cell Height
        
    // Private Member Functions

        //- set all the various height values specific for each cell of the domain
        void setTerrainHeights();

public:

    //- Runtime type information
    TypeName("logProfileDissipationRateInlet");


    // Constructors

        //- Construct from patch and internal field
        logProfileDissipationRateInletFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        logProfileDissipationRateInletFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given logProfileDissipationRateInletFvPatchScalarField
        //  onto a new patch
        logProfileDissipationRateInletFvPatchScalarField
        (
            const logProfileDissipationRateInletFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Copy constructor
        logProfileDissipationRateInletFvPatchScalarField
        (
            const logProfileDissipationRateInletFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new logProfileDissipationRateInletFvPatchScalarField(*this)
            );
        }

        //- Copy constructor setting internal field reference
        logProfileDissipationRateInletFvPatchScalarField
        (
            const logProfileDissipationRateInletFvPatchScalarField&,
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
                new logProfileDissipationRateInletFvPatchScalarField(*this, iF)
            );
        }


    // Member Functions

        // Attributes

            //- Return true: this patch field is altered by assignment
            virtual bool assignable() const
            {
                return true;
            }


        // Access

            //- Return the value of UfreeStream
            const scalar& UfreeStream() const
            {
                return UfreeStream_;
            }
            //- Return reference to the value of UfreeStream to allow adjustment
            scalar& UfreeStream()
            {
                return UfreeStream_;
            }

            //- Return the value of inputWindHeight_Veg
            const scalar& inputWindHeight_Veg() const
            {
                return inputWindHeight_Veg_;
            }
            //- Return reference to the value of inputWindHeight_Veg to allow adjustment
            scalar& inputWindHeight_Veg()
            {
                return inputWindHeight_Veg_;
            }
            
            //- Return the value of z0
            const scalar& z0() const
            {
                return z0_;
            }
            //- Return reference to the value of z0 to allow adjustment
            scalar& z0()
            {
                return z0_;
            }
            
            //- Return the value of Rd
            const scalar& Rd() const
            {
                return Rd_;
            }
            //- Return reference to the value of Rd to allow adjustment
            scalar& Rd()
            {
                return Rd_;
            }
            
            //- Return the value of firstCellHeight
            const scalar& firstCellHeight() const
            {
                return firstCellHeight_;
            }
            //- Return reference to the value of firstCellHeight to allow adjustment
            scalar& firstCellHeight()
            {
                return firstCellHeight_;
            }


        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
        
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
