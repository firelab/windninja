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
    turbulence fields based on the "method done for inlet log velocity".    Used to say "1/7th power-law", but I need to verify that this isn't just an old comment.

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

    // declare the velocity field IO object
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
    
    // find the patchname to use to find the right dictionary 
    // to lookup correct values of coefficients
	word patchName;
	forAll(U.boundaryField().types(),patchI)
	{
		if(!U.boundaryField().types()[patchI].compare(word("logProfileVelocityInlet")))
		{
			patchName = mesh.boundary()[patchI].name();
		}
	}

    // declare uFile object, used alongside patchName to get the dictionary
    // object with the correct values for the coefficients
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

    // get the dictionary with the coefficient values
	const dictionary dict(uFile_.subDict("boundaryField").subDict(patchName));

	Info << "Calculating wall distance field" << endl;

    // get the locations for the calculation
    // use comments to choose method/locations
    volScalarField y = wallDist(mesh).y();          //distance to nearest wall
	//volScalarField y(mesh.C().component(2));      //z-component of cell center


    // get the constants from the dictionary, the log profile inlet values
	const scalar UfreeStream_(readScalar(dict.lookup("UfreeStream")));
	const vector uDirection_(dict.lookup("uDirection"));
	const scalar inputWindHeight_Veg_(readScalar(dict.lookup("inputWindHeight_Veg")));
	const scalar z0_(readScalar(dict.lookup("z0")));
	//const scalar Rd_(readScalar(dict.lookup("Rd")));
    
    
    // now to finally do the calculations and value setting
	scalar ustar = UfreeStream_*0.41/Foam::log((inputWindHeight_Veg_)/z0_);
	scalar ucalc(0.0);

	// Loop over all the faces in the patch
	// and initialize the log profile
	forAll(y, faceI)
	{
		// relative height from ground for face lists
		scalar AGL = y[faceI];

		//Apply the log law equation profile
        ucalc = ustar/0.41*Foam::log((AGL)/z0_);
    	U[faceI] = ucalc*uDirection_;
	}

    // now write the output so the values calculated update
    //  in the file instead of just in the registry
	U.write();
    
    
	Info << "End\n" << endl;    
    return 0;

}


// ************************************************************************* //
