    // Thermal expansion coefficient [1/K]
    dimensionedScalar Pr(laminarTransport.lookup("Pr"));

    // Turbulent Prandtl number
    dimensionedScalar Prt(laminarTransport.lookup("Prt"));

    // Heat capacity
    dimensionedScalar Cp0(laminarTransport.lookup("Cp0"));

    // Fluid density
    dimensionedScalar rho0(laminarTransport.lookup("rho0"));
