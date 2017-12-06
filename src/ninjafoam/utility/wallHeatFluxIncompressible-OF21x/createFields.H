//    Info << "Reading field p\n" << endl;
//    volScalarField p
//    (
//        IOobject
//        (
//            "p",
//            runTime.timeName(),
//            mesh,
//            IOobject::READ_IF_PRESENT,
//            IOobject::AUTO_WRITE
//        ),
//        mesh
//    );

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


    Info << "Reading field Q\n" << endl;
    volScalarField Q
    (
        IOobject
        (
            "Q",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("Q", dimMass/(dimLength*dimTime*dimTime*dimTime),0)
    );

    Info << "Reading field U\n" << endl;
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


    Info << "Creating field kappaEff\n" << endl;
    volScalarField kappaEff
    (
        IOobject
        (
            "kappaEff",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("kappaEff", dimLength*dimLength/dimTime,0)
    );

    Info << "Creating field kappat\n" << endl;
    volScalarField kappat
    (
        IOobject
        (
            "kappat",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("kappat", dimLength*dimLength/dimTime,0)
    );


    surfaceScalarField gradT
    (
        IOobject
        (
            "gradT",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("gradT", dimTemperature/dimLength,scalar(0))
    );


#   include "createPhi.H"

//    label pRefCell = 0;
//    scalar pRefValue = 0.0;
//    setRefCell(p, mesh.solutionDict().subDict("SIMPLE"), pRefCell, pRefValue);

	Info << "Creating laminarTransportModel\n";
    singlePhaseTransportModel laminarTransport(U, phi);

    autoPtr<incompressible::turbulenceModel> turbulence
    (
        incompressible::turbulenceModel::New(U, phi, laminarTransport)
    );

	//Info << "Reading turbulence properties\n";
    turbulence->read();

	Info << "Correcting T boundaries (updateCoeffs for wall function)\n";
    //Correct boundaries only after all fields are loaded in
    T.correctBoundaryConditions();
