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

Class
    Foam::incompressible::epsilonNonEquiWallFunctionFvPatchScalarField

Group
    grpIcoWallFunctions

Description
    This boundary condition provides a turbulence dissipation wall function
    condition for high Reynolds number, turbulent flow cases.

    The condition can be applied to wall boundaries, whereby it
    - calculates \c epsilon and \c G
    - inserts near wall epsilon values directly into the epsilon equation
        to act as a constraint

    where

    \vartable
        epsilon | turblence dissipation field
        G       | turblence generation field
    \endvartable

    \heading Patch usage

    \table
        Property     | Description             | Required    | Default value
        Cmu          | model coefficient       | no          | 0.09
        kappa        | Von Karman constant     | no          | 0.41
        E            | model coefficient       | no          | 9.8
    \endtable

    Example of the boundary condition specification:
    \verbatim
    myPatch
    {
        type            epsilonNonEquiWallFunction;
    }
    \endverbatim

SeeAlso
    Foam::fixedInternalValueFvPatchField

SourceFiles
    epsilonNonEquiWallFunctionFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef EPSILONNONEQUIWALLFUNCTIONFVPATCHSCALARFIELD_H_
#define EPSILONNONEQUIWALLFUNCTIONFVPATCHSCALARFIELD_H_

#include "fixedInternalValueFvPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace incompressible
{

/*---------------------------------------------------------------------------*\
           Class epsilonNonEquiWallFunctionFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class epsilonNonEquiWallFunctionFvPatchScalarField
:
    public fixedInternalValueFvPatchField<scalar>
{
protected:

    // Protected data

        //- Cmu coefficient
        scalar Cmu_;

        //- Von Karman constant
        scalar kappa_;

        //- E coefficient
        scalar E_;


    // Protected Member Functions

        //- Check the type of the patch
        virtual void checkType();

        //- Write local wall function variables
        virtual void writeLocalEntries(Ostream&) const;


public:

    //- Runtime type information
    TypeName("epsilonNonEquiWallFunction");


    // Constructors

        //- Construct from patch and internal field
        epsilonNonEquiWallFunctionFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        epsilonNonEquiWallFunctionFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  epsilonNonEquiWallFunctionFvPatchScalarField
        //  onto a new patch
        epsilonNonEquiWallFunctionFvPatchScalarField
        (
            const epsilonNonEquiWallFunctionFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        epsilonNonEquiWallFunctionFvPatchScalarField
        (
            const epsilonNonEquiWallFunctionFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new epsilonNonEquiWallFunctionFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        epsilonNonEquiWallFunctionFvPatchScalarField
        (
            const epsilonNonEquiWallFunctionFvPatchScalarField&,
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
                new epsilonNonEquiWallFunctionFvPatchScalarField(*this, iF)
            );
        }


    // Member functions

        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();

            //- Evaluate the patchField
            virtual void evaluate(const Pstream::commsTypes);


        // I-O

            //- Write
            virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif /* EPSILONNONEQUIWALLFUNCTIONFVPATCHSCALARFIELD_H_ */
