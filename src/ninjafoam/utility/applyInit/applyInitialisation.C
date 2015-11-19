/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2014 OpenFOAM Foundation
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

Application
    applyBoundaryLayer

Description
    Apply a simplified boundary-layer model to the velocity and
    turbulence fields based on the 1/7th power-law.

    The uniform boundary-layer thickness is either provided via the -ybl option
    or calculated as the average of the distance to the wall scaled with
    the thickness coefficient supplied via the option -Cbl.  If both options
    are provided -ybl is used.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "singlePhaseTransportModel.H"
#include "RASModel.H"
#include "wallDist.H"
#include "scalar.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// turbulence constants - file-scope


int main(int argc, char *argv[])
{

    #include "setRootCase.H"

	#include "createTime.H"
    #include "createMesh.H"

	volVectorField U
	(
		IOobject
		(
			"U",
			runTime.timeName(),
			mesh,
			IOobject::MUST_READ,
			IOobject::AUTO_WRITE
		),
		mesh
	);
	word patchName;
	forAll(U.boundaryField().types(),patchI)
	{
		if(!U.boundaryField().types()[patchI].compare(word("logProfileVelocityInlet")))
		{
			patchName = mesh.boundary()[patchI].name();
		}
	}

	const IOdictionary uFile_
    (
        IOobject
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        U.readStream(word("volVectorField"))
    );

	const dictionary dict(uFile_.subDict("boundaryField").subDict(patchName));

	Info<< "Calculating wall distance field" << endl;

    volScalarField y = wallDist(mesh).y();	//distance to nearest wall
	//volScalarField y(mesh.C().component(2)); //z-component of cell center

	const scalar UfreeStream_(readScalar(dict.lookup("UfreeStream")));
	const vector uDirection_(dict.lookup("uDirection"));
	const scalar inputWindHeight_Veg_(readScalar(dict.lookup("inputWindHeight_Veg")));
	const scalar z0_(readScalar(dict.lookup("z0")));
	//const scalar Rd_(readScalar(dict.lookup("Rd")));
	scalar ustar = Foam::log((inputWindHeight_Veg_)/z0_);
	ustar = (UfreeStream_*0.41)/(ustar);
	scalar ucalc(0.0);

	// Loop over all the faces in the patch
	// and initialize the log profile
	forAll(y,cellI)
	{
		// relative height from ground for face lists
		scalar AGL = y[cellI];

		//Apply the log law equation profile
        ucalc = ustar/0.41*Foam::log((AGL)/z0_);
    	U[cellI] = ucalc*uDirection_;
	}

	U.write();
	Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
