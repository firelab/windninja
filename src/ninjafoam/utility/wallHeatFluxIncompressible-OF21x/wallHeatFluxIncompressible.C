/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2008 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
2010-08-02 Eelco van Vliet: 1st public version of wallHeatFluxIncompressible:
  http://www.cfd-online.com/Forums/openfoam-solving/66705-wallheatflux-bc-not-constant-after-restart.html#post269812

2012-05-21 Eelco van Vliet:
  Quoting: http://www.cfd-online.com/Forums/openfoam-post-processing/101972-wallheatflux-utility-incompressible-case.html#post362191
  «modified the standard wallHeatflux utility which comes default with OF into
  a version for incompressible flows. Also removed a bug out of the code.»

2012-06-26 Eelco van Vliet:
  Quoting: http://www.cfd-online.com/Forums/openfoam-post-processing/101972-wallheatflux-utility-incompressible-case.html#post368330
  «p is now not required anymore.»

-------------------------------------------------------------------------------
License
    This file is a derivative work of OpenFOAM.

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

Application
    wallHeatFluxIncompressible

Description
    Calculates and writes the heat flux for all patches as the boundary field
    of a volScalarField and also prints the integrated flux for all wall
    patches.
    Based on wallHeatFlux with changes to allow it on incompressible flows
    Also removed a bug at the typeid checkline
\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
// modified from  wallHeatFlux
#include "singlePhaseTransportModel.H"
#include "turbulenceModel.H"
#include "wallFvPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
    #include "setRootCase.H"
#   include "createTime.H"
    instantList timeDirs = timeSelector::select0(runTime, args);
#   include "createMesh.H"

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
        Info<< "Time = " << runTime.timeName() << endl;
        mesh.readUpdate();

#       include "createFields.H"
#       include "readTransportProperties.H"

         // update the turbulence fields
		//Info << "\nReading turbulence properties\n";
        turbulence->read();
		
		//Info << "Determining needed kappat\n";
        if (!(IOobject("kappat", runTime.timeName(), mesh).headerOk()))
        {
            Info<< "\nCalculating turbulent heat conductivity " << endl;
            kappat = turbulence->nut()/Prt;
            kappat.correctBoundaryConditions();
        }
        else
        {
            Info<< "\nRead turbulent heat conductivity kappat" << endl;
        }

		//Info << "Determining needed kappaEff\n";
        if (!(IOobject("kappaEff", runTime.timeName(), mesh).headerOk()))
        {
            Info<< "\nCalculating effective heat conductivity " << endl;
            kappaEff=turbulence->nu()/Pr+kappat;
        }
        else
        {
            Info<< "\nRead effective heat conductivity kappaEff" << endl;
        }

		//Info << "\nCalculating gradT\n";
        gradT=fvc::snGrad(T);

		//Info << "Calculating heatFlux\n";
        surfaceScalarField heatFlux =fvc::interpolate(kappaEff*Cp0*rho0)*gradT;

        const surfaceScalarField::GeometricBoundaryField& patchGradT =
                 gradT.boundaryField();
          
        const surfaceScalarField::GeometricBoundaryField& patchHeatFlux =
                 heatFlux.boundaryField();
//
        Info<< "\nWall heat fluxes " << endl;
        forAll(patchHeatFlux, patchi)
        {
           if (typeid(mesh.boundary()[patchi]) == typeid(wallFvPatch))
            {
                Info<< mesh.boundary()[patchi].name()
                    << ": Total "
                    << sum
                       (
                           mesh.magSf().boundaryField()[patchi]
                          *patchHeatFlux[patchi]
                       )
                    << " [W] over "
                    << sum
                       (
                           mesh.magSf().boundaryField()[patchi]
                       )
                    << " [m2] ("
                    << sum
                       (
                           mesh.magSf().boundaryField()[patchi]
                          *patchHeatFlux[patchi]
                       )/
                       sum 
                       (
                           mesh.magSf().boundaryField()[patchi]
                       )
                    << " [W/m2])"
                    << endl;
            }
      }
      Info<< endl;

      
      volScalarField wallHeatFlux
        (
            IOobject
            (
                "wallHeatFlux",
                runTime.timeName(),
                mesh
            ),
            mesh,
            dimensionedScalar("wallHeatFlux", heatFlux.dimensions(), 0.0)
        );

      volScalarField wallGradT
        (
            IOobject
            (
                "wallGradT",
                runTime.timeName(),
                mesh
            ),
            mesh,
            dimensionedScalar("wallGradT", gradT.dimensions(), 0.0)
        );
   
      forAll(wallHeatFlux.boundaryField(), patchi)
      {
         wallHeatFlux.boundaryField()[patchi] = patchHeatFlux[patchi];
      }

      forAll(wallGradT.boundaryField(), patchi)
      {
         wallGradT.boundaryField()[patchi] = patchGradT[patchi];
      }


      wallGradT.write();
      gradT.write();
      wallHeatFlux.write();
      kappaEff.write();
    }

    Info<< "End" << endl;

    return 0;
}

// ************************************************************************* //
