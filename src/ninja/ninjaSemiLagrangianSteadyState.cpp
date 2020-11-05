/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Steady state semi-lagrangian solver
 * Author:   Jason Forthofer
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "ninjaSemiLagrangianSteadyState.h"

NinjaSemiLagrangianSteadyState::NinjaSemiLagrangianSteadyState() : ninja()
, currentTime(boost::gregorian::date(2000, 1, 1), boost::posix_time::hours(0), input.ninjaTimeZone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR) 
, conservationOfMassEquation(FiniteElementMethod::conservationOfMassEquation)
, diffusionEquation(FiniteElementMethod::diffusionEquation)
{

}

/**
 * Copy constructor.
 * @param A Copied value.
 */

NinjaSemiLagrangianSteadyState::NinjaSemiLagrangianSteadyState(NinjaSemiLagrangianSteadyState const& A ) : ninja(A), U00(A.U00), transport(A.transport)
, currentTime(boost::local_time::not_a_date_time)
, conservationOfMassEquation(FiniteElementMethod::conservationOfMassEquation)
, diffusionEquation(FiniteElementMethod::diffusionEquation)
{

}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

NinjaSemiLagrangianSteadyState& NinjaSemiLagrangianSteadyState::operator= (NinjaSemiLagrangianSteadyState const& A)
{
    if(&A != this) {
        ninja::operator=(A);
        U00 = A.U00;
        transport = A.transport;
        conservationOfMassEquation = A.conservationOfMassEquation;
        diffusionEquation = A.diffusionEquation;
    }
    return *this;
}

NinjaSemiLagrangianSteadyState::~NinjaSemiLagrangianSteadyState()
{
    deleteDynamicMemory();
}

/**Method to start a wind simulation.
 * This is the method used to start the "number crunching" part of a simulation.
 * WindNinjaInputs should be completely filled in before running this method.
 * @return Returns true if simulation completes without error.
 */
bool NinjaSemiLagrangianSteadyState::simulate_wind()
{
    checkCancel();

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reading elevation file...");

    readInputFile();
    set_position();
    set_uniVegetation();

    checkInputs();

    if(!input.ninjaTime.is_not_a_date_time())
    {
        std::ostringstream out;
        out << "Simulation time is " << input.ninjaTime;
        input.Com->ninjaCom(ninjaComClass::ninjaNone, out.str().c_str());
    }

#ifdef _OPENMP
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.",
            input.inputsRunNumber, input.numberCPUs);
#endif

#ifdef _OPENMP
    startTotal = omp_get_wtime();
#endif

/*  ----------------------------------------*/
/*  USER INPUTS                             */
/*  ----------------------------------------*/
    int MAXITS = 100000;             //MAXITS is the maximum number of iterations in the solver
    double stop_tol = 1E-1;          //stopping criteria for iterations (2-norm of residual)
    int print_iters = 10;          //Iterations to print out

/*  ----------------------------------------*/
/*  MESH GENERATION                         */
/*  ----------------------------------------*/

#ifdef _OPENMP
    startMesh = omp_get_wtime();
#endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating mesh...");
    //generate mesh
    mesh.buildStandardMesh(input);

    //u is positive toward East
    //v is positive toward North
    //w is positive up
    U.allocate(&mesh);  //REMEMBER TO MODIFY FINITELEMENTMETHOD.COMPUTE_UVW() BECAUSE IT CURRENTLY ALLOCATES U!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111
    U0.allocate(&mesh);
    if(transport.transportType == TransportSemiLagrangian::settls)
        U00.allocate(&mesh);

#ifdef _OPENMP
    endMesh = omp_get_wtime();
#endif

/*  ----------------------------------------*/
/*  VELOCITY INITIALIZATION                 */
/*  ----------------------------------------*/

#ifdef _OPENMP
    startInit = omp_get_wtime();
#endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initializing flow...");

    //initialize
    //----------------------------COULD CHANGE THIS TO INITIALIZE USING REGULAR MASS CONSERVING SIMULATION OUTPUT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    init.reset(initializationFactory::makeInitialization(input));
    init->initializeFields(input, mesh, U0, CloudGrid);
    U00 = U0;


    /////////////Test/////////////////////////////////--------------------------------------------------------------
    volVTK VTK_test(U0, mesh.XORD, mesh.YORD, mesh.ZORD,
                    input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, "test.vtk");
    printf("here\n");



    //////////////////////////////////////////////////---------------------------------------------------------------








#ifdef _OPENMP
    endInit = omp_get_wtime();
#endif

    checkCancel();

    wn_3dVectorField Uaverage(U);
    Uaverage = 0.0;

    /*  ----------------------------------------*/
    /*  CHECK FOR "NULL" RUN                    */
    /*  ----------------------------------------*/
    //Don't do actual simulation if it's a run with all zero velocity.
    //We check the whole field for all zeros, but for this solver probably only need to check boundary conditions.
    if(!checkForNullRun())    
    {
        /*  ----------------------------------------------------*/
        /*  BUILD INITIAL FEM ARRAYS, ALLOCATE MEMORY, ETC.     */
        /*  ----------------------------------------------------*/

#ifdef _OPENMP
        startBuildEq = omp_get_wtime();
#endif

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Building equations...");

        //build A arrray
        conservationOfMassEquation.Initialize(mesh, input, U0);
        conservationOfMassEquation.SetupSKCompressedRowStorage();
        conservationOfMassEquation.SetStability(input, CloudGrid, init);
        conservationOfMassEquation.Discretize();
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Setting boundary conditions...");
        conservationOfMassEquation.SetBoundaryConditions();

        checkCancel();

        /*  -------------------------------------------------------------*/
        /*  DO ONE CONSERVATION OF MASS RUN BEFORE TIME STEPPING LOOP    */
        /*  -------------------------------------------------------------*/
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "First project...");
        //if the CG solver diverges, try the minres solver
        if(conservationOfMassEquation.Solve(input, MAXITS, print_iters, stop_tol)==false)
            if(conservationOfMassEquation.SolveMinres(input, MAXITS, print_iters, stop_tol)==false)
                throw std::runtime_error("Solver returned false.");
 
        //compute uvw field from phi field
        conservationOfMassEquation.ComputeUVWField(input, U);
        //use output from first projection step for copying inlet nodes later on
        U0=U;
        volVTK VTK2(U0, mesh.XORD, mesh.YORD, mesh.ZORD, 
        input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, "U0.vtk");

        //copy U field to new U1 field which will store the new velocity from the advection step
        wn_3dVectorField U1(U);

        /*  ----------------------------------------*/
        /*  START TIME STEPPING LOOP                */
        /*  ----------------------------------------*/
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Starting iteration loop...");
        iteration = 0;
        currentDt = boost::posix_time::seconds(int(get_meshResolution()/U.getMaxValue()));
        //currentDt = boost::posix_time::seconds(1);
        while(iteration <= 1000)
        {
            iteration += 1;
            cout<<"Iteration: "<<iteration<<endl;
            currentDt0 = currentDt;
            currentTime += currentDt;

            // Do semi-lagrangian steps of:
            //  1. Refresh boundary conditions (?)
            //  2. Add body forces (buoyancy)
            //  3. Transport
            //  4. Diffuse
            //  5. Project

            /*  ----------------------------------------*/
            /*  REFRESH BOUNDARY CONDITIONS             */
            /*  ----------------------------------------*/
            checkCancel();
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Refresh boundary conditions...");
            //copy inlet values from initial field U0 to U
            U.copyInletNodes(U0);

#ifdef _OPENMP
            endBuildEq = omp_get_wtime();
#endif

            /*  ----------------------------------------*/
            /*  ADD BODY FORCES                         */
            /*  ----------------------------------------*/
            checkCancel();
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Add body forces...");
            checkCancel();

            /*  ----------------------------------------*/
            /*  TRANSPORT                               */
            /*  ----------------------------------------*/
            checkCancel();
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Transport...");
            cout<<"currentDt = "<<currentDt.total_microseconds()/1000000.0<<endl;
            transport.transportVector(U, U1, currentDt.total_microseconds()/1000000.0);

            /*  ----------------------------------------*/
            /*  DIFFUSE                                 */
            /*  ----------------------------------------*/
            //checkCancel();
            //input.Com->ninjaCom(ninjaComClass::ninjaNone, "Diffuse...");
            ////resets mesh, input, and U0_ in finiteElementMethod
            //diffusionEquation.Initialize(mesh, input, U1); //U1 is output from advection step
            ////diffusionEquation.SetCurrentDt(boost::posix_time::seconds(6));
            //diffusionEquation.SetCurrentDt(currentDt);
            //diffusionEquation.DiscretizeDiffusion();
            //diffusionEquation.SolveDiffusion(U); //dump diffusion results into U
           
            //FOR TESTING WITHOUT DIFFUSION ONLY, REMOVE WHEN DIFFUSION IS TURNED ON 
            U=U1;

            /*  ----------------------------------------*/
            /*  PROJECT                                 */
            /*  ----------------------------------------*/
//            if(iteration >= 999)   //TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//            {
            checkCancel();
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Project...");
            //resets mesh, input, and U0 in finiteElementMethod
            conservationOfMassEquation.Initialize(mesh, input, U);
            conservationOfMassEquation.Discretize();
            conservationOfMassEquation.SetBoundaryConditions();
#ifdef _OPENMP
            startSolve = omp_get_wtime();
#endif
            if(conservationOfMassEquation.Solve(input, MAXITS, print_iters, stop_tol)==false)   //if the CG solver diverges, try the minres solver
                if(conservationOfMassEquation.SolveMinres(input, MAXITS, print_iters, stop_tol)==false)
                    throw std::runtime_error("Solver returned false.");

#ifdef _OPENMP
            endSolve = omp_get_wtime();
#endif

            checkCancel();

            //compute uvw field from phi field
            conservationOfMassEquation.ComputeUVWField(input, U);
//            }   //TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            /*  ----------------------------------------*/
            /*  WRITE OUTPUTS                           */
            /*  ----------------------------------------*/
            //input.simulationOutputFrequency

            //update the aveage velocity field
            for(int k=0;k<U.vectorData_x.mesh_->nlayers;k++)
            {
                for(int i=0;i<U.vectorData_x.mesh_->nrows;i++)
                {
                    for(int j=0;j<U.vectorData_x.mesh_->ncols;j++)
                    {
                        Uaverage.vectorData_x(i,j,k) += U.vectorData_x(i,j,k);
                        Uaverage.vectorData_y(i,j,k) += U.vectorData_y(i,j,k);
                        Uaverage.vectorData_z(i,j,k) += U.vectorData_z(i,j,k);
                    }
                }
            }

            int mod = 1;
            std::ostringstream filename;
            if(iteration % mod == 0)
            {
                filename << "vtk_" << iteration << ".vtk";
                volVTK VTK(U, mesh.XORD, mesh.YORD, mesh.ZORD, 
                input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, filename.str());
            }
        }

        //compute the final average velocity field
        for(int k=0;k<U.vectorData_x.mesh_->nlayers;k++)
        {
            for(int i=0;i<U.vectorData_x.mesh_->nrows;i++)
            {
                for(int j=0;j<U.vectorData_x.mesh_->ncols;j++)
                {
                    Uaverage.vectorData_x(i,j,k) /= iteration;
                    Uaverage.vectorData_y(i,j,k) /= iteration;
                    Uaverage.vectorData_z(i,j,k) /= iteration;
                }
            }
        }
        volVTK VTK(Uaverage, mesh.XORD, mesh.YORD, mesh.ZORD, 
        input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, "Uaverage.vtk");
    }

    checkCancel();

/*  ----------------------------------------*/
/*  PREPARE OUTPUT                          */
/*  ----------------------------------------*/

#ifdef _OPENMP
    startWriteOut = omp_get_wtime();
#endif

    //prepare output arrays
    prepareOutput(&Uaverage);

    checkCancel();

/*  ----------------------------------------*/
/*  WRITE OUTPUT FILES                      */
/*  ----------------------------------------*/

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing output files...");

    //write output files
    writeOutputFiles();

#ifdef _OPENMP
    endWriteOut = omp_get_wtime();
    endTotal = omp_get_wtime();
#endif

/*  ----------------------------------------*/
/*  WRAP UP...                              */
/*  ----------------------------------------*/

    //write timers
#ifdef _OPENMP
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Meshing time was %lf seconds.",endMesh-startMesh);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initialization time was %lf seconds.",endInit-startInit);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Equation building time was %lf seconds.",endBuildEq-startBuildEq);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solver time was %lf seconds.",endSolve-startSolve);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Output writing time was %lf seconds.",endWriteOut-startWriteOut);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Total simulation time was %lf seconds.",endTotal-startTotal);
#endif

     input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d done!", input.inputsRunNumber);

    //If its a pointInitialization Run, explicitly set run completion to 100 when they finish
    //for some reason this doesn't happen automatically
    if(input.initializationMethod == WindNinjaInputs::pointInitializationFlag)
    {
        if(input.matchWxStations == true)
        {
            int time_percent_complete=100;
            input.Com->ninjaCom(ninjaComClass::ninjaOuterIterProgress, "%d",(int) (time_percent_complete+0.5));
        }
    }

    deleteDynamicMemory();
    if(!input.keepOutGridsInMemory)
    {
        AngleGrid.deallocate();
        VelocityGrid.deallocate();
        CloudGrid.deallocate();
    }
    return true;
}

/**Deletes allocated dynamic memory.
 *
 */
void NinjaSemiLagrangianSteadyState::deleteDynamicMemory()
{
    ninja::deleteDynamicMemory();

    U00.deallocate();
}

