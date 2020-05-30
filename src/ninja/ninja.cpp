/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Main class for running WindNinja
 * Author:   Jason Forthofer <jforthofer@gmail.com>
 *
 ******************************************************************************
 * * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
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


#include "ninja.h"

extern boost::local_time::tz_database globalTimeZoneDB;

/**Ninja constructor
 * This is the default ninja constructor.
 */
ninja::ninja()
: conservationOfMass(FiniteElementMethod::conservationOfMassEquation)
{
    cancel = false;
    isNullRun = false;
    maxStartingOuterDiff = -1.0;
    matchTol = 0.22;    //0.22 m/s is about 1/2 mph

    //Timers
    startTotal=0.0;
    endTotal=0.0;
    startMesh=0.0;
    endMesh=0.0;
    startInit=0.0;
    endInit=0.0;
    startBuildEq=0.0;
    endBuildEq=0.0;
    startSolve=0.0;
    endSolve=0.0;
    startWriteOut=0.0;
    endWriteOut=0.0;

    //Pointers to dynamically allocated memory
    uDiurnal=NULL;
    vDiurnal=NULL;
    wDiurnal=NULL;
    height=NULL;
    aspect=NULL;
    slope=NULL;
    shade=NULL;
    solar=NULL;
    nMaxMatchingIters = atoi( CPLGetConfigOption( "NINJA_POINT_MAX_MATCH_ITERS",
                                                  "150" ) );
    CPLDebug( "NINJA", "Maximum match iterations set to: %d", nMaxMatchingIters );

    //ninjaCom stuff
    input.lastComString[0] = '\0';
    input.inputsRunNumber = 0;
    input.inputsComType = ninjaComClass::ninjaDefaultCom;
    input.Com = new ninjaDefaultComHandler();

}

/**Ninja destructor
 *
 */
ninja::~ninja()
{
	deleteDynamicMemory();
        delete input.Com;
}

/**
 * Copy constructor.
 * This copy constructor only works BEFORE the simulation has been done.  It doesn't work properly
 * AFTER a simulation, since it doesn't copy all the dynamic memory allocated during simulate_wind().
 * @param rhs Ninja to be copied.
 */
ninja::ninja(const ninja &rhs)
: AngleGrid(rhs.AngleGrid)
, VelocityGrid(rhs.VelocityGrid)
, CloudGrid(rhs.CloudGrid)
#ifdef EMISSIONS
, DustGrid(rhs.DustGrid)
#endif
#ifdef FRICTION_VELOCITY
, UstarGrid(rhs.UstarGrid)
#endif
, U(rhs.U)
, U0(rhs.U0)
, mesh(rhs.mesh)
, input(rhs.input)
, conservationOfMass(rhs.conservationOfMass)
{
    input.Com = NULL;   //must be set to null!
    set_ninjaCommunication(rhs.get_inputsRunNumber(), rhs.get_inputsComType());
    strcpy( input.lastComString, rhs.get_lastComString() );
    input.Com->fpLog = rhs.get_ComLogFp();


    cancel = rhs.cancel;
    isNullRun = rhs.isNullRun;
    maxStartingOuterDiff = rhs.maxStartingOuterDiff;
    nMaxMatchingIters = rhs.nMaxMatchingIters;
    matchTol = rhs.matchTol;
    num_outer_iter_tries_u = rhs.num_outer_iter_tries_u;
    num_outer_iter_tries_v = rhs.num_outer_iter_tries_v;
    num_outer_iter_tries_w = rhs.num_outer_iter_tries_w;

    //Timers
    startTotal=0.0;
    endTotal=0.0;
    startMesh=0.0;
    endMesh=0.0;
    startInit=0.0;
    endInit=0.0;
    startBuildEq=0.0;
    endBuildEq=0.0;
    startSolve=0.0;
    endSolve=0.0;
    startWriteOut=0.0;
    endWriteOut=0.0;

    //Pointers to dynamically allocated memory
    uDiurnal=NULL;
    vDiurnal=NULL;
    wDiurnal=NULL;
    height=NULL;
    aspect=NULL;
    slope=NULL;
    shade=NULL;
    solar=NULL;
}

/**
 * Equals operator.
 * This equals operator only works BEFORE the simulation has been done.  It doesn't work properly
 * AFTER a simulation, since it doesn't copy all the dynamic memory allocated during simulate_wind().
 * @param rhs ninja object to set equal to.
 * @return A reference to a copied rhs.
 */
ninja &ninja::operator=(const ninja &rhs)
{
    if(&rhs != this)
    {
        AngleGrid = rhs.AngleGrid;
        VelocityGrid = rhs.VelocityGrid;
        CloudGrid = rhs.CloudGrid;
        #ifdef EMISSIONS
        DustGrid = rhs.DustGrid;
        #endif
        #ifdef FRICTION_VELOCITY
        UstarGrid = rhs.UstarGrid;
        #endif
        U = rhs.U;
        U0 = rhs.U0;
        conservationOfMass = rhs.conservationOfMass;

        mesh = rhs.mesh;
        input = rhs.input;

        cancel = rhs.cancel;
        isNullRun = rhs.isNullRun;
        maxStartingOuterDiff = rhs.maxStartingOuterDiff;
        nMaxMatchingIters = rhs.nMaxMatchingIters;
        matchTol = rhs.matchTol;
        num_outer_iter_tries_u = rhs.num_outer_iter_tries_u;
        num_outer_iter_tries_v = rhs.num_outer_iter_tries_v;
        num_outer_iter_tries_w = rhs.num_outer_iter_tries_w;

        //Timers
        startTotal=0.0;
        endTotal=0.0;
        startMesh=0.0;
        endMesh=0.0;
        startInit=0.0;
        endInit=0.0;
        startBuildEq=0.0;
        endBuildEq=0.0;
        startSolve=0.0;
        endSolve=0.0;
        startWriteOut=0.0;
        endWriteOut=0.0;

        //Pointers to dynamically allocated memory
        uDiurnal=NULL;
        vDiurnal=NULL;
        wDiurnal=NULL;
        height=NULL;
        aspect=NULL;
        slope=NULL;
        shade=NULL;
        solar=NULL;
    }
    return *this;
}

/**Method to start a wind simulation.
 * This is the method used to start the "number crunching" part of a simulation.
 * WindNinjaInputs should be completely filled in before running this method.
 * @return Returns true if simulation completes without error.
 */
bool ninja::simulate_wind()
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

    /*
    ** Set matching its from config options, default to 150.
    ** See constructor to set default.
    */
    //maximum number of outer iterations to do (for matching observations)
    int max_matching_iters = nMaxMatchingIters;

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
    U0.allocate(&mesh);

#ifdef _OPENMP
    endMesh = omp_get_wtime();
#endif

/*  ----------------------------------------*/
/*  START OUTER ITERATIVE LOOP FOR          */
/*  MATCHING INPUT POINTS		            */
/*  ----------------------------------------*/

    if(input.initializationMethod == WindNinjaInputs::pointInitializationFlag)
    {
        if(input.matchWxStations == true)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Starting outer wx station \"matching\" loop...");
            //don't print normal solver progress, just "outer iter" "matching" progress
            //If this is commented, it messes with the progress-bar
            input.Com->noSolverProgress();
        }
    }

    int matchingIterCount = 0;
    bool matchFlag = false;
    if(input.matchWxStations == true)
    {
        num_outer_iter_tries_u = std::vector<int>(input.stations.size(),0);
        num_outer_iter_tries_v = std::vector<int>(input.stations.size(),0);
        num_outer_iter_tries_w = std::vector<int>(input.stations.size(),0);
    }

/*  ----------------------------------------*/
/*  VELOCITY INITIALIZATION                 */
/*  ----------------------------------------*/
    do  //start wx station matching do-while loop
    {
        if(input.matchWxStations == true)
        {
            matchingIterCount++;
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "\"matching\" loop iteration %i...",
                    matchingIterCount);
        }

#ifdef _OPENMP
        startInit = omp_get_wtime();
#endif

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initializing flow...");

        //initialize
        init.reset(initializationFactory::makeInitialization(input));
        init->initializeFields(input, mesh, U0, CloudGrid);

#ifdef _OPENMP
        endInit = omp_get_wtime();
#endif

        checkCancel();

/*  ----------------------------------------*/
/*  CHECK FOR "NULL" RUN                    */
/*  ----------------------------------------*/
        if(checkForNullRun())	//if it's a run with all zero velocity...
            break;

/*  ----------------------------------------*/
/*  BUILD "A" ARRAY OF AX=B                 */
/*  ----------------------------------------*/
#ifdef _OPENMP
        startBuildEq = omp_get_wtime();
#endif

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Building equations...");

        //build A arrray
        conservationOfMass.SetupSKCompressedRowStorage(mesh, input);
        conservationOfMass.SetStability(mesh, input, U0, CloudGrid, init);
        conservationOfMass.Discretize(mesh, input, U0);

        checkCancel();

/*  ----------------------------------------*/
/*  SET BOUNDARY CONDITIONS                 */
/*  ----------------------------------------*/

        //set boundary conditions
        conservationOfMass.SetBoundaryConditions(mesh, input);

//#define WRITE_A_B
#ifdef WRITE_A_B	//used for debugging...
        conservationOfMass.Write_A_and_b(1000);
#endif

#ifdef _OPENMP
        endBuildEq = omp_get_wtime();
#endif

        checkCancel();

/*  ----------------------------------------*/
/*  CALL SOLVER                             */
/*  ----------------------------------------*/

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solving...");
#ifdef _OPENMP
        startSolve = omp_get_wtime();
#endif

        //solver
        //if the CG solver diverges, try the minres solver
        if(conservationOfMass.Solve(input, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
            if(conservationOfMass.SolveMinres(input, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
                throw std::runtime_error("Solver returned false.");

#ifdef _OPENMP
        endSolve = omp_get_wtime();
#endif

checkCancel();

/*  ----------------------------------------*/
/*  COMPUTE UVW WIND FIELD                   */
/*  ----------------------------------------*/

        //compute uvw field from phi field
        conservationOfMass.ComputeUVWField(mesh, input, U0, U);

        checkCancel();

        matchFlag = matched(matchingIterCount);

    }while(matchingIterCount<max_matching_iters && !matchFlag);	//end outer iterations is over max_matching_iters or wind field matches wx stations

    if(input.matchWxStations == true && !isNullRun)
    {
        double smallestInfluenceRadius = getSmallestRadiusOfInfluence();

        if(matchFlag == false)
        {
            const char* error;
            error = CPLSPrintf("Solution did not converge to match weather stations.\n" \
            "Sometimes this is caused by a very low radius of influence when compared to the mesh resolution.\n" \
            "Your horizontal mesh resolution is %lf meters and the smallest radius of influence is %.2E meters,\n" \
            "which means that the radius of influence is %.2E cells in distance.\n" \
            "It is usually a good idea to have at least 10 cells of distance (%.2E meters in this case).\n" \
            "If convergence is still not reached, try increasing the radius of influence even more.\n", \
            mesh.meshResolution, smallestInfluenceRadius, 
            smallestInfluenceRadius/mesh.meshResolution, 10.0*mesh.meshResolution);

            input.Com->ninjaCom(ninjaComClass::ninjaWarning, error);
            throw(std::runtime_error(error));
        }
    }

/*  ----------------------------------------*/
/*  COMPUTE FRICTION VELOCITY               */
/*  ----------------------------------------*/
#ifdef FRICTION_VELOCITY
    if(input.frictionVelocityFlag == 1){
#ifdef _OPENMP
        startComputeFrictionVelocity = omp_get_wtime();
#endif
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Computing friction velocities...");
        computeFrictionVelocity();
#ifdef _OPENMP
        endComputeFrictionVelocity = omp_get_wtime();
#endif
    }
#endif

#ifdef EMISSIONS
/*  ----------------------------------------*/
/*  COMPUTE DUST EMISSIONS                  */
/*  ----------------------------------------*/

    if(input.dustFlag == 1){
#ifdef _OPENMP
        startDustEmissions = omp_get_wtime();
#endif
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Computing dust emissions...");
        computeDustEmissions();
#ifdef _OPENMP
        endDustEmissions = omp_get_wtime();
#endif
    }

#endif //EMISSIONS

/*  ----------------------------------------*/
/*  PREPARE OUTPUT                          */
/*  ----------------------------------------*/

#ifdef _OPENMP
    startWriteOut = omp_get_wtime();
#endif

    //prepare output arrays
    prepareOutput();

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
#ifdef FRICTION_VELOCITY
    if(input.frictionVelocityFlag == 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Friction velocity calculation time was %lf seconds.",endComputeFrictionVelocity-startComputeFrictionVelocity);
    }
#endif
#ifdef EMISSIONS
    if(input.dustFlag == 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Dust emissions simulation time was %lf seconds.",endDustEmissions-startDustEmissions);
    }
#endif
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
#ifdef FRICTION_VELOCITY
        if(input.frictionVelocityFlag == 1){
            UstarGrid.deallocate();
        }
#endif
#ifdef EMISSIONS
        if(input.dustFlag == 1){
            DustGrid.deallocate();
        }
#endif
    }
    return true;
}

/**Method used to get the smallest radius of influence from a vector of wxStation.
 *
 * @return Value of smallest radius of influence.
 */
double ninja::getSmallestRadiusOfInfluence()
{
	double smallest = DBL_MAX;
    for(unsigned int i = 0; i < input.stations.size(); i++)
	{
        if(input.stations[i].get_influenceRadius() > 0)
		{
            if(input.stations[i].get_influenceRadius() < smallest)
                smallest = input.stations[i].get_influenceRadius();
		}
	}

	return smallest;
}

/**Interpolates the 3d volume wind field to the output wind height surface.
 *
 */
void ninja::interp_uvw()
{
#pragma omp parallel default(shared)
    {

        int i,j,k;
        double h2, h1=0.0, slopeu, slopev, slopew, uu, vv, ww, intermedval;
        windProfile profile;
        profile.profile_switch = windProfile::monin_obukov_similarity;	//switch that detemines what profile is used...

                                                                        //make sure rough_h is set to zero if profile switch is 0 or 2



        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //            JUST TESTING!!!!!!     DELETE!!!!!

        //Add a blob of stronger wind using a multiplier of existing wind speed
//        double blobWindSpeedMultiplier = 3.0;
//        double blobWindSpeed = 30.0;

//        for(int k=0; k<U.vectorData_x.mesh_->nlayers; k++)
//        {
//            for(int j=0; j<U.vectorData_x.mesh_->ncols; j++)
//            {
//                for(int i=0; i<U.vectorData_x.mesh_->nrows; i++)
//                {
//                    if(j>5 && j<10)
//                    {
//                        if(k>5 && k<10)
//                        {
//                            //U.vectorData_x(i,j,k) = blobWindSpeedMultiplier * U.vectorData_x(i,j,k);
//                            U.vectorData_x(i,j,k) = blobWindSpeed;
//                        }
//                    }
//                }
//            }
//        }

//        double dt = 10.0;
//        wn_3dVectorField U1(U);
//        TransportSemiLagrangian transport;
//        transport.transportVector(U, U1, dt);
//        U = U1;
        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


#pragma omp for
        for(i=0;i<VelocityGrid.get_nRows();i++)
        {
            for(j=0;j<VelocityGrid.get_nCols();j++)
            {
                k=1;
                h2=0;

                while(h2 < (input.outputWindHeight + input.surface.Rough_h(i,j)))
                {
                    assert( k < mesh.nlayers );
                    h2=mesh.ZORD(i, j, k)-mesh.ZORD(i, j, 0);
                    h1=mesh.ZORD(i, j, k-1)-mesh.ZORD(i, j, 0);
                    k++;
                }
                k--;

                if(k <= 1)   //if we're in the first cell, use log profile
                {
                    profile.ObukovLength = init->L(i,j);
                    profile.ABL_height = init->bl_height(i,j);
                    profile.Roughness = input.surface.Roughness(i,j);
                    profile.Rough_h = input.surface.Rough_h(i,j);
                    profile.Rough_d = input.surface.Rough_d(i,j);
                    profile.inputWindHeight = h2 - input.surface.Rough_h(i,j);

                    profile.AGL=input.outputWindHeight + input.surface.Rough_h(i,j);			//this is height above THE GROUND!! (not "z=0" for the log profile)

                    profile.inputWindSpeed = U.vectorData_x(i, j, k);
                    uu = profile.getWindSpeed();

                    profile.inputWindSpeed = U.vectorData_y(i, j, k);
                    vv = profile.getWindSpeed();

                    profile.inputWindSpeed = U.vectorData_z(i, j, k);
                    ww = profile.getWindSpeed();

                    VelocityGrid(i,j)=std::pow((uu*uu+vv*vv),0.5);       //calculate velocity magnitude (in x,y plane; I decided to NOT include z here so the wind is the horizontal wind)

                }else{  //else use linear interpolation
                    slopeu=(U.vectorData_x(i, j, k)-U.vectorData_x(i, j, k-1))/(h2-h1);
                    slopev=(U.vectorData_y(i, j, k)-U.vectorData_y(i, j, k-1))/(h2-h1);
                    slopew=(U.vectorData_z(i, j, k)-U.vectorData_z(i, j, k-1))/(h2-h1);
                    uu=slopeu*(input.outputWindHeight + input.surface.Rough_h(i,j))+
                        U.vectorData_x(i, j, k-1)-slopeu*h1;
                    vv=slopev*(input.outputWindHeight + input.surface.Rough_h(i,j))+
                        U.vectorData_y(i, j, k-1)-slopev*h1;
                    ww=slopew*(input.outputWindHeight + input.surface.Rough_h(i,j))+
                        U.vectorData_z(i, j, k-1)-slopew*h1;
                    VelocityGrid(i,j)=std::pow((uu*uu+vv*vv),0.5);       //calculate velocity magnitude (in x,y plane; I decided to NOT include z here so the wind is the horizontal wind)
                }

                if (uu==0.0 && vv==0.0)
                    intermedval=0.0;
                else
                    intermedval=-atan2(uu, -vv);
                if(intermedval<0)
                    intermedval+=2.0*pi;
                AngleGrid(i,j)=(180.0/pi*intermedval);

            }
        }
    }	//end parallel region
}
/**Writes an output file comparing the current simulation to an input file containing points of measured wind speeds and directions.
 *
 */
void ninja::write_compare_output()
{
    FILE *points;
    FILE *output;

    points = fopen("points.txt", "r");
    output = fopen("output.txt", "w");

    double x,y,meas_vel,meas_dir;

    fprintf(output,"x-value\ty-value\tmeasured_vel\tmeasured_dir\tsim_vel\tsim_dir\n");

    while((fscanf(points, "%lf %lf %lf %lf", &x, &y, &meas_vel, &meas_dir)) != EOF)
        fprintf(output,"%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",x,y,meas_vel,meas_dir,VelocityGrid.interpolateGrid(x,y,AsciiGrid<double>::order1),AngleGrid.interpolateGrid(x,y,AsciiGrid<double>::order1));

    fclose(points);
    fclose(output);
}

/**Writes a projection file.
 *
 * @param inPrjString Incoming projection string to be written.
 * @param outFileName Name of output file.
 * @return True if completes correctly.
 */
bool ninja::writePrjFile(std::string inPrjString, std::string outFileName)
{
    std::ofstream outputFile(outFileName.c_str(), fstream::trunc);
    outputFile << inPrjString.c_str();

    return true;
}

/**Checks for a "null" run, which means that all initial velocities are zero.
 * When this occurs, the solver is skipped and output files are written with zero for values.
 * @return True if this is a null run, else false.
 */
bool ninja::checkForNullRun()
{
	int i;
	isNullRun = true;
	for(i=0;i<mesh.NUMNP;i++)
	{
        if(U0.vectorData_x(i) != 0.0)
			isNullRun = false;
	}
	for(i=0;i<mesh.NUMNP;i++)
	{
        if(U0.vectorData_y(i) != 0.0)
			isNullRun = false;
	}
	for(i=0;i<mesh.NUMNP;i++)
	{
        if(U0.vectorData_z(i) != 0.0)
			isNullRun = false;
	}

	return isNullRun;
}

/**Prepares for writing output files.
 * Builds 2d surfaces and calls interp_uvw() to interpolate volume data to output surface at output height.
 *
 */
void ninja::prepareOutput()
{
    VelocityGrid.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0, input.dem.prjString);
	AngleGrid.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0, input.dem.prjString);
	
	if(!isNullRun)
		interp_uvw();
 
        if(input.initializationMethod == WindNinjaInputs::foamDomainAverageInitializationFlag){
            //Set cloud grid
            int longEdge = input.dem.get_nRows();
            if(input.dem.get_nRows() < input.dem.get_nCols())
                    longEdge = input.dem.get_nCols();
            double tempCloudCover;
            if(input.cloudCover < 0)
                tempCloudCover = 0.0;
            else
                tempCloudCover = input.cloudCover;
            CloudGrid.set_headerData(1, 1, input.dem.get_xllCorner(), 
                    input.dem.get_yllCorner(),
                    (longEdge * input.dem.cellSize), 
                    -9999.0, tempCloudCover, input.dem.prjString);
        }

	//Clip off bounding doughnut if desired
	VelocityGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
	AngleGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);

	//Clip cloud cover grid if it's a wxModel intitialization (since it's gridded)
	//	if not wxModel initialization, don't clip since it's just one cell anyway
	if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
	{
		CloudGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
	}
	//change windspeed units back to what is specified by speed units switch
	velocityUnits::fromBaseUnits(VelocityGrid, input.outputSpeedUnits);

	/*
	 * Interpolate u, v, w to specific locations if an input_points_file is provided
     */
#pragma omp critical
    {
        if(input.inputPointsFilename != "!set"){
            element elem(&mesh);
            double new_u, new_v, new_w;
            double lat, lon;
            std::string pointName;
            double u_coord, v_coord, w_coord, x, y, z, height_above_ground;
            double z_ground, z_temp;
            int elem_i, elem_j, elem_k, elem_num;
            windProfile profile;
            profile.profile_switch = windProfile::monin_obukov_similarity;

            std::vector<boost::local_time::local_date_time> times;
            std::string dt;

            if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){
                times = (init->getTimeList(input.ninjaTimeZone));
                dt =  boost::lexical_cast<std::string>(input.ninjaTime);
            }

            FILE *output;

            if(input.outputPointsFilename != "!set"){ //if output file name is specified
                if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){// if it's a wx model run
                    if(input.ninjaTime == times[0]){ //if it's the first time step write headers to new file
                        output = fopen(input.outputPointsFilename.c_str(), "w");
                        if(init->getForecastIdentifier()=="WRF-3D"){ //it's a 2D wx model run
                            fprintf(output, "ID,lat,lon,height,datetime,u,v,w,wx_u,wx_v,wx_w\n");
                        }
                        else{
                            fprintf(output, "ID,lat,lon,height,datetime,u,v,w,wx_u,wx_v\n");
                        }
                    }
                    else{ //if not the first time step, just append data
                        output = fopen(input.outputPointsFilename.c_str(), "a"); //append new time to end of file
                    }
                }
                else{ //if it's not a wx model run
                    output = fopen(input.outputPointsFilename.c_str(), "w");
                    fprintf(output, "ID,lat,lon,height,u,v,w\n");
                }
            }
            else{ //if out filename is not specified
                if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){ //if wx model run
                    if(input.ninjaTime == times[0]){ //if it's the first time step write headers to new file
                        output = fopen("output.txt", "w"); //append new time to end of file
                        if(init->getForecastIdentifier()=="WRF-3D"){ //it's a 3D wx model run
                            fprintf(output, "ID,lat,lon,height,datetime,u,v,w,wx_u,wx_v,wx_w\n");
                        }
                        else{//it's a 2D wx model run
                            fprintf(output, "ID,lat,lon,height,datetime,u,v,w,wx_u,wx_v\n");
                        }
                    }
                    else{ //if not the first time step, just append data
                        output = fopen(input.outputPointsFilename.c_str(), "a"); //append new time to end of file
                    }
                }
                else{ //if not a wx model run
                    output = fopen("output.txt", "w");
                    fprintf(output, "ID,lat,lon,height,u,v,w\n");
                }
            }

            for(unsigned int i = 0; i < input.lonList.size(); i++ ){

                /*
                 * Transform coords from lat/long to WN mesh space
                 */
                x = input.projXList[i];
                y = input.projYList[i];
                height_above_ground = input.heightList[i];
                lat = input.latList[i];
                lon = input.lonList[i];
                pointName = input.pointsNamesList[i];

                if(!input.dem.check_inBounds(x, y)){
                    throw std::runtime_error("Requested output point is located outside of the DEM extent.");
                }

                x -= input.dem.xllCorner; //adjust to wn coords
                y -= input.dem.yllCorner; //adjust to wn coords

                elem.get_uv(x, y, elem_i, elem_j, u_coord, v_coord);
                elem_num = mesh.get_elemNum(elem_i, elem_j, 0);
                elem.get_xyz(elem_num, u_coord, v_coord, -1, x, y, z); // get z at ground
                z_ground = z;
                z += height_above_ground;

                /*
                 * Perform interpolation on u, v, w
                 */
                elem.get_uvw(x, y, z, elem_i, elem_j, elem_k, u_coord, v_coord, w_coord); // find elem_k for this point
                if(elem_k == 0){//if in first layer, use log profile
                    //profile stuff is a little weird bc we are not at nodes
                    //profile is set based on southwest corner of current cell (elem_i, elem_j)
                    //could interpolate these too?
                    profile.ObukovLength = init->L(elem_i,elem_j);
                    profile.ABL_height = init->bl_height(elem_i,elem_j);
                    profile.Roughness = input.surface.Roughness(elem_i,elem_j);
                    profile.Rough_h = input.surface.Rough_h(elem_i,elem_j);
                    profile.Rough_d = input.surface.Rough_d(elem_i,elem_j);
                    profile.AGL = height_above_ground;  // height above the ground

                    elem.get_xyz(elem_num, u_coord, v_coord, 1, x, y, z_temp); // get z at first layer

                    profile.inputWindHeight = z_temp - z_ground - input.surface.Rough_h(elem_i, elem_j); // height above vegetation
                    profile.inputWindSpeed = U.vectorData_x.interpolate(x, y, z_temp);

                    new_u = profile.getWindSpeed();
                    profile.inputWindSpeed = U.vectorData_y.interpolate(x, y, z_temp);
                    new_v = profile.getWindSpeed();
                    profile.inputWindSpeed = U.vectorData_z.interpolate(x, y, z_temp);
                    new_w = profile.getWindSpeed();
                }
                else{//else use linear interpolation
                    new_u = U.vectorData_x.interpolate(x,y,z);
                    new_v = U.vectorData_y.interpolate(x,y,z);
                    new_w = U.vectorData_z.interpolate(x,y,z);
                }

                if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){ //if wx model run

                    if(init->getForecastIdentifier() == "WRF-3D"){
                        fprintf(output,"%s,%lf,%lf,%lf,%s,%lf,%lf,%lf,%lf,%lf,%lf\n", pointName.c_str(), lat, lon, height_above_ground, dt.c_str(),
                                new_u, new_v, new_w, init->u_wxList[i], init->v_wxList[i], init->w_wxList[i]);
                    }
                    else{
                    fprintf(output,"%s,%lf,%lf,%lf,%s,%lf,%lf,%lf,%lf,%lf\n", pointName.c_str(), lat, lon, height_above_ground, dt.c_str(),
                            new_u, new_v, new_w, init->u10List[i], init->v10List[i]);
                    }
                }
                else{ //if not a wx model run
                   fprintf(output,"%s,%lf,%lf,%lf,%lf,%lf,%lf\n", pointName.c_str(), lat, lon, height_above_ground, new_u, new_v, new_w);
                }
            }
            fclose(output);

        }
    }
}

/**Compares the current simulated wind field to the measured wind at points.
 * This function is used during the outer ninja iterations to determine if the solved wind field matches the input wx stations.
 * @param iter Interation number
 * @return Returns true if the wind field is within matchTol of the wx stations.  If not, it adjusts stationsScratch for another outer loop simulation.
 */
bool ninja::matched(int iter)
{
	if(input.matchWxStations == true)
	{

		element elem(&mesh);
		double x, y, z;
		double u_loc, v_loc, w_loc;
        double try_output_u, try_output_v, try_output_w;
		int cell_i, cell_j, cell_k;
		double spd, dir;
		double true_u, true_v, true_w;
        double try_input_u, try_input_v, try_input_w;
        double old_input_u, old_input_v, old_input_w;
        double old_output_u, old_output_v, old_output_w;
		double new_input_u, new_input_v, new_input_w;
		double maxCurrentOuterDiff = -1.0;
		double percent_complete, time_percent_complete;
		double tempCompleteIn, tempCompleteOut;
		bool ret = true;
        bool u_keep_old, v_keep_old, w_keep_old;
        double delta = 0.1;


		input.Com->ninjaCom(ninjaComClass::ninjaNone, "Stations matching check:");
		//input.Com->ninjaCom(ninjaComClass::ninjaNone, "Station #\tmeas_u\tcomp_u\tmeas_v\tcomp_v\tmeas_w\tcomp_w");

		for(unsigned int i=0; i<input.stations.size(); i++)
		{
			//Get (x,y,z) value of station
			x = input.stations[i].get_xord();
			y = input.stations[i].get_yord();
            //Check if station is in mesh, if not, can't do matching so skip
            if(!mesh.inMeshXY(x, y))
                continue;
            z = input.stations[i].get_height() + input.surface.Rough_h.interpolateGridLocalCoordinates(x, y, AsciiGrid<double>::order1) + input.dem.interpolateGridLocalCoordinates(x, y, AsciiGrid<double>::order1);

			//Get cell number and "parent cell" coordinates of station location
			elem.get_uvw(x, y, z, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);

            //Get velocity at the station location
            try_output_u = U.vectorData_x.interpolate(elem, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);
            try_output_v = U.vectorData_y.interpolate(elem, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);
            try_output_w = U.vectorData_z.interpolate(elem, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);

			//Convert true station values to u, v for comparison below
            wind_sd_to_uv(input.stations[i].get_speed(), input.stations[i].get_direction(), &true_u, &true_v);
			true_w = input.stations[i].get_w_speed();

			//Check if we're within the tolerance
            if( abs(true_u-try_output_u) > matchTol)
			{
				ret = false;
			}
            if( abs(true_v-try_output_v) > matchTol)
			{
				ret = false;
			}
			//At this point, we don't match vertical velocity...
            //if( abs(input.stations[i].get_w_speed()-try_output_w) > matchTol)
			//{
			//	ret = false;
			//}

			//Store starting difference
			if(iter == 1)
			{
                if( abs(true_u-try_output_u) > maxStartingOuterDiff)
                    maxStartingOuterDiff = abs(true_u-try_output_u);
                if( abs(true_v-try_output_v) > maxStartingOuterDiff)
                    maxStartingOuterDiff = abs(true_v-try_output_v);
			    maxCurrentOuterDiff = maxStartingOuterDiff;
			}else{
                if(abs(true_u-try_output_u) > maxCurrentOuterDiff)
                    maxCurrentOuterDiff = abs(true_u-try_output_u);
                if(abs(true_v-try_output_v) > maxCurrentOuterDiff)
                    maxCurrentOuterDiff = abs(true_v-try_output_v);
			}

            //index for storing data back in wxstation object
            int dataIndex=input.inputsRunNumber;

            wind_sd_to_uv(input.stationsScratch[i].get_speed(), input.stationsScratch[i].get_direction(), &try_input_u, &try_input_v);
            try_input_w = input.stationsScratch[i].get_w_speed();
            wind_sd_to_uv(input.stationsOldInput[i].get_speed(), input.stationsOldInput[i].get_direction(), &old_input_u, &old_input_v);
            old_input_w = input.stationsOldInput[i].get_w_speed();
            wind_sd_to_uv(input.stationsOldOutput[i].get_speed(), input.stationsOldOutput[i].get_direction(), &old_output_u, &old_output_v);
            old_output_w = input.stationsOldOutput[i].get_w_speed();

            input.Com->ninjaCom(ninjaComClass::ninjaNone, "%i\t%s\tU_diff = %lf\tV_diff = %lf\tW_diff = %lf", i, input.stations[i].get_stationName().c_str(), true_u - try_output_u, true_v - try_output_v, true_w - try_output_w);
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "try_input_u = %lf\tu_solve = %lf\tu_true = %lf", try_input_u, try_output_u, true_u);
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "try_input_v = %lf\tv_solve = %lf\tv_true = %lf", try_input_v, try_output_v, true_v);
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "try_input_w = %lf\tw_solve = %lf\tw_true = %lf", try_input_w, try_output_w, true_w);

			//Compute new values using formula (from Lopes (2003)):
			//
			//            x2 = x1 + outer_relax(yr - y1)
			//
			//        where
			//
			//            x2 = new input value at the present iteration
			//            x1 = input value at the previous iteration
			//            yr = reference value, i.e., reading at the wx station
			//            y1 = computed value at the wx station, in the previous iteration
            //			  outer_relax = relaxation value (Lopes found 0.8 to be good compromise between fast convergence and to prevent divergence)

			//u component
            //new_input_u = try_input_u + input.outer_relax*(true_u - try_output_u);
			//v component
            //new_input_v = try_input_v + input.outer_relax*(true_v - try_output_v);
			//w component
            //new_input_w = try_input_w + input.outer_relax*(true_w - try_output_w);

            //Compute new values using formula (use ideas from Walter Murray 2010, page 6, http://web.stanford.edu/class/cme304/docs/newton-type-methods.pdf):
            //Changed the old method above to include limiting of over shooting and making the solutions worse, in this case we just scrap that step and try a half step.
            //    this recurses (the half steps) until it's better than the last good step OR 3 half step recursions have been attempted.  In the latter case, we stop
            //    and just take the step, because there was a problem where we got into a never ending recursion problem.


            //Did our last correction attempt improve things for u?

            if(fabs(try_output_u-true_u) > fabs(old_output_u-true_u) && iter>1 && num_outer_iter_tries_u[i]<1)
            {
                //If worse, then scrap the last try and, only step halfway
                num_outer_iter_tries_u[i]++;
                new_input_u = old_input_u + input.outer_relax*(true_u - old_output_u)/((double)pow(2.0, num_outer_iter_tries_u[i]));
                u_keep_old = true;
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Last u step was worse, try half that step.");
            }else
            {
                num_outer_iter_tries_u[i] = 0;
                new_input_u = try_input_u + input.outer_relax*(true_u - try_output_u)/((double)pow(2.0, num_outer_iter_tries_u[i]));
                u_keep_old = false;
            }

            //Did our last correction attempt improve things for v?
            if(fabs(try_output_v-true_v) > fabs(old_output_v-true_v) && iter>1 && num_outer_iter_tries_v[i]<1)
            {
                //Then scrap the last try and, only step halfway
                num_outer_iter_tries_v[i]++;
                new_input_v = old_input_v + input.outer_relax*(true_v - old_output_v)/((double)pow(2.0, num_outer_iter_tries_v[i]));
                v_keep_old = true;
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Last v step was worse, try half that step.");
            }else
            {
                num_outer_iter_tries_v[i] = 0;
                new_input_v = try_input_v + input.outer_relax*(true_v - try_output_v)/((double)pow(2.0, num_outer_iter_tries_v[i]));
                v_keep_old = false;
            }

            //Did our last correction attempt improve things for w?
            if(fabs(try_output_w-true_w) > fabs(old_output_w-true_w) && iter>1 && num_outer_iter_tries_w[i]<1)
            {
                //Then scrap the last try and, only step halfway
                num_outer_iter_tries_w[i]++;
                new_input_w = old_input_w + input.outer_relax*(true_w - old_output_w)/((double)pow(2.0, num_outer_iter_tries_w[i]));
                w_keep_old = true;
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Last w step was worse, try half that step.");
            }else
            {
                num_outer_iter_tries_w[i] = 0;
                new_input_w = try_input_w + input.outer_relax*(true_w - try_output_w)/((double)pow(2.0, num_outer_iter_tries_w[i]));
                w_keep_old = false;
            }


            if(u_keep_old==true && v_keep_old==true)
            {
                wind_uv_to_sd(old_input_u, old_input_v, &spd, &dir);
                input.stationsOldInput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldInput[i].update_direction(dir,dataIndex);

                wind_uv_to_sd(old_output_u, old_output_v, &spd, &dir);
                input.stationsOldOutput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldOutput[i].update_direction(dir,dataIndex);

            }else if(u_keep_old==true && v_keep_old==false)
            {
                wind_uv_to_sd(old_input_u, try_input_v, &spd, &dir);
                input.stationsOldInput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldInput[i].update_direction(dir,dataIndex);

                wind_uv_to_sd(old_output_u, try_output_v, &spd, &dir);
                input.stationsOldOutput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldOutput[i].update_direction(dir,dataIndex);

            }else if(u_keep_old==false && v_keep_old==true)
            {
                wind_uv_to_sd(try_input_u, old_input_v, &spd, &dir);
                input.stationsOldInput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldInput[i].update_direction(dir,dataIndex);

                wind_uv_to_sd(try_output_u, old_output_v, &spd, &dir);
                input.stationsOldOutput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldOutput[i].update_direction(dir,dataIndex);

            }else
            {
                wind_uv_to_sd(try_input_u, try_input_v, &spd, &dir);
                input.stationsOldInput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldInput[i].update_direction(dir,dataIndex);

                wind_uv_to_sd(try_output_u, try_output_v, &spd, &dir);
                input.stationsOldOutput[i].update_speed(spd, velocityUnits::metersPerSecond,dataIndex);
                input.stationsOldOutput[i].update_direction(dir,dataIndex);
            }


            //input.stationsScratch[i].set_w_speed(new_input_w, velocityUnits::metersPerSecond);

            //u component
            //new_input_u = try_input_u + input.outer_relax*(true_u - try_output_u);
            //v component
            //new_input_v = try_input_v + input.outer_relax*(true_v - try_output_v);
            //w component
            //new_input_w = try_input_w + input.outer_relax*(true_w - try_output_w);

			//Set stationsScratch to new velocities and direction that are closer (hopefully!)
			wind_uv_to_sd(new_input_u, new_input_v, &spd, &dir);
            input.stationsScratch[i].update_speed(spd,velocityUnits::metersPerSecond,dataIndex);
            input.stationsScratch[i].update_direction(dir,dataIndex);

//			//input.stationsScratch[i].set_w_speed(new_input_w, velocityUnits::metersPerSecond);
        }

		//compute percent complete
		percent_complete=100.0-100.0*((maxCurrentOuterDiff-matchTol)/(maxStartingOuterDiff-matchTol));
		//if(residual_percent_complete<residual_percent_complete_old)
		//    residual_percent_complete=residual_percent_complete_old;
		if(percent_complete<0.0)
		    percent_complete=0.0;
		else if(percent_complete>100.0)
		    percent_complete=100.0;

		//compute percent complete (map so convergence is more linear on progress bar)
		tempCompleteIn = 1 - percent_complete/100.0;
		tempCompleteOut = (std::pow(2.0, 0.46371))*std::pow((tempCompleteIn/(1+std::pow(tempCompleteIn, 2.0))),0.46371);
		time_percent_complete = -100.0*(tempCompleteOut-1.0);

		if(time_percent_complete >= 100.0)
		    time_percent_complete = 100.0;
		else if(time_percent_complete >= 99.0 )
		    time_percent_complete = 99.0;
        input.Com->ninjaCom(ninjaComClass::ninjaOuterIterProgress, "%d",(int) (time_percent_complete+0.5));
//        input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete)); //STATION FETCH COMM


		return ret;
	}
	return true;
}

/**
 * Computes friction velocity.
 */
#ifdef FRICTION_VELOCITY
void ninja::computeFrictionVelocity()
{
    FrictionVelocity fv;

    fv.ComputeVertexNormals(mesh, input);

    UstarGrid.set_headerData(input.dem.get_nCols(),
                            input.dem.get_nRows(),
                            input.dem.get_xllCorner(),
                            input.dem.get_yllCorner(),
                            input.dem.get_cellSize(),
                            -9999.0,
                            -9999.0,
                            input.dem.prjString);
    UstarGrid = 0;

    //fv.ComputeUstar(input, UstarGrid, u, v, w, mesh, "shearStress"); // options are "logProfile" and "shearStress"
    fv.ComputeUstar(input, UstarGrid, u, v, w, mesh, input.frictionVelocityCalculationMethod);
    //UstarGrid.write_Grid("ustar", 2);

}
#endif

/**
 * Computes dust emissions.
 */
#ifdef EMISSIONS
void ninja::computeDustEmissions()
{
    CPLDebug("DUST", "dust flag = %i\n", input.dustFlag);

    Dust dust;

    DustGrid.set_headerData(input.dem.get_nCols(),
                            input.dem.get_nRows(),
                            input.dem.get_xllCorner(),
                            input.dem.get_yllCorner(),
                            input.dem.get_cellSize(),
                            -9999.0,
                            -9999.0,
                            input.dem.prjString);


    dust.MakeGrid(input, DustGrid);    //burn the fire perimeter .shp into a raster

    dust.ComputePM10(UstarGrid, DustGrid);

    //DustGrid.ascii2png("dust_out_shear.png", "pm10", "ug/m3", "legend", false);
}
#endif //EMISISONS

/**Writes output files.
 * Writes VTK, FARSITE ASCII Raster, text comparison, shape, and kmz output files.
 */

void ninja::writeOutputFiles()
{
    set_outputFilenames(mesh.meshResolution, mesh.meshResolutionUnits);

    //Write volume data to VTK format (always in m/s?)
    if(input.volVTKOutFlag)
    {
        try{
            volVTK VTK(U, mesh.XORD, mesh.YORD, mesh.ZORD, 
            input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, input.volVTKFile);
        }catch (exception& e)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning,
                    "Exception caught during volume VTK file writing: %s", e.what());
        }catch (...)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning,
                    "Exception caught during volume VTK file writing: Cannot determine exception type.");
        }
    }

    U.deallocate();

#pragma omp parallel sections
    {
    //write FARSITE files
#pragma omp section
    {
        try{
            if(input.asciiOutFlag==true)
            {
                AsciiGrid<double> *velTempGrid, *angTempGrid;
                velTempGrid=NULL;
                angTempGrid=NULL;

                angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.angResolution,
                            AsciiGrid<double>::order0));
                velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.velResolution, 
                            AsciiGrid<double>::order0));

                AsciiGrid<double> tempCloud(CloudGrid);
                tempCloud *= 100.0;  //Change to percent, which is what FARSITE needs

                //ensure grids cover original DEM extents for FARSITE
                tempCloud.BufferGridInPlace();
                angTempGrid->BufferGridInPlace();
                velTempGrid->BufferGridInPlace();

                tempCloud.write_Grid(input.cldFile.c_str(), 1);
                angTempGrid->write_Grid(input.angFile.c_str(), 0);
                velTempGrid->write_Grid(input.velFile.c_str(), 2);

#ifdef FRICTION_VELOCITY
                if(input.frictionVelocityFlag == 1){
                    AsciiGrid<double> *ustarTempGrid;
                    ustarTempGrid=NULL;

                    ustarTempGrid = new AsciiGrid<double> (UstarGrid.resample_Grid(input.velResolution,
                                AsciiGrid<double>::order0));

                    ustarTempGrid->write_Grid(input.ustarFile.c_str(), 2);

                    if(ustarTempGrid)
                    {
                        delete ustarTempGrid;
                        ustarTempGrid=NULL;
                    }
                }
#endif

#ifdef EMISSIONS
                if(input.dustFlag == 1){
                    AsciiGrid<double> *dustTempGrid;
                    dustTempGrid=NULL;

                    dustTempGrid = new AsciiGrid<double> (DustGrid.resample_Grid(input.velResolution,
                                AsciiGrid<double>::order0));

                    dustTempGrid->write_Grid(input.dustFile.c_str(), 2);

                    if(dustTempGrid)
                    {
                        delete dustTempGrid;
                        dustTempGrid=NULL;
                    }
                }
#endif
                if(angTempGrid)
                {
                    delete angTempGrid;
                    angTempGrid=NULL;
                }
                if(velTempGrid)
                {
                    delete velTempGrid;
                    velTempGrid=NULL;
                }

                //Write .atm file for this run.  Only has one time value in file.
                if(input.writeAtmFile)
                {
                    farsiteAtm atmosphere;
                    atmosphere.push(input.ninjaTime, input.velFile, input.angFile, input.cldFile);
                    atmosphere.writeAtmFile(input.atmFile, input.outputSpeedUnits, input.outputWindHeight);
                }
            }
        }catch (exception& e)
    {
        input.Com->ninjaCom(ninjaComClass::ninjaWarning,
                "Exception caught during ascii file writing: %s", e.what());
    }catch (...)
    {
        input.Com->ninjaCom(ninjaComClass::ninjaWarning, 
                "Exception caught during ascii file writing: Cannot determine exception type.");
    }
    }//end omp section

    //write text file comparing measured to simulated winds (measured read from file, 
    //filename, etc. hard-coded in function)
#pragma omp section
    {
    try{
    if(input.txtOutFlag==true)
    write_compare_output();
    }catch (exception& e)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during text file writing: %s", e.what());
    }catch (...)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during text file writing: Cannot determine exception type.");
    }
    }//end omp section

    //write shape files
#pragma omp section
    {
    try{
    if(input.shpOutFlag==true)
    {
    AsciiGrid<double> *velTempGrid, *angTempGrid, *ustarTempGrid;
    velTempGrid=NULL;
    angTempGrid=NULL;
    ustarTempGrid=NULL;

    ShapeVector ninjaShapeFiles;

    angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.shpResolution, AsciiGrid<double>::order0));
    velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.shpResolution, AsciiGrid<double>::order0));

    ninjaShapeFiles.setDirGrid(*angTempGrid);
    ninjaShapeFiles.setSpeedGrid(*velTempGrid);
    ninjaShapeFiles.setDataBaseName(input.dbfFile);
    ninjaShapeFiles.setShapeFileName(input.shpFile);
    ninjaShapeFiles.makeShapeFiles();

    if(angTempGrid)
    {
    delete angTempGrid;
    angTempGrid=NULL;
    }
    if(velTempGrid)
    {
    delete velTempGrid;
    velTempGrid=NULL;
    }
    }
    }catch (exception& e)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during shape file writing: %s", e.what());
    }catch (...)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during shape file writing: Cannot determine exception type.");
    }
    }//end omp section


    //write kmz file
#pragma omp section
    {
    try{
    if(input.googOutFlag==true)

    {
    AsciiGrid<double> *velTempGrid, *angTempGrid;
    velTempGrid=NULL;
    angTempGrid=NULL;

    KmlVector ninjaKmlFiles;

    angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.kmzResolution, AsciiGrid<double>::order0));
    velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.kmzResolution, AsciiGrid<double>::order0));

#ifdef FRICTION_VELOCITY
    if(input.frictionVelocityFlag == 1){
    AsciiGrid<double> *ustarTempGrid;

    ustarTempGrid=NULL;

    ustarTempGrid = new AsciiGrid<double> (UstarGrid.resample_Grid(input.kmzResolution, AsciiGrid<double>::order0));

    ninjaKmlFiles.setUstarFlag(input.frictionVelocityFlag);
    ninjaKmlFiles.setUstarGrid(*ustarTempGrid);

    if(ustarTempGrid)
    {
    delete ustarTempGrid;
    ustarTempGrid=NULL;
    }
    }
#endif //FRICTION_VELOCITY

#ifdef EMISSIONS
    if(input.dustFlag == 1){
    AsciiGrid<double> *dustTempGrid;

    dustTempGrid=NULL;

    dustTempGrid = new AsciiGrid<double> (DustGrid.resample_Grid(input.kmzResolution, AsciiGrid<double>::order0));

    ninjaKmlFiles.setDustFlag(input.dustFlag);
    ninjaKmlFiles.setDustGrid(*dustTempGrid);

    if(dustTempGrid)
    {
    delete dustTempGrid;
    dustTempGrid=NULL;
    }
    }
#endif //EMISSIONS

    ninjaKmlFiles.setKmlFile(input.kmlFile);
    ninjaKmlFiles.setKmzFile(input.kmzFile);
    ninjaKmlFiles.setDemFile(input.dem.fileName);


    ninjaKmlFiles.setLegendFile(input.legFile);
    ninjaKmlFiles.setDateTimeLegendFile(input.dateTimeLegFile, input.ninjaTime);
    ninjaKmlFiles.setSpeedGrid(*velTempGrid, input.outputSpeedUnits);
    ninjaKmlFiles.setDirGrid(*angTempGrid);

    ninjaKmlFiles.setLineWidth(input.googLineWidth);
    ninjaKmlFiles.setTime(input.ninjaTime);
    if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
    {
    std::vector<boost::local_time::local_date_time> times(init->getTimeList(input.ninjaTimeZone));
    ninjaKmlFiles.setWxModel(init->getForecastIdentifier(), times[0]);
    }
    if(ninjaKmlFiles.writeKml(input.googSpeedScaling,input.googColor,input.googVectorScale))
    {
    if(ninjaKmlFiles.makeKmz())
    ninjaKmlFiles.removeKmlFile();
    }
    if(angTempGrid)
    {
    delete angTempGrid;
    angTempGrid=NULL;
    }
    if(velTempGrid)
    {
    delete velTempGrid;
    velTempGrid=NULL;
    }
    }
    }catch (exception& e)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during Google Earth file writing: %s", e.what());
    }catch (...)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during Google Earth file writing: Cannot determine exception type.");
    }
    }//end omp section

#pragma omp section
    {
    try{
    if(input.pdfOutFlag==true)
    {
    AsciiGrid<double> *velTempGrid, *angTempGrid;
    velTempGrid=NULL;
    angTempGrid=NULL;
    OutputWriter output;

    angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.pdfResolution, AsciiGrid<double>::order0));
    velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.pdfResolution, AsciiGrid<double>::order0));

    output.setDirGrid(*angTempGrid);
    output.setSpeedGrid(*velTempGrid, input.outputSpeedUnits);
    output.setDEMfile(input.pdfDEMFileName);
    output.setLineWidth(input.pdfLineWidth);
    output.setDPI(input.pdfDPI);
    output.setSize(input.pdfWidth, input.pdfHeight);
    output.write(input.pdfFile, "PDF");



    if(angTempGrid)
    {
    delete angTempGrid;
    angTempGrid=NULL;
    }
    if(velTempGrid)
    {
    delete velTempGrid;
    velTempGrid=NULL;
    }
    }
    }catch (exception& e)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during pdf file writing: %s", e.what());
    }catch (...)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during pdf file writing: Cannot determine exception type.");
    }
    } //end omp section

#pragma omp section
    {
#ifdef EMISSIONS
    try{
    if(input.geotiffOutFlag==true)
    {
    OutputWriter output;

    output.setNinjaTime( boost::lexical_cast<std::string>(input.ninjaTime) );
    output.setRunNumber(input.inputsRunNumber);
    output.setMaxRunNumber(input.armySize-1);

    output.setDirGrid(AngleGrid);
    output.setSpeedGrid(VelocityGrid, input.outputSpeedUnits);

    output.setMemDs(input.hSpdMemDs, input.hDirMemDs, input.hDustMemDs);// set the in-memory datasets


#ifdef EMISSIONS
    if(input.dustFlag == 1){
    output.setDustGrid(DustGrid);
    }
#endif
    output.write(input.geotiffOutFilename, "GTiff");
    }
    }catch (exception& e)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during geotiff file writing: %s", e.what());
    }catch (...)
    {
    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during geotiff file writing: Cannot determine exception type.");
    }
#endif //EMISSIONS
    } //end omp section
    }	//end parallel sections region
}

/**Deletes allocated dynamic memory.
 *
 */
void ninja::deleteDynamicMemory()
{
	if(solar)
	{	delete solar;
		solar=NULL;
	}
	if(uDiurnal)
	{	delete uDiurnal;
		uDiurnal=NULL;
	}
	if(vDiurnal)
	{	delete vDiurnal;
		vDiurnal=NULL;
	}
	if(wDiurnal)
	{	delete wDiurnal;
		wDiurnal=NULL;
	}
	if(aspect)
	{	delete aspect;
		aspect=NULL;
	}
	if(slope)
	{	delete slope;
		slope=NULL;
	}
	if(shade)
	{	delete shade;
		shade=NULL;
	}
	if(height)
	{	delete height;
		height=NULL;
	}

	U0.deallocate();
}

/**Checks the cancel flag for cancelling simulation.
 * This allows the user to cancel a simulation where the user can interact during the simulation, such as a GUI run simulation.
 */
void ninja::checkCancel()
{
	if(cancel)
	{
		throw cancelledByUser();
	}
}

void ninja::set_inputPointsFilename(std::string filename)
{
    input.inputPointsFilename = filename;

    if(!CPLCheckForFile((char*)input.inputPointsFilename.c_str(), NULL)){
        throw std::runtime_error(std::string("The file ") +
                input.inputPointsFilename + " does not exist or may be in use by another program.");
    }

    FILE *points;
    points = fopen( input.inputPointsFilename.c_str(), "r" );

    char datum [6];
    fscanf(points, "%5s\n.", datum);

    if(!EQUAL(datum, "WGS84") && !EQUAL(datum, "NAD83") && !EQUAL(datum, "NAD27")){
        throw std::runtime_error(std::string("The datum must be specified as either WGS84, NAD83, or NAD27 in ninja::set_inputPointsFilename")
                                        + input.inputPointsFilename + ".");
    }

    GDALDataset *poDS;
    poDS = (GDALDataset*)GDALOpen(input.dem.fileName.c_str(), GA_ReadOnly);

    double projX, projY;
    char buffer[1024];
    char **papszTokens;
    int i,j,posi;
    int nTokens = 0;

    while((fgets(buffer, 1024-1, points)) != NULL){   //read a line
        //If the line is just white space, keep going
        posi = 0;
        while(buffer[posi] != '\0' && isspace(buffer[posi]))
            posi++;
        if(posi == strlen(buffer))
            continue;

        //Strip out strings seperated by commas
        papszTokens = CSLTokenizeString2(buffer, ",", CSLT_ALLOWEMPTYTOKENS | CSLT_STRIPLEADSPACES | CSLT_STRIPENDSPACES | CSLT_HONOURSTRINGS);
        nTokens = CSLCount(papszTokens);
        if(papszTokens == NULL || nTokens<4)  //Check that there are 4 columns
        {
            throw std::runtime_error(std::string("There was a problem reading the file ")
                                         + input.inputPointsFilename + ".  It appears that there are less than 4 columns.");
        }

        for( i = 0; i<nTokens; i++ )  //loop over columns
        {
            if(i==0)
                input.pointsNamesList.push_back(papszTokens[i]);
            else if(i==1)
                input.latList.push_back(atof(papszTokens[i]));
            else if(i==2)
                input.lonList.push_back(atof(papszTokens[i]));
            else if(i==3)
                input.heightList.push_back(atof(papszTokens[i]));
            else
                throw std::runtime_error(std::string("There was a problem reading the file ")
                                         + input.inputPointsFilename +
                                         ".  It appears that there are more than 4 columns.");
        }
        CSLDestroy( papszTokens );

        //Check that each name is unique
        for(i = 0; i < input.pointsNamesList.size(); i++)
        {
            for(j = i+1; j < input.pointsNamesList.size(); j++)
            {
                if( input.pointsNamesList[i] == input.pointsNamesList[j] )
                    throw std::runtime_error("There was a problem reading the file " +
                                             input.inputPointsFilename + ". The ID names "
                                             "should be unique, but your file has at least "
                                             "two with the name \"" + input.pointsNamesList[i]
                                             + "\".");
            }
        }

        projX = input.lonList[input.lonList.size()-1];
        projY = input.latList[input.latList.size()-1];

        //Transform from lat/lon to dem coords
        GDALPointFromLatLon( projX, projY, poDS, datum );

        input.projYList.push_back(projY);
        input.projXList.push_back(projX);

    }
    fclose(points);
    GDALClose((GDALDatasetH) poDS );
}

void ninja::set_outputPointsFilename(std::string filename)
{
    input.outputPointsFilename = filename;
}

#ifdef NINJA_SPEED_TESTING
void ninja::set_speedDampeningRatio(double r)
{
    input.speedDampeningRatio = r;
}
void ninja::set_downDragCoeff(double coeff)
{
    input.downDragCoeff = coeff;
}
void ninja::set_downEntrainmentCoeff(double coeff)
{
    input.downEntrainmentCoeff = coeff;
}
void ninja::set_upDragCoeff(double coeff)
{
    input.upDragCoeff = coeff;
}
void ninja::set_upEntrainmentCoeff(double coeff)
{
    input.upEntrainmentCoeff = coeff;
}
#endif

#ifdef FRICTION_VELOCITY
void ninja::set_frictionVelocityFlag(bool flag)
{
    input.frictionVelocityFlag = flag;
}
void ninja::set_frictionVelocityCalculationMethod(std::string calcMethod)
{
    input.frictionVelocityCalculationMethod = calcMethod;
}
#endif //FRICTION_VELOCITY

#ifdef EMISSIONS
void ninja::set_dustFlag(bool flag)
{
    input.dustFlag = flag;
}
void ninja::set_dustFilename(std::string filename)
{
    if( input.dustFlag==true )
    {
        input.dustFilename = filename;

        if(!CPLCheckForFile((char*)input.dustFilename.c_str(), NULL))
            throw std::runtime_error(std::string("The file ") +
                    input.dustFilename + " does not exist or may be in use by another program.");
    }
}
void ninja::set_dustFileOut(std::string filename)
{
    if( input.dustFlag==true )
    {
        input.dustFileOut = filename;
    }
}
void ninja::set_geotiffOutFlag(bool flag)
{
    input.geotiffOutFlag = flag;
}
void ninja::set_geotiffOutFilename(std::string filename)
{
    if( input.geotiffOutFlag==true )
    {
        input.geotiffOutFilename = filename; //if file exisits, bands are appended
    }
}
#endif //EMISSIONS

void ninja::set_DEM(std::string dem_file_name)
{
    //VSIStatBufL* buff;
    if(!CPLCheckForFile((char*)dem_file_name.c_str(), NULL))
        throw std::runtime_error(std::string("The file ") +
                dem_file_name + " does not exist or may be in use by another program.");
//	dem.read_elevation(dem_file_name, units);
//	input.surface.Roughness.set_headerData(dem);
//	input.surface.Rough_h.set_headerData(input.dem);
//	input.surface.Rough_d.set_headerData(input.dem);
//	input.surface.Albedo.set_headerData(input.dem);
//	input.surface.Bowen.set_headerData(input.dem);
//	input.surface.Cg.set_headerData(input.dem);
//	input.surface.Anthropogenic.set_headerData(input.dem);
//
//	input.surface.RoughnessUnits = lengthUnits::meters;
//	input.surface.Rough_hUnits = lengthUnits::meters;
//	input.surface.Rough_dUnits = lengthUnits::meters;
    input.dem.fileName = dem_file_name;
}

int ninja::get_inputsRunNumber() const
{
    return input.inputsRunNumber;
}

ninjaComClass::eNinjaCom ninja::get_inputsComType() const
{
    return input.inputsComType;
}

char * ninja::get_lastComString() const
{
    return (char*)input.lastComString;
}

FILE * ninja::get_ComLogFp() const
{
    return input.Com->fpLog;
}
ninjaComClass * ninja::get_Com() const
{
    return input.Com;
}

#ifdef NINJA_GUI
int ninja::get_ComNumRuns() const
{
    return input.Com->nRuns;
}

void ninja::set_ComNumRuns( int nRuns )
{
    input.Com->nRuns = nRuns;
}

#endif //NINJA_GUI

double ninja::get_progressWeight()
{
    return input.Com->progressWeight;
}

void ninja::set_progressWeight(double progressWeight)
{
    CPLDebug("NINJA","ADJUSTING PROGRESS BAR WT TO: %f",progressWeight);
    input.Com->progressWeight = progressWeight;
}

void ninja::set_initializationMethod( WindNinjaInputs::eInitializationMethod method,
                                      bool matchPoints )
{
    input.initializationMethod = method;
    input.matchWxStations = matchPoints;
}

WindNinjaInputs::eInitializationMethod ninja::get_initializationMethod()
{
    return input.initializationMethod;
}

void ninja::set_uniVegetation( WindNinjaInputs::eVegetation vegetation_ )
{
    if( (vegetation_ != WindNinjaInputs::grass) &&
        (vegetation_ != WindNinjaInputs::brush) &&
        (vegetation_ != WindNinjaInputs::trees) )
        throw std::logic_error("Problem with vegetation type in ninja::set_uniVegetation().");

    input.vegetation = vegetation_;
}

void ninja::set_uniVegetation()
{
    //vegetation = vegetation_;
    if( input.vegetation == WindNinjaInputs::grass )
    {
        set_uniRoughness(0.01, lengthUnits::meters);
        set_uniRoughH(0.0, lengthUnits::meters);
        set_uniRoughD(0.0, lengthUnits::meters);
        set_uniAlbedo(0.25);
        set_uniBowen(1.0);
        set_uniCg(0.15);
        set_uniAnthropogenic(0.0);
    }
    else if( input.vegetation == WindNinjaInputs::brush )
    {
        set_uniRoughness(0.43, lengthUnits::meters);
        set_uniRoughH(2.3, lengthUnits::meters);
        set_uniRoughD(1.8, lengthUnits::meters);
        set_uniAlbedo(0.25);
        set_uniBowen(1.0);
        set_uniCg(0.15);
        set_uniAnthropogenic(0.0);
    }
    else if( input.vegetation == WindNinjaInputs::trees )
    {
        set_uniRoughness(1.0, lengthUnits::meters);
        set_uniRoughH(15.4, lengthUnits::meters);
        set_uniRoughD(12.0, lengthUnits::meters);
        set_uniAlbedo(0.1);
        set_uniBowen(1.0);
        set_uniCg(0.15);
        set_uniAnthropogenic(0.0);
    }
    else
    {
        throw std::logic_error("Problem with vegetation type in ninja::set_uniVegetation().");
    }
}

WindNinjaInputs::eVegetation ninja::get_eVegetationType(std::string veg)
{
    if(veg == "grass")
        return WindNinjaInputs::grass;
    else if(veg == "brush")
        return WindNinjaInputs::brush;
    else if(veg == "trees")
        return WindNinjaInputs::trees;
    else{
        throw std::logic_error("Problem with vegetation string in ninja::get_vegetation().");
    }
}
void ninja::setArmySize(int n)
{
    input.armySize = n;
}

void ninja::set_stabilityFlag(bool flag)
{
    input.stabilityFlag = flag;
}

void ninja::set_alphaStability(double stability_)
{
    input.alphaStability = stability_;
    if(input.alphaStability < 0 || input.alphaStability > 5)
    {
        throw std::logic_error("Problem with stability in ninja::set_alphaStability().");
    }
}

#ifdef NINJAFOAM
void ninja::set_NumberOfIterations(int nIterations)
{
    input.nIterations = nIterations;
}
void ninja::set_MeshCount(int meshCount)
{
    input.meshCount = meshCount;
}

void ninja::set_MeshCount(WindNinjaInputs::eNinjafoamMeshChoice meshChoice)
{
    //if these change, update values in GUI for horizontal resolution estimation
    if(meshChoice == WindNinjaInputs::coarse){
        input.meshCount = 25000;
    }
    else if(meshChoice == WindNinjaInputs::medium){
        input.meshCount = 50000;
    }
    else if(meshChoice == WindNinjaInputs::fine){
        input.meshCount = 100000;
    }
    else{
        throw std::range_error("The mesh resolution choice has been set improperly.");
    }
}

WindNinjaInputs::eNinjafoamMeshChoice ninja::get_eNinjafoamMeshChoice(std::string meshChoice)
{
    if(meshChoice == "coarse"){
        return WindNinjaInputs::coarse;
    }
    else if(meshChoice == "medium"){
        return WindNinjaInputs::medium;
    }
    else if(meshChoice == "fine"){
        return WindNinjaInputs::fine;
    }
    else{
        throw std::logic_error("Problem with mesh type string in ninja::get_eMeshType().");
    }
}

void ninja::set_ExistingCaseDirectory(std::string directory)
{
    input.existingCaseDirectory = directory;
}

void ninja::set_foamVelocityGrid(AsciiGrid<double> velocityGrid)
{
    input.foamVelocityGrid = velocityGrid;
}

void ninja::set_foamAngleGrid(AsciiGrid<double> angleGrid)
{
    input.foamAngleGrid = angleGrid;
}
#endif

void ninja::set_speedFile(std::string speedFile, velocityUnits::eVelocityUnits units)
{
    input.speedInitGridFilename = speedFile;
    if(!CPLCheckForFile((char*)speedFile.c_str(), NULL))
        throw std::runtime_error(std::string("The file ") +
                speedFile + " does not exist or may be in use by another program.");

    input.inputSpeedUnits = units; //units set here, conversion to base units in griddedInitialization
}

void ninja::set_dirFile(std::string dirFile)
{
    input.dirInitGridFilename = dirFile;
    if(!CPLCheckForFile((char*)dirFile.c_str(), NULL))
        throw std::runtime_error(std::string("The file ") +
                dirFile + " does not exist or may be in use by another program.");
   
}

void ninja::computeSurfPropForCell
    ( long i, long j, double canopyHeight, lengthUnits::eLengthUnits canopyHeightUnits,
      double canopyCover, coverUnits::eCoverUnits canopyCoverUnits, int fuelModel,
      double fuelBedDepth, lengthUnits::eLengthUnits fuelBedDepthUnits )
{	// This function computes surface properties for WindNinja, based on information
    // available in a FARSITE .lcp file.
    // If any of the input parameters (canopyHeight, canopyCover, fuelModel, fuelBedDepth)
    // are not available (NO_DATA), enter a value less than 0.
    // If sufficient data is not passed to this function to determine the surface properties,
    // default values (rangeland) are used


    lengthUnits::toBaseUnits(canopyHeight, canopyHeightUnits);
    coverUnits::toBaseUnits(canopyCover, canopyCoverUnits);
    lengthUnits::toBaseUnits(fuelBedDepth, fuelBedDepthUnits);

    // Go through logic of determining surface properties, depending on what data is available at this cell

    if (canopyCover >= 0.05 && canopyHeight > 0)	// if enough cover use the canopy hgt
    {
        input.surface.Rough_h.set_cellValue(i, j, canopyHeight);
        input.surface.Rough_d.set_cellValue(i, j, canopyHeight*0.63);
        input.surface.Roughness.set_cellValue(i, j, canopyHeight*0.13);
        input.surface.Albedo.set_cellValue(i, j, 0.1);	//assuming forest land cover for heat transfer parameters
        input.surface.Bowen.set_cellValue(i, j, 1.0);
        input.surface.Cg.set_cellValue(i, j, 0.15);
        input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
    }else if(fuelModel==90 || fuelModel==91 || fuelModel==92 || fuelModel==93 || fuelModel==98)	// See if it's an unburnable Fuel Model
    {
        if(fuelModel == 90)			// Barren
        {
            input.surface.Rough_h.set_cellValue(i, j,  0.00230769);
            input.surface.Rough_d.set_cellValue(i, j, 0.00230769*0.63);
            input.surface.Roughness.set_cellValue(i, j, 0.00230769*0.13);
            input.surface.Albedo.set_cellValue(i, j, 0.3);
            input.surface.Bowen.set_cellValue(i, j, 1.0);
            input.surface.Cg.set_cellValue(i, j, 0.15);
            input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
        }else if(fuelModel == 91)	// Urban Roughness
        {
            input.surface.Rough_h.set_cellValue(i, j,  5.0);
            input.surface.Rough_d.set_cellValue(i, j, 5.0*0.63);
            input.surface.Roughness.set_cellValue(i, j, 5.0*0.13);
            input.surface.Albedo.set_cellValue(i, j, 0.18);
            input.surface.Bowen.set_cellValue(i, j, 1.5);
            input.surface.Cg.set_cellValue(i, j, 0.25);
            input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
        }else if(fuelModel == 92)	// Snow Ice
        {
            input.surface.Rough_h.set_cellValue(i, j,  0.00076923);
            input.surface.Rough_d.set_cellValue(i, j, 0.00076923*0.63);
            input.surface.Roughness.set_cellValue(i, j, 0.00076923*0.13);
            input.surface.Albedo.set_cellValue(i, j, 0.7);
            input.surface.Bowen.set_cellValue(i, j, 0.5);
            input.surface.Cg.set_cellValue(i, j, 0.15);
            input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
        }else if(fuelModel == 93)	// Agriculture
        {
            input.surface.Rough_h.set_cellValue(i, j,  1.0);
            input.surface.Rough_d.set_cellValue(i, j, 1.0*0.63);
            input.surface.Roughness.set_cellValue(i, j, 1.0*0.13);
            input.surface.Albedo.set_cellValue(i, j, 0.15);
            input.surface.Bowen.set_cellValue(i, j, 1.0);
            input.surface.Cg.set_cellValue(i, j, 0.15);
            input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
        }else if(fuelModel == 98)	// Water
        {
            input.surface.Rough_h.set_cellValue(i, j,  0.00153846);
            input.surface.Rough_d.set_cellValue(i, j, 0.00153846*0.63);
            input.surface.Roughness.set_cellValue(i, j, 0.00153846*0.13);
            input.surface.Albedo.set_cellValue(i, j, 0.1);
            input.surface.Bowen.set_cellValue(i, j, 0.0);
            input.surface.Cg.set_cellValue(i, j, 1.0);
            input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
        }else{
            throw std::logic_error("There was a problem setting the surface properties in ninja::computeSurfPropForCell().");
        }
    }else if(fuelBedDepth > 0.0)	//just use fuel bed depth
    {
        input.surface.Rough_h.set_cellValue(i, j,  fuelBedDepth);
        input.surface.Rough_d.set_cellValue(i, j, fuelBedDepth*0.63);
        input.surface.Roughness.set_cellValue(i, j, fuelBedDepth*0.13);
        input.surface.Albedo.set_cellValue(i, j, 0.25);	//use rangeland values for heat flux parameters
        input.surface.Bowen.set_cellValue(i, j, 1.0);
        input.surface.Cg.set_cellValue(i, j, 0.15);
        input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
    }else if(canopyHeight > 0.0)	//if there is a canopy height (no fuel model though)
    {
        input.surface.Rough_h.set_cellValue(i, j,  canopyHeight);
        input.surface.Rough_d.set_cellValue(i, j, canopyHeight*0.63);
        input.surface.Roughness.set_cellValue(i, j, canopyHeight*0.13);
        input.surface.Albedo.set_cellValue(i, j, 0.1);	//assume forest land for heat flux parameters
        input.surface.Bowen.set_cellValue(i, j, 1.0);
        input.surface.Cg.set_cellValue(i, j, 0.15);
        input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
    }else		// If we make it to here, we'll just choose parameters based on rangeland...
    {
        input.surface.Rough_h.set_cellValue(i, j,  0.384615);
        input.surface.Rough_d.set_cellValue(i, j, 0.384615*0.63);
        input.surface.Roughness.set_cellValue(i, j, 0.384615*0.13);
        input.surface.Albedo.set_cellValue(i, j, 0.25);
        input.surface.Bowen.set_cellValue(i, j, 1.0);
        input.surface.Cg.set_cellValue(i, j, 0.15);
        input.surface.Anthropogenic.set_cellValue(i, j, 0.0);
    }
}



// Go through logic of determining surface properties, depending on what data is available at this cell
/*
    if (canopyCover >= 0.05 && canopyHeight > 0 )	// if enough cover use the canopy hgt
    {
        (*input.surface.Rough_h.poData)[i][j] = canopyHeight;
        (*input.surface.Rough_d.poData)[i][j] = canopyHeight*0.63;
        (*input.surface.Roughness.poData)[i][j] = canopyHeight*0.13;
        (*input.surface.Albedo.poData)[i][j] = 0.1;	//assuming forest land cover for heat transfer parameters
        (*input.surface.Bowen.poData)[i][j] = 1.0;
        (*input.surface.Cg.poData)[i][j] = 0.15;
        (*input.surface.Anthropogenic.poData)[i][j] = 0.0;

        return true;
    }


    if(fuelModel==90 || fuelModel==91 || fuelModel==92 || fuelModel==93 || fuelModel==98)	// See if it's an unburnable Fuel Model
    {
        switch(fuelModel)
        {
        case 90:	// Barren
            (*input.surface.Rough_h.poData)[i][j] = 0.00230769;
            (*input.surface.Rough_d.poData)[i][j] = 0.00230769*0.63;
            (*input.surface.Roughness.poData)[i][j] = 0.00230769*0.13;
            (*input.surface.Albedo.poData)[i][j] = 0.3;
            (*input.surface.Bowen.poData)[i][j] = 1.0;
            (*input.surface.Cg.poData)[i][j] = 0.15;
            (*input.surface.Anthropogenic.poData)[i][j] = 0.0;
            break;
        case 91:	// Urban Roughness
            (*input.surface.Rough_h.poData)[i][j] = 5.0;
            (*input.surface.Rough_d.poData)[i][j] = 5.0*0.63;
            (*input.surface.Roughness.poData)[i][j] = 5.0*0.13;
            (*input.surface.Albedo.poData)[i][j] = 0.18;
            (*input.surface.Bowen.poData)[i][j] = 1.5;
            (*input.surface.Cg.poData)[i][j] = 0.25;
            (*input.surface.Anthropogenic.poData)[i][j] = 0.0;
            break;
        case 92:	// Snow Ice
            (*input.surface.Rough_h.poData)[i][j] = 0.00076923;
            (*input.surface.Rough_d.poData)[i][j] = 0.00076923*0.63;
            (*input.surface.Roughness.poData)[i][j] = 0.00076923*0.13;
            (*input.surface.Albedo.poData)[i][j] = 0.7;
            (*input.surface.Bowen.poData)[i][j] = 0.5;
            (*input.surface.Cg.poData)[i][j] = 0.15;
            (*input.surface.Anthropogenic.poData)[i][j] = 0.0;
            break;
        case 93:	// Agriculture
            (*input.surface.Rough_h.poData)[i][j] = 1.0;
            (*input.surface.Rough_d.poData)[i][j] = 1.0*0.63;
            (*input.surface.Roughness.poData)[i][j] = 1.0*0.13;
            (*input.surface.Albedo.poData)[i][j] = 0.15;
            (*input.surface.Bowen.poData)[i][j] = 1.0;
            (*input.surface.Cg.poData)[i][j] = 0.15;
            (*input.surface.Anthropogenic.poData)[i][j] = 0.0;
            break;
        case 98:	// Water
            (*input.surface.Rough_h.poData)[i][j] = 0.00153846;
            (*input.surface.Rough_d.poData)[i][j] = 0.00153846*0.63;
            (*input.surface.Roughness.poData)[i][j] = 0.00153846*0.13;
            (*input.surface.Albedo.poData)[i][j] = 0.1;
            (*input.surface.Bowen.poData)[i][j] = 0.0;
            (*input.surface.Cg.poData)[i][j] = 1.0;
            (*input.surface.Anthropogenic.poData)[i][j] = 0.0;
            break;
        default:
            generalError = 35;
            updateGeneralErrorString();
            return false;
            break;
        }

        return true;
    }

    if(fuelBedDepth > 0.0)		//just use fuel bed depth
    {
        (*input.surface.Rough_h.poData)[i][j] = fuelBedDepth;
        (*input.surface.Rough_d.poData)[i][j] = fuelBedDepth*0.63;
        (*input.surface.Roughness.poData)[i][j] = fuelBedDepth*0.13;
        (*input.surface.Albedo.poData)[i][j] = 0.25;	//use rangeland values for heat flux parameters
        (*input.surface.Bowen.poData)[i][j] = 1.0;
        (*input.surface.Cg.poData)[i][j] = 0.15;
        (*input.surface.Anthropogenic.poData)[i][j] = 0.0;

        return true;
    }

    if(canopyHeight > 0.0)	//if there is a canopy height (no fuel model though)
    {
        (*input.surface.Rough_h.poData)[i][j] = canopyHeight;
        (*input.surface.Rough_d.poData)[i][j] = canopyHeight*0.63;
        (*input.surface.Roughness.poData)[i][j] = canopyHeight*0.13;
        (*input.surface.Albedo.poData)[i][j] = 0.1;	//assume forest land for heat flux parameters
        (*input.surface.Bowen.poData)[i][j] = 1.0;
        (*input.surface.Cg.poData)[i][j] = 0.15;
        (*input.surface.Anthropogenic.poData)[i][j] = 0.0;

        return true;
    }

    // If we make it to here, we'll just choose parameters based on rangeland...
    (*input.surface.Rough_h.poData)[i][j] = 0.384615;
    (*input.surface.Rough_d.poData)[i][j] = 0.384615*0.63;
    (*input.surface.Roughness.poData)[i][j] = 0.384615*0.13;
    (*input.surface.Albedo.poData)[i][j] = 0.25;
    (*input.surface.Bowen.poData)[i][j] = 1.0;
    (*input.surface.Cg.poData)[i][j] = 0.15;
    (*input.surface.Anthropogenic.poData)[i][j] = 0.0;

    return true;
}
*/


void ninja::set_uniRoughness(double roughness, lengthUnits::eLengthUnits units)			//set uniform values of Roughness
{
    if(roughness <= 0)	//error check
        throw std::range_error("Roughness is less than zero in ninja::set_uniRoughness().");
    else
    {
        input.surface.RoughnessUnits = units;
        input.surface.Roughness.set_headerData( input.dem );
        lengthUnits::toBaseUnits(roughness, units);
        input.surface.Roughness = roughness;
    }
}

void ninja::set_uniRoughH(double rough_h, lengthUnits::eLengthUnits units)				//set uniform values of RoughH
{
    if(rough_h < 0)	//error check
        throw std::range_error("Roughness height parameter is less than zero in ninja::set_uniRoughH().");
    else{
        input.surface.Rough_hUnits = units;
        input.surface.Rough_h.set_headerData( input.dem );
        lengthUnits::toBaseUnits(rough_h, units);
        input.surface.Rough_h = rough_h;
    }
}

void ninja::set_uniRoughD(double rough_d, lengthUnits::eLengthUnits units)				//set uniform values of RoughD
{
    if(rough_d < 0)	//error check
        throw std::range_error("Roughness height parameter is less than zero in ninja::set_uniRoughD().");
    else{
        input.surface.Rough_dUnits = units;
        input.surface.Rough_d.set_headerData( input.dem );
        lengthUnits::toBaseUnits(rough_d, units);
        input.surface.Rough_d = rough_d;
    }
}

void ninja::set_uniAlbedo(double albedo)				//set uniform values of Albedo
{
    if((albedo < 0) || (albedo > 1.0))	//error check
        throw std::range_error("Albedo is less than zero or greater than one in ninja::set_uniAlbedo().");
    else{
        input.surface.Albedo.set_headerData(input.dem);
        input.surface.Albedo = albedo;
    }
}

void ninja::set_uniBowen(double bowen)				//set uniform values of Bowen
{
    if((bowen < 0) || (bowen > 1.0))	//error check
        throw std::range_error("Bowen ratio is less than zero or greater than one in ninja::set_uniBowen().");
    else{
        input.surface.Bowen.set_headerData(input.dem);
        input.surface.Bowen = bowen;
    }
}

void ninja::set_uniCg(double cg)					//set uniform values of Cg
{
    if((cg < 0) || (cg > 1.0))	//error check
        throw std::range_error("Cg parameter is less than zero or greater than one in ninja::set_uniCg().");
    else{
        input.surface.Cg.set_headerData(input.dem);
        input.surface.Cg = cg;
    }
}

void ninja::set_uniAnthropogenic(double anthropogenic)	//set uniform values of Anthropogenic
{
    if((anthropogenic < 0) || (anthropogenic > 1.0))	//error check
        throw std::range_error("Anthropogenic heat flux parameter is less than zero or greater than one in ninja::set_uniAnthropogenic().");
    else{
        input.surface.Anthropogenic.set_headerData(input.dem);
        input.surface.Anthropogenic = anthropogenic;
    }
}

void ninja::set_inputSpeed(double speed, velocityUnits::eVelocityUnits units)
{
    //function reads input windspeed in any units and converts and stores it in m/s in the WindNinjaInputs class

    if(speed < 0)	//error check
        throw std::range_error("Input wind speed is less than zero in ninja::set_inputSpeed().");

    input.inputSpeedUnits = units;	//set inputSpeedUnits in WindNinjaInputs class
    velocityUnits::toBaseUnits(speed, units);
    input.inputSpeed = speed;
    input.surface.set_windspeed( input.inputSpeed );	//set windspeed in surface class for later use in diurnal calcs (in m/s)
}

#ifdef EMISSIONS
const std::string ninja::get_DustFileName() const
{
    return input.dustFile;
}
const std::string ninja::get_GeotiffFileName() const
{
    return input.geotiffOutFilename;
}
#endif
#ifdef FRICTION_VELOCITY
const std::string ninja::get_UstarFileName() const
{
    return input.ustarFile;
}
#endif

const std::string ninja::get_VelFileName() const
{
    return input.velFile;
}

const std::string ninja::get_AngFileName() const
{
    return input.angFile;
}

const std::string ninja::get_CldFileName() const
{
    return input.cldFile;
}

void ninja::set_inputDirection(double direction)
{
    if(direction<0.0 || direction>360.0)	//error checking
        throw std::range_error("Wind direction is less than zero or greater than 360 in ninja::set_inputDirection().");

    input.inputDirection = direction;
}

void ninja::set_inputWindHeight(double height, lengthUnits::eLengthUnits units)
{
    //sets the input wind height above the top of the vegetation
    if(height < 0.0)
        throw std::range_error("Height is less than zero in ninja::set_inputWindHeight().");

    input.inputWindHeightUnits = units;
    lengthUnits::toBaseUnits(height, units);
    input.inputWindHeight = height;
    input.surface.Z = input.inputWindHeight;	//set input windspeed height in surface class for later use in diurnal calcs (in meters)
}
void ninja::set_inputWindHeight(double height)
{
    if( height < 0.0 )
    {
        throw std::range_error( "Height is less than zero in ninja::set_inputWindHeight().");
    }
    input.inputWindHeight = height;
    return;
}
void ninja::set_outputSpeedUnits(velocityUnits::eVelocityUnits units)
{
    input.outputSpeedUnits = units;
    return;
}

velocityUnits::eVelocityUnits ninja::get_outputSpeedUnits() const
{
    return input.outputSpeedUnits;
}


void ninja::set_outputWindHeight(double height, lengthUnits::eLengthUnits units)
{
    //sets the output wind height above the top of the vegetation
    if(height < 0.0)
        throw std::range_error("Height is less than zero in ninja::set_outputWindHeight().");

    input.outputWindHeightUnits = units;
    lengthUnits::toBaseUnits(height, units);
    input.outputWindHeight = height;
}

double ninja::get_outputWindHeight() const
{
    return input.outputWindHeight;
}

void ninja::set_diurnalWinds(bool flag)
{
   input.diurnalWinds = flag;
}
bool ninja::get_diurnalWindFlag()
{
    return input.diurnalWinds;
}

void ninja::set_date_time(int const &yr, int const &mo, int const &day, int const &hr,
                          int const &min, int const &sec, std::string const &timeZoneString)
{
  input.ninjaTimeZone =
      globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( NULL ==  input.ninjaTimeZone )
    {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str() << " does not match any in "
                << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error(os.str());
    }

    input.ninjaTime = boost::local_time::local_date_time( boost::gregorian::date(yr, mo, day),
            boost::posix_time::time_duration(hr,min,sec,0),
            input.ninjaTimeZone,
            boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    if(input.ninjaTime.is_not_a_date_time())
        throw std::runtime_error("Time could not be properly set in ninja::set_date_time().");
}

void ninja::set_date_time(boost::local_time::local_date_time time)
{
    input.ninjaTime = time;
    input.ninjaTimeZone = input.ninjaTime.zone();

    if(input.ninjaTime.is_not_a_date_time())
        throw std::runtime_error("Time could not be properly set in ninja::set_date_time().");
}

void ninja::set_simulationStartTime(int const &yr, int const &mo, int const &day, int const &hr,
                          int const &min, int const &sec, std::string const &timeZoneString)
{
  input.ninjaTimeZone =
      globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( NULL ==  input.ninjaTimeZone )
    {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str() << " does not match any in "
                << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error(os.str());
    }

    input.simulationStartTime = boost::local_time::local_date_time( boost::gregorian::date(yr, mo, day),
            boost::posix_time::time_duration(hr,min,sec,0),
            input.ninjaTimeZone,
            boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    if(input.simulationStartTime.is_not_a_date_time())
        throw std::runtime_error("Time could not be properly set in ninja::set_simulationStartTime().");
}

void ninja::set_simulationStopTime(int const &yr, int const &mo, int const &day, int const &hr,
                          int const &min, int const &sec, std::string const &timeZoneString)
{
  input.ninjaTimeZone =
      globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( NULL ==  input.ninjaTimeZone )
    {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str() << " does not match any in "
                << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error(os.str());
    }

    input.simulationStopTime = boost::local_time::local_date_time( boost::gregorian::date(yr, mo, day),
            boost::posix_time::time_duration(hr,min,sec,0),
            input.ninjaTimeZone,
            boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    if(input.simulationStopTime.is_not_a_date_time())
        throw std::runtime_error("Time could not be properly set in ninja::set_simulationStopTime().");
}

void ninja::set_simulationOutputFrequency( int const &hr, int const &min, int const &sec )
{
    input.simulationOutputFrequency = boost::posix_time::time_duration(hr,min,sec,0);
}

boost::local_time::local_date_time ninja::get_date_time() const
{
    return input.ninjaTime;
}

void ninja::set_uniAirTemp(double temp, temperatureUnits::eTempUnits units)
{
    //set uniform air temperature, always stored in K
    input.airTempUnits = units;
    temperatureUnits::toBaseUnits(temp, units);
    input.airTemp = temp;
}

void ninja::set_uniCloudCover(double cloud_cover, coverUnits::eCoverUnits units)
{
    //Check cloud cover
    if(units == coverUnits::percent)
    {
        if(cloud_cover<0.0 || cloud_cover>100.0)
        {
            std::stringstream outText;
            outText << "Cloud cover set to " << cloud_cover << ".  It should not be less than 0 percent or greater than 100 percent." << endl;
            throw std::range_error(outText.str());
        }
    }
    else if(units == coverUnits::fraction)
    {
        if(cloud_cover<0.0 || cloud_cover>1.0)
        {
            std::stringstream outText;
            outText << "Cloud cover set to " << cloud_cover << ".  It should not be less than zero or greater than 1." << endl;
            throw std::range_error(outText.str());
        }
    }else if(units == coverUnits::canopyCategories)
    {
        if(cloud_cover!=0.0 && cloud_cover!=99.0 &&
           cloud_cover!=1.0 && cloud_cover!=2.0 &&
           cloud_cover!=3.0 && cloud_cover!=4.0)
        {
            std::stringstream outText;
            outText << "Cloud cover set to category" << cloud_cover <<
                ".  It must be set to 0, 1, 2, 3, 4, or 99." << endl;
            throw std::range_error(outText.str());
        }
    }
    else{
            std::stringstream outText;
            outText << "Cloud cover units cannot be identified." << endl;
            throw std::range_error(outText.str());
    }

    //Change to base units of fraction
    input.cloudCoverUnits = units;
    coverUnits::toBaseUnits(cloud_cover, units);
    input.cloudCover = cloud_cover;
}

/**
 * Sets the in-memory datasets for GTiff output writer.
 * @param hSpdMemDS Name of the in-memory speed dataset.
 * @param hDirMemDS Name of the in-memory direction dataset.
 * @param hDustMemDS Name of the in-memory dust dataset.
 */
void ninja::set_memDs(GDALDatasetH hSpdMemDs, GDALDatasetH hDirMemDs, GDALDatasetH hDustMemDs)
{
    input.hSpdMemDs = hSpdMemDs;
    input.hDirMemDs = hDirMemDs;
    input.hDustMemDs = hDustMemDs;
}

/**
 * Sets the flag indicating whether station fetch is on or off 
 * @param flag true if station fetch is enbaled, otherwise false 
 */
void ninja::set_stationFetchFlag(bool flag)
{
//    cout<<"ninja: set_stationFetch=="<<flag<<endl;
    input.stationFetch=flag;   
}


/**
 * Sets the filename for a weather model initialization run.
 * @param forecast_initialization_filename Name of forecast file.  This typically is a
 * relative path starting from the directory where the elevation file is.
 */
void ninja::set_wxModelFilename(const std::string& forecast_initialization_filename)
{
    if(forecast_initialization_filename.empty())
        throw std::runtime_error("The weather model initialization file has not been set.");

    input.forecastFilename = forecast_initialization_filename;
}
/**
 * Sets and reads the weather station(s) from station_filename.
 * @param station_filename Name of stations file.
 */
void ninja::set_wxStationFilename(std::string station_filename)
{
    if(station_filename.empty())
        throw std::runtime_error("Weather station filename empty in ninja::set_wxStationFilename().");
    input.wxStationFilename = station_filename;

    input.stationsScratch = input.stations;
    input.stationsOldInput = input.stations;
    input.stationsOldOutput = input.stations;

    input.inputSpeedUnits = input.stations[0].get_speedUnits(); //set inputSpeedUnits in ninja class to first station.
}


/**
 * Returns the list of stations. Should be run after set_wxStationFilename.
 * @return stations A vector of wxStation objects
 */
std::vector<wxStation> ninja::get_wxStations()
{
    return input.stations;
}


/** Function sets the vector of weather stations.
 * @param wxStations Reference to input weather station vector.
 */
void ninja::set_wxStations(std::vector<wxStation> &wxStations)
{
    input.stations = wxStations;
    for (unsigned int i = 0; i < input.stations.size(); i++)
    {
        if (wxStation::check_station(input.stations[i])==false)
        {
            throw std::range_error("Error in weather station parameters.");
        }
    }
}
void ninja::set_meshResChoice( std::string choice )
{
    if( choice == std::string( "coarse" ) )
    {
       mesh.set_meshResChoice( Mesh::coarse );
    }
    else if( choice == std::string( "medium" ) )
    {
        mesh.set_meshResChoice( Mesh::medium );
    }
    else if( choice == std::string( "fine" ) )
    {
        mesh.set_meshResChoice( Mesh::fine );
    }
    else
        throw std::invalid_argument( "Invalid input '" + choice +
                                    "' in ninja::set_meshResChoice" );
}
void ninja::set_meshResChoice( const Mesh::eMeshChoice choice )
{
    mesh.set_meshResChoice( choice );
}
void ninja::set_meshResolution( double resolution, lengthUnits::eLengthUnits units )
{
    mesh.set_meshResolution( resolution, units );
}

double ninja::get_meshResolution()
{
    return mesh.meshResolution;
}

void ninja::set_numVertLayers( const int nLayers )
{
    mesh.set_numVertLayers( nLayers );
}
/**
 * Use the dem file to extract the center of the domain.
 *
 * @return false on failure, true otherwise
 * @overload
 */
bool ninja::set_position()
{
    /*
     * Open the dataset
     */
    GDALDataset *poDS = (GDALDataset *)GDALOpen(input.dem.fileName.c_str(),
                        GA_ReadOnly);

    if(poDS == NULL)
    throw std::runtime_error("Error in ninja::set_position() trying to find the center of the elevation file.");

    double longitude, latitude;

    if( GDALGetCenter( poDS, &longitude, &latitude ) )
    {
        // XXX: note y/x axis ordering
        set_position(latitude, longitude);
        GDALClose( (GDALDatasetH)poDS );
        return true;
    }
    else
    {
        GDALClose( (GDALDatasetH)poDS );
        return false;
  }
}

void ninja::add_WxStation(wxStation const& Station)
{
    input.stations.push_back(Station);
}

/**
 * Set the center of the domain.
 *
 * @param lat_degrees decimal degrees latitude
 * @param long_degrees decimal degrees longitude
 * @overload double, double
 */
void ninja::set_position(double lat_degrees, double long_degrees)
{
    if(lat_degrees<-90.0 ||lat_degrees>90.0)
        throw std::range_error("Latitude greater than 90 degrees or "
                               "less than -90 degrees in ninja::set_position().");

    if(long_degrees<-180.0 ||long_degrees>180.0)
        throw std::range_error("Longitude greater than 180 degrees or "
                               "less than -180 degrees in ninja::set_position().");

    input.latitude  = lat_degrees;
    input.longitude = long_degrees;
}

/**
 * Set the center of the domain.
 *
 * @param lat_degrees latitude degrees
 * @param lat_minutes latitude minutes
 * @param long_degrees longitude degrees
 * @param long_minutes longitude minutes
 * @overload double, double, double, double
 */
void ninja::set_position(double lat_degrees, double lat_minutes, double long_degrees, double long_minutes)
{
    double x;

    if(lat_degrees >= 0.0)
    {
        x              = lat_minutes/60.0;
        input.latitude = lat_degrees + x;
    }
    else
    {
        x              = lat_minutes/60.0;
        input.latitude = lat_degrees - x;
    }

    if(long_degrees >= 0.0)
    {
        x               = long_minutes/60.0;
        input.longitude = long_degrees + x;
    }
    else
    {
        x               = long_minutes/60.0;
        input.longitude = long_degrees - x;
    }

    if( input.latitude < -90.0 || input.latitude > 90.0 )
        throw std::range_error("Latitude greater than 90 degrees or "
                               "less than -90 degrees in ninja::set_position().");

    if( input.longitude < -180.0 || input.longitude>180.0 )
        throw std::range_error("Longitude greater than 180 degrees or "
                               "less than -180 degrees in ninja::set_position().");
}

void ninja::set_position(double lat_degrees, double lat_minutes, double lat_seconds,
                         double long_degrees, double long_minutes, double long_seconds)
{
    double x,xx;

    if(lat_degrees >= 0.0)
    {
        xx             = lat_seconds/60.0;
        lat_minutes   += xx;
        x              = lat_minutes/60.0;
        input.latitude = lat_degrees + x;
    }
    else
    {
        xx             = lat_seconds/60.0;
        lat_minutes   += xx;
        x              = lat_minutes/60.0;
        input.latitude = lat_degrees - x;
    }

    if(long_degrees >= 0.0)
    {
        xx              = long_seconds/60.0;
        long_minutes   += xx;
        x               = long_minutes/60.0;
        input.longitude = long_degrees + x;
    }
    else
    {
        xx              = long_seconds/60.0;
        long_minutes   += xx;
        x               = long_minutes/60.0;
        input.longitude = long_degrees - x;
    }
    if( input.latitude < -90.0 || input.latitude > 90.0 )
        throw std::range_error("Latitude greater than 90 degrees or "
                               "less than -90 degrees in ninja::set_position().");

    if( input.longitude < -180.0 || input.longitude > 180.0 )
        throw std::range_error("Longitude greater than 180 degrees or "
                               "less than -180 degrees in ninja::set_position().");
}

void ninja::set_numberCPUs(int CPUs)
{
    if(CPUs < 1)
        throw std::range_error("Number of CPUs is less than one in ninja::set_numberCPUs().");

    input.numberCPUs = CPUs;

    #ifdef _OPENMP
    omp_set_num_threads( input.numberCPUs );
    /*
        if(omp_in_parallel())
        {
            //omp_set_nested(false);
            //omp_set_dynamic(true);
            omp_set_num_threads(1);
            //ninjaCom(ninjaComClass::ninjaDebug, "IN OMP PARALLEL REGION AND SHOULDN'T BE!!!!");
        }else{
            //omp_set_nested(true);
            //omp_set_dynamic(true);
            omp_set_num_threads(numberCPUs);
            //ninjaCom(ninjaComClass::ninjaDebug, "Max number of threads = %d\nDynamic = %d\nNested = %d", omp_get_max_threads(), omp_get_dynamic(), omp_get_nested());
        }
    */
    #endif
    //ninjaCom(ninjaComClass::ninjaDebug, "In parallel = %d\n", omp_in_parallel());
    //	#pragma omp parallel
    //	{
    //		#pragma omp single
    //		{
    //			ninjaCom(ninjaComClass::ninjaDebug, "Number of threads = %d", omp_get_num_threads());
    //		}
    //	}
    //ninjaCom(ninjaComClass::ninjaDebug, "In parallel = %d", omp_in_parallel());
}

void ninja::set_outputBufferClipping(double percent)
{
    if(percent < 0.0 || percent >= 50.0)
    {
        std::ostringstream buff_str;
        buff_str << "The output file buffer clipping is improperly set to "
                 << percent << ".  It should be 0-50 percent.";
        throw std::out_of_range(buff_str.str().c_str());
    }
    input.outputBufferClipping = percent;
}

void ninja::set_writeAtmFile(bool flag)
{
    input.writeAtmFile = flag;
}

void ninja::set_googOutFlag(bool flag)
{
    input.googOutFlag = flag;
}

void ninja::set_googColor(std::string scheme, bool scaling)
{
    input.googColor = scheme;
    input.googVectorScale = scaling;
}



void ninja::set_googSpeedScaling(KmlVector::egoogSpeedScaling scaling)
{
    if(scaling != KmlVector::equal_color && scaling != KmlVector::equal_interval)
        throw std::logic_error("Google speed scaling parameter not set properly"
                               " in ninja::set_googSpeedScaling().");

    input.googSpeedScaling = scaling;
}

void ninja::set_googLineWidth(double width)
{
    if(width < 0.0 || width > 100.0)
        throw std::range_error("Line width out of allowed range in "
                               "ninja::set_googLineWidth().");
    else
        input.googLineWidth = width;
}

void ninja::set_wxModelGoogOutFlag(bool flag)
{
    input.wxModelGoogOutFlag = flag;
}

void ninja::set_wxModelGoogSpeedScaling(KmlVector::egoogSpeedScaling scaling)
{
    if(scaling != KmlVector::equal_color && scaling != KmlVector::equal_interval)
        throw std::logic_error("Weather model google speed scaling "
                "parameter not set properly in ninja::set_wxModelGoogSpeedScaling().");

    input.wxModelGoogSpeedScaling = scaling;
}

void ninja::set_wxModelGoogLineWidth(double width)
{
    if(width < 0.0 || width > 100.0)
        throw std::range_error("Line width out of allowed range in "
                               "ninja::set_wxModelGoogLineWidth().");
    else
       input.wxModelGoogLineWidth = width;
}

void ninja::set_googResolution(double Resolution, lengthUnits::eLengthUnits units)
{
    input.kmzUnits = units;
    lengthUnits::toBaseUnits(Resolution, units);

    input.kmzResolution = Resolution;
}

void ninja::set_shpOutFlag(bool flag)
{
    input.shpOutFlag = flag;
}

void ninja::set_wxModelShpOutFlag(bool flag)
{
    input.wxModelShpOutFlag = flag;
}

void ninja::set_shpResolution(double Resolution, lengthUnits::eLengthUnits units)
{
    input.shpUnits = units;
    lengthUnits::toBaseUnits(Resolution, units);

    input.shpResolution = Resolution;
}

void ninja::set_pdfOutFlag(bool flag)
{
    input.pdfOutFlag = flag;
}

void ninja::set_pdfResolution(double Resolution, lengthUnits::eLengthUnits units)
{
    input.pdfUnits = units;
    lengthUnits::toBaseUnits(Resolution, units);

    input.pdfResolution = Resolution;
}

void ninja::set_pdfLineWidth(const float w)
{
    input.pdfLineWidth = w;
}

void ninja::set_pdfBaseMap(const int b)
{
    input.pdfBaseType = (WindNinjaInputs::ePDFBaseMap)b;
}

void ninja::set_pdfSize( const double height, const double width, const unsigned short dpi )
{
    input.pdfHeight = height;
    input.pdfWidth = width;
    input.pdfDPI = dpi;
}

void ninja::set_pdfDEM(std::string dem_file_name)
{
    /*
    if(!CPLCheckForFile((char*)dem_file_name.c_str(), NULL))
        throw std::runtime_error(std::string("The file ") +
                dem_file_name + " does not exist or may be in use by another program.");
    */
    input.pdfDEMFileName = dem_file_name;
}


void ninja::set_asciiOutFlag(bool flag)
{
    input.asciiOutFlag = flag;
}

void ninja::set_wxModelAsciiOutFlag(bool flag)
{
    input.wxModelAsciiOutFlag = flag;
}

void ninja::set_asciiResolution(double Resolution, lengthUnits::eLengthUnits units)
{
    input.velOutputFileDistanceUnits = units;
    input.angOutputFileDistanceUnits = units;
    lengthUnits::toBaseUnits(Resolution, units);

    input.velResolution = input.angResolution = Resolution;
}

void ninja::set_txtOutFlag(bool flag)
{
    input.txtOutFlag = flag;
}

void ninja::set_vtkOutFlag(bool flag)
{
    input.volVTKOutFlag = flag;
}

void ninja::set_outputPath(std::string path)
{
    VSIStatBufL sStat;
    VSIStatL( path.c_str(), &sStat );
    const char *pszTestPath = CPLFormFilename(path.c_str(), "NINJA_TEST", "");
    int nRet;
    
    if( VSI_ISDIR( sStat.st_mode ) ){
        //see if we can write to this path
        nRet = VSIMkdir(pszTestPath, 0777);
        if(nRet == 0){
            VSIRmdir(pszTestPath);
            input.customOutputPath = path;
        }
        else{
            input.Com->ninjaCom(ninjaComClass::ninjaNone, 
            CPLSPrintf("Cannot write to path %s. Do you have write permission on this directory? Writing outputs to default location...", path.c_str()));
        }
    }
    else{
        input.Com->ninjaCom(ninjaComClass::ninjaNone, 
            CPLSPrintf("The path %s does not exist, writing outputs to default location...", path.c_str()));
    }
        
}

void ninja::set_outputFilenames(double& meshResolution,
                                lengthUnits::eLengthUnits meshResolutionUnits)
{
    //Set output file resolutions now
    if( input.kmzResolution <= 0.0 )  //if negative, use computational mesh resolution
        input.kmzResolution = meshResolution;
    if( input.shpResolution <= 0.0 )  //if negative, use computational mesh resolution
        input.shpResolution = meshResolution;
    if( input.velResolution <= 0.0 )  //if negative, use computational mesh resolution
        input.velResolution = meshResolution;
    if( input.angResolution <= 0.0 )  //if negative, use computational mesh resolution
        input.angResolution = meshResolution;
    if( input.pdfResolution <= 0.0 )
        input.pdfResolution = meshResolution;

    //Do file naming string stuff for all output files
    std::string rootFile, rootName, fileAppend, timeAppend, wxModelTimeAppend, kmz_fileAppend, \
        shp_fileAppend, ascii_fileAppend, volVTK_fileAppend, mesh_units, kmz_mesh_units, \
        shp_mesh_units, ascii_mesh_units, pdf_fileAppend, pdf_mesh_units;

    boost::local_time::local_time_facet* timeOutputFacet;
    timeOutputFacet = new boost::local_time::local_time_facet();
    //NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
    //		THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
    //		https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
    //		http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203

    std::ostringstream timestream;
    timestream.imbue(std::locale(std::locale::classic(), timeOutputFacet));
    timeOutputFacet->format("%m-%d-%Y_%H%M_");

    if( input.diurnalWinds == true ||
        input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag )
    {
        timestream << input.ninjaTime;
    }
    else if( input.initializationMethod == WindNinjaInputs::pointInitializationFlag )
    {
        if(wxStation::stationFormat == wxStation::newFormat)
        {
            timestream << input.ninjaTime;
        }
    }
    else if( input.stabilityFlag == true && input.alphaStability == -1 )
        timestream << input.ninjaTime;

    std::string pathName;
    std::string baseName(CPLGetBasename(input.dem.fileName.c_str()));
    
    if(input.customOutputPath == "!set"){ // if a custom output path was not specified in the cli
        if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag ||
            input.initializationMethod == WindNinjaInputs::foamWxModelInitializationFlag )	//prepend directory paths to rootFile for wxModel run
        {
            pathName = CPLGetPath(input.forecastFilename.c_str());
            //if it's a .tar, write to directory containing the .tar file
            if( strstr(pathName.c_str(), ".tar") ){
                pathName.erase( pathName.rfind("/") );
            }
        }else{
            pathName = CPLGetPath(input.dem.fileName.c_str());
        }
    }
    else{ // if a custom output path was specified in the cli
        pathName = input.customOutputPath;
    }
    
    rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);
    
    /* set the output path member variable */
    input.outputPath = pathName;

    //stringPos = (int)input.demFile.find_last_of('\\');
    //if (stringPos > 0)
    //{
    //	fullPath = demFile;
    //	fullPath.erase(stringPos);
    //}
    //else
    //	fullPath = "";


    timeAppend = timestream.str();
    ostringstream wxModelTimestream;
    boost::local_time::local_time_facet* wxModelOutputFacet;
    wxModelOutputFacet = new boost::local_time::local_time_facet();
    wxModelTimestream.imbue(std::locale(std::locale::classic(), wxModelOutputFacet));
    wxModelOutputFacet->format("%m-%d-%Y_%H%M");
    if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag ||
        input.initializationMethod == WindNinjaInputs::foamWxModelInitializationFlag )
    {
        wxModelTimestream << input.ninjaTime;
    }
    wxModelTimeAppend = wxModelTimestream.str();

    mesh_units = lengthUnits::getString( meshResolutionUnits );
    kmz_mesh_units = lengthUnits::getString( input.kmzUnits );
    shp_mesh_units = lengthUnits::getString( input.shpUnits );
    ascii_mesh_units = lengthUnits::getString( input.velOutputFileDistanceUnits );
    pdf_mesh_units   = lengthUnits::getString( input.pdfUnits );

    ostringstream os, os_kmz, os_shp, os_ascii, os_pdf;
    if( input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag ||
        input.initializationMethod == WindNinjaInputs::foamDomainAverageInitializationFlag )
    {
        double tempSpeed = input.inputSpeed;
        velocityUnits::fromBaseUnits(tempSpeed, input.inputSpeedUnits);
        os << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_kmz << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_shp << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_ascii << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_pdf << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
    }
    else if( input.initializationMethod == WindNinjaInputs::pointInitializationFlag )
    {
        os << "_point";
        os_kmz << "_point";
        os_shp << "_point";
        os_ascii << "_point";
        os_pdf   << "_point";
    }


    double meshResolutionTemp = meshResolution;
    double kmzResolutionTemp = input.kmzResolution;
    double shpResolutionTemp = input.shpResolution;
    double velResolutionTemp = input.velResolution;
    double pdfResolutionTemp = input.pdfResolution;

    lengthUnits::fromBaseUnits(meshResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(kmzResolutionTemp, input.kmzUnits);
    lengthUnits::fromBaseUnits(shpResolutionTemp, input.shpUnits);
    lengthUnits::fromBaseUnits(velResolutionTemp, input.velOutputFileDistanceUnits);
    lengthUnits::fromBaseUnits(pdfResolutionTemp, input.pdfUnits);

    os << "_" << timeAppend << (long) (meshResolutionTemp+0.5)  << mesh_units;
    os_kmz << "_" << timeAppend << (long) (kmzResolutionTemp+0.5)  << kmz_mesh_units;
    os_shp << "_" << timeAppend << (long) (shpResolutionTemp+0.5)  << shp_mesh_units;
    os_ascii << "_" << timeAppend << (long) (velResolutionTemp+0.5)  << ascii_mesh_units;
    os_pdf << "_" << timeAppend << (long) (pdfResolutionTemp+0.5)    << pdf_mesh_units;

    if( input.stabilityFlag == true && input.alphaStability != -1 )
    {
        os       << "_alpha_" << input.alphaStability;
        os_kmz   << "_alpha_" << input.alphaStability;
        os_shp   << "_alpha_" << input.alphaStability;
        os_ascii << "_alpha_" << input.alphaStability;
        os_pdf   << "_alpha_" << input.alphaStability;
    }
    else if( input.stabilityFlag == true && input.alphaStability == -1 )
    {
        os       << "_non_neutral_stability";
        os_kmz   << "_non_neutral_stability";
        os_shp   << "_non_neutral_stability";
        os_ascii << "_non_neutral_stability";
        os_pdf   << "_non_neutral_stability";
    }

    fileAppend = os.str();
    kmz_fileAppend = os_kmz.str();
    shp_fileAppend = os_shp.str();
    ascii_fileAppend = os_ascii.str();
    pdf_fileAppend   = os_pdf.str();


    input.kmlFile = rootFile + kmz_fileAppend + ".kml";
    input.kmzFile = rootFile + kmz_fileAppend + ".kmz";

    //wxModelKmlFile = wxModelTimeAppend + ".kml";
    //wxModelKmzFile = wxModelTimeAppend + ".kmz";

    input.shpFile = rootFile + shp_fileAppend + ".shp";
    input.dbfFile = rootFile + shp_fileAppend + ".dbf";

    input.pdfFile = rootFile + pdf_fileAppend + ".pdf";

    //wxModelShpFile = wxModelTimeAppend + ".shp";
    //wxModelDbfFile = wxModelTimeAppend + ".dbf";

    input.cldFile = rootFile + ascii_fileAppend + "_cld.asc";
    input.velFile = rootFile + ascii_fileAppend + "_vel.asc";
    input.angFile = rootFile + ascii_fileAppend + "_ang.asc";
    input.atmFile = rootFile + ascii_fileAppend + ".atm";

    #ifdef FRICTION_VELOCITY
    input.ustarFile = rootFile + ascii_fileAppend + "_ustar.asc";
    #endif

    #ifdef EMISSIONS
    input.dustFile = rootFile + ascii_fileAppend + "_dust.asc";
    #endif //EMISSIONS

    //wxModelCldFile = "wxModel" + wxModelTimeAppend + "_cld.asc";
    //wxModelVelFile = "wxModel" + wxModelTimeAppend + "_vel.asc";
    //wxModelAngFile = "wxModel" + wxModelTimeAppend + "_ang.asc";

    input.volVTKFile = rootFile + fileAppend + ".vtk";

    input.legFile = rootFile + kmz_fileAppend + ".bmp";
    if( input.ninjaTime.is_not_a_date_time() )	//date and time not set?
        input.dateTimeLegFile = "";
    else
        input.dateTimeLegFile = rootFile + kmz_fileAppend + ".date_time" + ".bmp";

    input.wxModelLegFile = wxModelTimeAppend + ".bmp";
    if( input.ninjaTime.is_not_a_date_time() )	//date and time not set?
        input.dateTimewxModelLegFile = "";
    else
        input.dateTimewxModelLegFile = wxModelTimeAppend + ".date_time" + ".bmp";
}

const std::string ninja::get_outputPath() const
{
    return input.outputPath;
}

/**
 * Function that sets a flag to determine whether or not the final
 * output AsciiGrids of AngleGrid, VelocityGrid, and CloudGrid should
 * be stored after simulate_wind() or deallocated.  It should be set
 * to true only if they are needed after simulate_wind(), which is
 * normally only for a dll type run.  For other runs they are deallocated
 * to save memory during multirun simulations.
 * @param flag True to keep grids, false to deallocate grids.
 */
void ninja::keepOutputGridsInMemory(bool flag)
{
    input.keepOutGridsInMemory = flag;
}

double ninja::getFuelBedDepth(int fuelModel)
{	//at this point must be in meters...  could change...

    //TODO: add units info, turn into table so there arent >200 branches
    if(fuelModel == 1)
        return 1.000000;
    else if(fuelModel == 2)
        return 1.000000;
    else if(fuelModel == 3)
        return 2.500000;
    else if(fuelModel == 4)
        return 6.000000;
    else if(fuelModel == 5)
        return 2.000000;
    else if(fuelModel == 6)
        return 2.500000;
    else if(fuelModel == 7)
        return 2.500000;
    else if(fuelModel == 8)
        return 0.200000;
    else if(fuelModel == 9)
        return 0.200000;
    else if(fuelModel == 10)
        return 1.000000;
    else if(fuelModel == 11)
        return 1.000000;
    else if(fuelModel == 12)
        return 2.300000;
    else if(fuelModel == 13)
        return 3.000000;
    else if(fuelModel == 101)
        return 0.400000;
    else if(fuelModel == 102)
        return 1.000000;
    else if(fuelModel == 103)
        return 2.000000;
    else if(fuelModel == 104)
        return 2.000000;
    else if(fuelModel == 105)
        return 1.500000;
    else if(fuelModel == 106)
        return 1.500000;
    else if(fuelModel == 107)
        return 3.000000;
    else if(fuelModel == 108)
        return 4.000000;
    else if(fuelModel == 109)
        return 5.000000;
    else if(fuelModel == 121)
        return 0.900000;
    else if(fuelModel == 122)
        return 1.500000;
    else if(fuelModel == 123)
        return 1.800000;
    else if(fuelModel == 124)
        return 2.100000;
    else if(fuelModel == 141)
        return 1.000000;
    else if(fuelModel == 142)
        return 1.000000;
    else if(fuelModel == 143)
        return 2.400000;
    else if(fuelModel == 144)
        return 3.000000;
    else if(fuelModel == 145)
        return 6.000000;
    else if(fuelModel == 146)
        return 2.000000;
    else if(fuelModel == 147)
        return 6.000000;
    else if(fuelModel == 148)
        return 3.000000;
    else if(fuelModel == 149)
        return 4.400000;
    else if(fuelModel == 161)
        return 0.600000;
    else if(fuelModel == 162)
        return 1.000000;
    else if(fuelModel == 163)
        return 1.300000;
    else if(fuelModel == 164)
        return 0.500000;
    else if(fuelModel == 165)
        return 1.000000;
    else if(fuelModel == 181)
        return 0.200000;
    else if(fuelModel == 182)
        return 0.200000;
    else if(fuelModel == 183)
        return 0.300000;
    else if(fuelModel == 184)
        return 0.400000;
    else if(fuelModel == 185)
        return 0.600000;
    else if(fuelModel == 186)
        return 0.300000;
    else if(fuelModel == 187)
        return 0.400000;
    else if(fuelModel == 188)
        return 0.300000;
    else if(fuelModel == 189)
        return 0.600000;
    else if(fuelModel == 201)
        return 1.000000;
    else if(fuelModel == 202)
        return 1.000000;
    else if(fuelModel == 203)
        return 1.200000;
    else if(fuelModel == 204)
        return 2.700000;
    else
        return -1.0;
}

void ninja::set_ninjaCommunication(int RunNumber, ninjaComClass::eNinjaCom comType)
{
    input.inputsComType = comType;

    if(input.Com)
        delete input.Com;

    if(comType == ninjaComClass::ninjaDefaultCom)
        input.Com = new ninjaDefaultComHandler();
    else if(comType == ninjaComClass::ninjaQuietCom)
        input.Com = new ninjaQuietComHandler();
    else if(comType == ninjaComClass::ninjaLoggingCom)
        input.Com = new ninjaLoggingComHandler();
    else if(comType == ninjaComClass::ninjaGUICom)
        input.Com = new ninjaGUIComHandler();
    else if(comType == ninjaComClass::WFDSSCom)
        input.Com = new ninjaWFDSSComHandler();
    else if(comType == ninjaComClass::ninjaCLICom)
        input.Com = new ninjaCLIComHandler();
    else
        input.Com = new ninjaDefaultComHandler();

    input.inputsRunNumber = RunNumber;
    input.Com->runNumber = &input.inputsRunNumber;
    input.Com->lastMsg = input.lastComString;
}

void ninja::checkInputs()
{
    //Check DEM
    GDALDataset *poDS;
    poDS = (GDALDataset*)GDALOpen(input.dem.fileName.c_str(), GA_ReadOnly);
    if(poDS == NULL)
    {
        throw std::runtime_error("Could not open DEM for reading.");
    }
    if(GDALHasNoData(poDS, 1))
    {
        throw std::runtime_error("The DEM has no data values.");
    }
    GDALClose((GDALDatasetH)poDS);

    //check for invalid characters in DEM name
    std::string s = std::string(CPLGetBasename(input.dem.fileName.c_str()));
    if(s.find_first_of("/\\:;\"'") != std::string::npos){
        throw std::runtime_error("The DEM name contains an invalid character."
                " The DEM name cannot contain the following characters: / \\ : ; \" '.");
    }

    //Check base inputs needed for run
    if( input.dem.prjString == "" && input.googOutFlag == true )
        throw std::logic_error("Projection information in prjString is not set but should be.");
    if( input.dem.prjString == "" && input.wxModelGoogOutFlag == true )
        throw std::logic_error("Projection information in prjString is not set but should be.");
    if( input.initializationMethod == WindNinjaInputs::noInitializationFlag )
        throw std::logic_error("No initialization method has been set.");
    if( input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag )	//single domain-averaged input speed and direction
    {	if( input.inputSpeed < 0.0 )
            throw std::out_of_range("The input speed should be greater than 0.");
        if( input.inputDirection < 0.0 || input.inputDirection > 360.0 )
            throw std::out_of_range("The input direction is less than 0 or greater than 360.");
        if( input.inputWindHeight < 0.0 )
            throw std::out_of_range("The input wind height should be greater than 0.");
    }
    if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag )	//Weather forecast model initialization
    {
        if( input.forecastFilename == "!set" )
            throw std::logic_error("The surface field initialization filename is not set.");
        if( input.ninjaTime.is_not_a_date_time() )
            throw std::runtime_error("The time is not set correctly.");
    }
    if( input.initializationMethod == WindNinjaInputs::pointInitializationFlag )	//point initialization
    {
        if( input.wxStationFilename == "!set" )
            throw std::logic_error("The weather station filename is not set.");
    }

    if( input.outputWindHeight < 0.0 )
        throw std::out_of_range("The output wind height should be greater than 0.");

    if( input.diurnalWinds == true )
    {
        if( input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag )	//single domain-averaged input speed and direction
        {
            if( input.airTemp < -1000 )
                throw std::out_of_range("The air temperature has not been set or "
                                        "was set incorrectly.");
            if( input.cloudCover < 0.0 )
                throw std::out_of_range("The percent cloud cover cannot be less than 0.");
        }
        if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag )	//Weather forecast model initialization
        {
            if( input.forecastFilename == "!set" )
                throw std::logic_error("The surface field initialization filename is not set.");
        }
        if( input.latitude < -1000.0 )
            throw std::out_of_range("The latitude has not been set or was set improperly.");
        if( input.longitude < -1000.0 )
            throw std::out_of_range("The longitude has not been set or was set improperly.");
        if( input.ninjaTime.is_not_a_date_time() )
            throw std::runtime_error("The time is not set correctly.");
    }
    if( input.numberCPUs <= 0 )
        throw std::out_of_range("The number of CPUs for the simulation cannot be less "
                                "than or equal to 0.");
    if( input.outputBufferClipping < 0.0 || input.outputBufferClipping >= 50.0 )
    {
        std::ostringstream buff_str;
        buff_str << "The output file buffer clipping is improperly set to "
                 << input.outputBufferClipping << ".  It should be 0-50 percent.";
        throw std::out_of_range(buff_str.str().c_str());
    }
}

/**
 * Function used to dump as much memory as possible AFTER a run is complete.
 * After running this function, you SHOULD NOT try to do another simulation
 * with this object unless you reset/redo all members.
 */
void ninja::dumpMemory()
{
    input.dem.deallocate();
    input.surface.deallocate();
}
