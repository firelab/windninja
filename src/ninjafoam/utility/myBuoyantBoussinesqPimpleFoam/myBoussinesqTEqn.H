{
    kappat = turbulence->nut()/Prt;
    kappat.correctBoundaryConditions();

	// I think that this is the constructor from GeometricField where you give it a word and a GeometricField. The idea is that it constructs as a copy, reseting the field name
    //volScalarField kappaEff("kappaEff", turbulence->nu()/Pr + kappat);
	kappaEff = turbulence->nu()/Pr + kappat;	// the tricky part is that nu and nut aren't defined in the createFields.H and I don't want to define them too early for worries of messing up here stuff, so I made a fake value that had the right properties to help out with the dimensions.
	
    fvScalarMatrix TEqn
    (
        fvm::ddt(T)
      + fvm::div(phi, T)
      - fvm::laplacian(kappaEff, T)
    );

    TEqn.relax();
    TEqn.solve();

    rhok = 1.0 - beta*(T - TRef);
}
