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
, currentTime(boost::gregorian::date(2000, 1, 1), boost::posix_time::hours(0), 
              input.ninjaTimeZone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR) 
{
    eDiffusionDiscretizationType diffusionType = getDiffusionDiscretizationType("explicitLumpedCapacitance");
    //eDiffusionDiscretizationType diffusionType = getDiffusionDiscretizationType("implicitCentralDifference");

    if(diffusionType == explicitLumpedCapacitance){
        diffusionEquation = new ExplicitLumpedCapacitanceDiffusion();
    }
    else if(diffusionType == implicitCentralDifference){
        diffusionEquation = new ImplicitCentralDifferenceDiffusion();
    }
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

NinjaSemiLagrangianSteadyState::NinjaSemiLagrangianSteadyState(NinjaSemiLagrangianSteadyState const& A ) 
: ninja(A)
, transport(A.transport)
, U_1(A.U_1)
, U_0(A.U_0)
, U_00(A.U_00)
, currentTime(boost::local_time::not_a_date_time)
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
        U_1 = A.U_1;
        U_0 = A.U_0;
        U_00 = A.U_00;
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
/*  MESH GENERATION                         */
/*  ----------------------------------------*/

#ifdef _OPENMP
    startMesh = omp_get_wtime();
#endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating mesh...");

    mesh.buildStandardMesh(input);

    //u is positive toward East
    //v is positive toward North
    //w is positive up
    U.allocate(&mesh); //current/final wind field
    U0.allocate(&mesh); //initial wind field
    U_1.allocate(&mesh); 
    U_0.allocate(&mesh);
    if(transport.transportType == TransportSemiLagrangian::settls)
        U_00.allocate(&mesh);

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
    //U_00 = U0;

    /////////////Test/////////////////////////////////--------------------------------------------------------------
    //volVTK VTK_test(U0, mesh.XORD, mesh.YORD, mesh.ZORD,
    //                input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, "test.vtk");

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

        //--------COM EQUATION-----------
        conservationOfMassEquation.Initialize(mesh, input);
        conservationOfMassEquation.SetAlphaCoefficients(input, CloudGrid, init);
	conservationOfMassEquation.SetInitialVelocity(U0);
        conservationOfMassEquation.Discretize();
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Setting boundary conditions...");
        conservationOfMassEquation.SetBoundaryConditions();

        //--------DIFFUSION EQUATION-----------
        diffusionEquation->Initialize(&mesh, &input);

        checkCancel();

        /*  -------------------------------------------------------------*/
        /*  DO ONE CONSERVATION OF MASS RUN BEFORE TIME STEPPING LOOP    */
        /*  -------------------------------------------------------------*/
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "First project...");

        conservationOfMassEquation.Solve(input);
 
        //compute uvw field from phi field
        //TODO: have ComputeUVWField take a reference to a wn_3dVectorField &U to fill in, rather than
        //returning a value so we are consistent with the behavior in the diffusion step
        U = conservationOfMassEquation.ComputeUVWField();

        //use output from first projection step for copying inlet nodes later on
        U0=U;
        volVTK VTK2(U0, mesh.XORD, mesh.YORD, mesh.ZORD, 
        input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, "U0.vtk");

        //copy U field to new U_1 field which will store the new velocity from the advection step
        U_1 = U;

        /*  ----------------------------------------*/
        /*  START TIME STEPPING LOOP                */
        /*  ----------------------------------------*/
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Starting iteration loop...");
        iteration = 0;
        bool with_advection = true;
        bool with_diffusion = true;
        bool with_projection = true;

        while(iteration <= 1000)
        {
            currentDt = boost::posix_time::seconds(int(get_meshResolution()/U.getMaxValue()));
            //currentDt = boost::posix_time::seconds(5);
            currentDt0 = currentDt;
            currentTime += currentDt;

            cout<<"Iteration: "<<iteration<<endl;
            cout<<"currentDt = "<<currentDt.total_microseconds()/1000000.0<<endl;

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

            //TESTING------------------------------------------
            //for(int i=0;i<input.dem.get_nRows();i++)
            //{
            //    for(int j=0;j<input.dem.get_nCols();j++)
            //    {
            //        for(int k=0;k<mesh.nlayers;k++)
            //        {
            //            //if(iteration < 20){
            //                if(i>20 && i<30 && j>20 && j<30 && k<5 && k>2)
            //                {
            //                    U.vectorData_z(i, j, k) += 8.0;
            //                }
            //           // }
            //        }
            //    }
            //}

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
            if(with_advection){
                checkCancel();
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Transport...");
                transport.transportVector(U, U_1, currentDt.total_microseconds()/1000000.0);

                //testing
                volVTK VTK_transport(U_1, mesh.XORD, mesh.YORD, mesh.ZORD, 
                input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, "vtk_transport.vtk");
            }

            /*  ----------------------------------------*/
            /*  DIFFUSE                                 */
            /*  ----------------------------------------*/
            if(with_diffusion){
                checkCancel();
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Diffuse...");

                //U_1 is output from advection step, dump diffusion results into U
                diffusionEquation->Solve(U_1, U, currentDt); 

                int mod_ = 1;
                std::ostringstream diff_fname;
                if(iteration % mod_ == 0)
                {
                    diff_fname << "vtk_diffusion" << iteration << ".vtk";
                    volVTK VTK_diff(U, mesh.XORD, mesh.YORD, mesh.ZORD, 
                    input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, diff_fname.str());
                }
            }
            else{
                //FOR TESTING WITHOUT DIFFUSION ONLY
                U=U_1;
            }

            /*  ----------------------------------------*/
            /*  PROJECT                                 */
            /*  ----------------------------------------*/
            if(with_projection){
                checkCancel();
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Project...");

                //Debugging finite element solver--------------------------------
                //write PHI and RHS for debugging
                std::ostringstream phi_fname;
                phi_fname << "PHI_" << iteration << ".vtk";
                conservationOfMassEquation.phiOutFilename = phi_fname.str();
                std::ostringstream rhs_fname;
                rhs_fname << "RHS_" << iteration << ".vtk";
                conservationOfMassEquation.rhsOutFilename = rhs_fname.str();
                conservationOfMassEquation.WritePHIandRHS();
                //---------------------------------------------------------------

                //set ground to zero
                for(int i=0; i<U.vectorData_x.mesh_->nrows; i++)
                {
                    for(int j=0; j<U.vectorData_x.mesh_->ncols; j++)
                    {
                        U.vectorData_x(i,j,0) = 0.0;
                        U.vectorData_y(i,j,0) = 0.0;
                        U.vectorData_z(i,j,0) = 0.0;
                    }
                }
                conservationOfMassEquation.SetAlphaCoefficients(input, CloudGrid, init);
                conservationOfMassEquation.SetInitialVelocity(U);
                conservationOfMassEquation.Discretize();
                conservationOfMassEquation.SetBoundaryConditions();
#ifdef _OPENMP
                startSolve = omp_get_wtime();
#endif
                conservationOfMassEquation.Solve(input);

#ifdef _OPENMP
                endSolve = omp_get_wtime();
#endif
                //compute uvw field from phi field
                U = conservationOfMassEquation.ComputeUVWField();

                checkCancel();
            }

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

            iteration += 1;
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
    prepareOutput(Uaverage);

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

NinjaSemiLagrangianSteadyState::eDiffusionDiscretizationType NinjaSemiLagrangianSteadyState::getDiffusionDiscretizationType(std::string type)
{
    if(type == "explicitLumpedCapacitance")
            return explicitLumpedCapacitance;
    else if(type == "implicitCentralDifference")
        return implicitCentralDifference;
    else
        throw std::runtime_error(std::string("Cannot determine diffusion discretization type in" 
                    "ninjaSemiLagrangianSteadyState::getDiffusionDiscretizationType()."));
}

/**Deletes allocated dynamic memory.
 *
 */
void NinjaSemiLagrangianSteadyState::deleteDynamicMemory()
{
    ninja::deleteDynamicMemory();

    U_00.deallocate();
    U_0.deallocate();
    U_1.deallocate();

    //TODO: The below causes a segfault, double check how this should be handled
    //if(diffusionEquation)
    //{	
    //    delete diffusionEquation;
    //    diffusionEquation=NULL;
    //}
}

