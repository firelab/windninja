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
	myApplyInit

Description
	Apply a simple temperature field initial condition to all inner cells.
	Based on the simple correlation that temperature drops by 0.0098 K/m in the z direction
	for dry air when in the planetary boundary layer of the troposphere.

    Alternatively, you can comment that part out and uncomment another part that
	sets the pressure and temperature using a potential temperature equation.
	This assumes a standard pressure variation with height.
	
	This script is very useful for understanding how to mess with OpenFoam code/syntax.
	Also makes it easy to find some useful functions for setting values.
	Finally, it can be adjusted to set any value/property in the mesh according to 
	whatever profile that is based on height.

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
	// this is what I used to do. I like this, but apparently a potential temperature field is better
	// note that even when not accessing p_rgh, it rewrites some BC condition field stuff from being a value to a gradient, which happens also if you run a simulation, the value turns to gradient.
	#include "setRootCase.H"

	#include "createTime.H"
    #include "createMesh.H"

	volScalarField T
	(
		IOobject
		(
			"T",
			runTime.timeName(),
			mesh,
			IOobject::MUST_READ,
			IOobject::AUTO_WRITE
		),
		mesh
	);
	
	Info<< "Calculating lapse rate profile for T" << endl;

	// calculating maxSurfaceT for use in generating the lapse rate
	label minZpatchID = mesh.boundaryMesh().findPatchID("minZ");
	const scalarField& minZTvalues = T.boundaryField()[minZpatchID];
	scalar TsurfMax = 0;
	forAll(minZTvalues,facei)
	{
		scalar Tvalue = minZTvalues[facei];
		if(Tvalue > TsurfMax)
		{
			TsurfMax = Tvalue;
		}
	} 

	std::cout << "TsurfMax = " << TsurfMax << "\n";
	if(TsurfMax == 0)
	{
		std::cout << "changing TsurfMax to value of 310\n";
		TsurfMax = 310;
	}
	scalar Tcalc(0.0);
	scalar TlapseRate = 0.0098;
	std::cout << "TlapseRate = " << TlapseRate << "\n";

	volScalarField z = wallDist(mesh).y();	//distance to nearest wall zmin. So height above z0 for each z point. Beware if there are other walls than minZ
	//volScalarField z(mesh.C().component(2)); //z-component of cell center. So height above lowest z point, not height above z0 for each z point.
	//looks like you can replace wallDist with patchDistMethod for other options

	// Loop over all the faces in the patch
	// and initialize the log profile
	forAll(z, cellI)
	{
		// relative height from ground for face lists
		scalar AGL = z[cellI];	//I think that AGL is the height of the cell

		//Apply formula to get profile. Make sure that the AGL is corrected for the minZ height
		Tcalc = TsurfMax - TlapseRate*AGL;
    	T[cellI] = Tcalc;
	}
	T.write();
	
	
	
/*
	okay, so if I am going to try to set a potential temperature instead of a temperature
	with that equation, then I'm going to need to first set pressure.
		Use P2 = P1 + rho*g*(Z2-Z1), adapting for number of fluid types (changing rho).
	Then calculate p_rgh from this pressure field. Finally, this all depends on the temperature
	So since there's this interdependence, would need to calculate rhok from T eqn and redo P
	and p_rgh, then redo T iteratively.
	Finally, potential temperature is the following phi = T * (Pnot/P)^(R/Cp)

	To avoid this iterative process, assume rhok = rho at Tref for now
*/
/*
	Info<< "Calculating innerField profile for T using potential temperature" << endl;
	Info<< "This involves setting inner fields for p and p_rgh" << endl;

    #include "setRootCase.H"

	#include "createTime.H"
    #include "createMesh.H"

	Info<< "Reading field T\n" << endl;
	volScalarField T
	(
		IOobject
		(
			"T",
			runTime.timeName(),
			mesh,
			IOobject::MUST_READ,
			IOobject::AUTO_WRITE
		),
		mesh
	);
	
	Info<< "Reading field p\n" << endl;
	volScalarField p
	(
		IOobject
		(
			"p",
			runTime.timeName(),
			mesh,
			IOobject::MUST_READ,
			IOobject::AUTO_WRITE
		),
		mesh
	);
	
	Info<< "Reading field p_rgh\n" << endl;
	volScalarField p_rgh
	(
		IOobject
		(
			"p_rgh",
			runTime.timeName(),
			mesh,
			IOobject::MUST_READ,
			IOobject::AUTO_WRITE
		),
		mesh
	);

	//volScalarField z_innerField = wallDist(mesh).y();	//distance to nearest wall zmin. So height above z0 for each z point. Beware if there are other walls than minZ. Notice that this is only for the inner field!
	volScalarField z_innerField(mesh.C().component(2)); //z-component of cell center. So height above lowest z point, not height above z0 for each z point.
	
	dimensionedScalar phi_s
	(
	"phi_s",
	dimensionSet(0,0,0,1,0,0,0),
	scalar(280)
	);

	dimensionedScalar gamma
	(
	"gamma",
	dimensionSet(0,-1,0,1,0,0,0),
	scalar(0.0032)
	);

	dimensionedScalar deltaphi
	(
	"deltaphi",
	dimensionSet(0,0,0,1,0,0,0),
	scalar(5)
	);

	dimensionedScalar beta
	(
	"beta",
	dimensionSet(0,-1,0,0,0,0,0),
	scalar(0.002)
	);

	Info<< "Reading field U\n" << endl;
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
	
	#include "createPhi.H"

	singlePhaseTransportModel laminarTransport(U, phi);
	// Thermal expansion coefficient [1/K]
    //dimensionedScalar beta(laminarTransport.lookup("beta"));
	//dimensionedScalar TRef(laminarTransport.lookup("TRef"));
	// Reference density
    dimensionedScalar rho(laminarTransport.lookup("rho"));

	// get g
	//#include "readGravitationalAcceleration.H"
	// because standard g seems to be hard to get the right values/dimensional math, going to do this
	dimensionedScalar g
	(
	"g",
	dimensionSet(0,1,-2,0,0,0,0),
	scalar(-9.81)
	);
	
	dimensionedScalar p1 
	(
	"p1",
	dimensionSet(1,-1,-2,0,0,0,0),
	scalar(101325)
	); 	// Pa, assumes height 0 is sea level
	// Pa is N/m^2. N = kg*m/s^2. So Pa is kg/(m*s^2). So going to have to use density in equations
	dimensionedScalar z1
	(
	"z1",
	dimensionSet(0,1,0,0,0,0,0),
	scalar(0)
	);			// m, height 0
	
	// if you were iteratively setting the values use this chunk of code somewhere
	//Info<< "Creating turbulence model to get rhok\n" << endl;
    //autoPtr<incompressible::RASModel> turbulence
    //(
    //    incompressible::RASModel::New(U, phi, laminarTransport)
    //);
	//dimensionedScalar rhok = 1.0 - beta*(T - TRef); // need to calculate T field before this. Use this for iterative tricks
	
	// Loop over all the faces in the patch
	// and initialize the temperature profile
	// find Tmin for setting minZ temperature
	dimensionedScalar Tmin
	(
	"Tmin",
	dimensionSet(0,0,0,1,0,0,0),
	scalar(99999999) 	// set to huge value initially
	);

	Info<< "Calculating values for p, p_rgh, phi, and T\n" << endl;
	forAll(z_innerField, cellI)
	{
		// relative height from ground for face lists
		dimensionedScalar z
		(
		"z",
		dimensionSet(0,1,0,0,0,0,0),
		scalar(z_innerField[cellI])
		);
		//dimensionedScalar z = z_innerField[cellI];	//I think that AGL is the height of the cell
		
		dimensionedScalar pCalc = p1+rho*g*z;
		dimensionedScalar p_rghCalc = pCalc - rho*g*z;
		dimensionedScalar phiCalc = phi_s + gamma*z + deltaphi*(1 - exp(-beta*z));
		
		//Apply formula to get profile. Make sure that the AGL is corrected for the minZ height
		dimensionedScalar Tcalc = phiCalc*pow(pCalc/p1,0.286);
		if(Tcalc < Tmin)
		{
			Tmin = Tcalc;	// I don't actually use this in this setup though, since I'm setting minZ faces using the innerField cell values, but you can use this if you want to use minZ as Tmin instead.
		}
		// divide by rho to get the right units
		p[cellI] = (pCalc/rho).value();
		p_rgh[cellI] = (p_rghCalc/rho).value();
		T[cellI] = Tcalc.value();
	}
	Info<< "Writing innerField values for p, p_rgh, and T\n" << endl;
	p.write();
	p_rgh.write();	// it keeps overwriting the fixedFluxPressure BCs to have gradient 0 instead of value 0. Technically need the value given by inner field here probably.
	T.write();


	// use this to set minZ to equal the values as given by the inner cells
	// alternatively, comment it out to use Tmin for minZ.
	Info<< "Setting minZ T profile" << endl;
	label minZpatchID = mesh.boundaryMesh().findPatchID("minZ");
	scalarField& minZTpatch = refCast<scalarField>(T.boundaryField()[minZpatchID]);
	// get the cell indices of cells along the patch
	const labelList& minZfaceCells = mesh.boundaryMesh()[minZpatchID].faceCells();
	
	forAll(z_innerField,cellI)
	{
		forAll(minZfaceCells, faceI)
		{
			if(minZfaceCells[faceI] == cellI)
			{
				minZTpatch[faceI] = T[cellI];
				break;
			}
		}
	}
	T.write();
	*/

	// comment this section out if setting minZ T to the values given in the inner fields
	// comment above section out and uncomment this section if you want a single minZ temperature as the smallest temperature in the inner field
	/*Info<< "Setting minZ profile to Tmin. Tmin = " << Tmin.value() << endl;
	
	label minZpatchID = mesh.boundaryMesh().findPatchID("minZ");
	scalarField& minZTpatch = refCast<scalarField>(T.boundaryField()[minZpatchID]);
	forAll(minZTpatch, faceI)
	{
		minZTpatch[faceI] = Tmin.value();
	}
	T.write();*/
	

	/*
// This next section is for setting the inner T values to all other patches and should be commented out unless you are planning on using fixed T values for all boundaries. So comment out this whole section if you want to use zeroGradient instead of fixed value for them. Also note that patches don't get set if the type is zeroGradient instead of fixedValue
	Info << "Setting maxZ T profile" << endl;
	label maxZpatchID = mesh.boundaryMesh().findPatchID("maxZ");
	scalarField& maxZTpatch = refCast<scalarField>(T.boundaryField()[maxZpatchID]);
	// get the cell indices of cells along the patch
	const labelList& maxZfaceCells = mesh.boundaryMesh()[maxZpatchID].faceCells();
	forAll(z_innerField,cellI)
	{
		forAll(maxZfaceCells, faceI)
		{
			if(maxZfaceCells[faceI] == cellI)
			{
				maxZTpatch[faceI] = T[cellI];
				break;
			}
		}
	}
	T.write();
	
	Info << "Setting west_face T profile" << endl;
	label westFacePatchID = mesh.boundaryMesh().findPatchID("west_face");
	scalarField& westFaceTpatch = refCast<scalarField>(T.boundaryField()[westFacePatchID]);
	// get the cell indices of cells along the patch
	const labelList& westFaceFaceCells = mesh.boundaryMesh()[westFacePatchID].faceCells();
	forAll(z_innerField,cellI)
	{
		forAll(westFaceFaceCells, faceI)
		{
			if(westFaceFaceCells[faceI] == cellI)
			{
				westFaceTpatch[faceI] = T[cellI];
				break;
			}
		}
	}
	T.write();
	
	Info << "Setting east_face T profile" << endl;
	label eastFacePatchID = mesh.boundaryMesh().findPatchID("east_face");
	scalarField& eastFaceTpatch = refCast<scalarField>(T.boundaryField()[eastFacePatchID]);
	// get the cell indices of cells along the patch
	const labelList& eastFaceFaceCells = mesh.boundaryMesh()[eastFacePatchID].faceCells();
	forAll(z_innerField,cellI)
	{
		forAll(eastFaceFaceCells, faceI)
		{
			if(eastFaceFaceCells[faceI] == cellI)
			{
				eastFaceTpatch[faceI] = T[cellI];
				break;
			}
		}
	}
	T.write();
	
	Info << "Setting south_face T profile" << endl;
	label southFacePatchID = mesh.boundaryMesh().findPatchID("south_face");
	scalarField& southFaceTpatch = refCast<scalarField>(T.boundaryField()[southFacePatchID]);
	// get the cell indices of cells along the patch
	const labelList& southFaceFaceCells = mesh.boundaryMesh()[southFacePatchID].faceCells();
	forAll(z_innerField,cellI)
	{
		forAll(southFaceFaceCells, faceI)
		{
			if(southFaceFaceCells[faceI] == cellI)
			{
				southFaceTpatch[faceI] = T[cellI];
				break;
			}
		}
	}
	T.write();
	
	Info << "Setting north_face T profile" << endl;
	label northFacePatchID = mesh.boundaryMesh().findPatchID("north_face");
	scalarField& northFaceTpatch = refCast<scalarField>(T.boundaryField()[northFacePatchID]);
	// get the cell indices of cells along the patch
	const labelList& northFaceFaceCells = mesh.boundaryMesh()[northFacePatchID].faceCells();
	forAll(z_innerField,cellI)
	{
		forAll(northFaceFaceCells, faceI)
		{
			if(northFaceFaceCells[faceI] == cellI)
			{
				northFaceTpatch[faceI] = T[cellI];
				break;
			}
		}
	}
	T.write();
	*/

	Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
