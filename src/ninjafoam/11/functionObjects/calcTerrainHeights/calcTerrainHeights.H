/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2026 OpenFOAM Foundation  or  Copyright held by original author
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
    Foam::functionObjects::calcTerrainHeights

Description
    Evaluates and outputs a z_AGL/relativeHeights_ volScalarField with values for the inlet patches filled and all other patches full of zeroes.
    Also evaluates and outputs a minZ 1D line of data for each inlet patch.

    Example of function object specification:
    \verbatim
    calcTerrainHeights1
    {
        type             calcTerrainHeights;
        libs             ("libWindNinja.so");
        ...
        patches          (west_face south_face);  // the list of inlet patches
        groundPatchName  minZ;  // the ground patch name
    }
    \endverbatim

Usage
    \table
        Property     | Description               | Required | Default value
        type         | type name: calcTerrainHeights | yes      |
        patches      | wordList patch names...       | yes      |
        groundPatchName    | word value...      | yes      |
    \endtable

SourceFiles
    calcTerrainHeights.C

\*---------------------------------------------------------------------------*/

#ifndef calcTerrainHeights_H
#define calcTerrainHeights_H

#include "fvMeshFunctionObject.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                            Class calcTerrainHeights Declaration
\*---------------------------------------------------------------------------*/

class calcTerrainHeights
:
    public fvMeshFunctionObject
{
    // Private Data

        //- Names of the inlet patches to process
        wordList patchNames_;

        //- Name of the ground patch
        word groundPatchName_;

    // Private Member Functions

        void computeTerrainHeights();

public:

    //- Runtime type information
    TypeName("calcTerrainHeights");

    // Constructors

        //- Construct from Time and dictionary
        calcTerrainHeights
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict
        );

        //- Disallow default bitwise copy construction
        calcTerrainHeights(const calcTerrainHeights&) = delete;

    //- Destructor
    virtual ~calcTerrainHeights();

    // Member Functions

        //- Read the dictionary
        virtual bool read(const dictionary&);

        //- Return the list of fields -- required
        virtual wordList fields() const
        {
            return wordList::null();
        }

        //- Calculations run each timestep
        virtual bool execute();

        //- Write fields
        virtual bool write();

        //- Update topology using the given mesh
        virtual void movePoints(const polyMesh& mesh);

        //- Update topology using the given map
        virtual void topoChange(const polyTopoChangeMap& map);

        //- Update from another mesh using the given map
        virtual void mapMesh(const polyMeshMap&);

        //- Redistribute or update using the given distribution map
        virtual void distribute(const polyDistributionMap&);

    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const calcTerrainHeights&) = delete;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
