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

/**Ninja constructor
 * This is the default ninja constructor.
 */
ninja::ninja()
{
    cancel = false;
    alphaH = 1.0;
    //alphaV = 1.0;
    alpha = 1.0;
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
    DIAG=NULL;
    PHI=NULL;
    RHS=NULL;
    SK=NULL;
    row_ptr=NULL;
    col_ind=NULL;
    //L=NULL;
    //u_star=NULL;
    //bl_height=NULL;
    uDiurnal=NULL;
    vDiurnal=NULL;
    wDiurnal=NULL;
    height=NULL;
    aspect=NULL;
    slope=NULL;
    shade=NULL;
    solar=NULL;
    speedInitializationGrid=NULL;
    dirInitializationGrid=NULL;
    uInitializationGrid=NULL;
    vInitializationGrid=NULL;
    airTempGrid=NULL;
    cloudCoverGrid=NULL;
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
, u(rhs.u)
, v(rhs.v)
, w(rhs.w)
, u0(rhs.u0)
, v0(rhs.v0)
, w0(rhs.w0)
, mesh(rhs.mesh)
, L(rhs.L)
, u_star(rhs.u_star)
, bl_height(rhs.bl_height)
, input(rhs.input)
{
    input.Com = NULL;   //must be set to null!
    set_ninjaCommunication(rhs.get_inputsRunNumber(), rhs.get_inputsComType());
    strcpy( input.lastComString, rhs.get_lastComString() );
    input.Com->fpLog = rhs.get_ComLogFp();


    cancel = rhs.cancel;
    alphaH = rhs.alphaH;
    //alphaV = rhs.alphaV;
    alpha = rhs.alpha;
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
    DIAG=NULL;
    PHI=NULL;
    RHS=NULL;
    SK=NULL;
    row_ptr=NULL;
    col_ind=NULL;
    uDiurnal=NULL;
    vDiurnal=NULL;
    wDiurnal=NULL;
    height=NULL;
    aspect=NULL;
    slope=NULL;
    shade=NULL;
    solar=NULL;
    speedInitializationGrid=NULL;
    dirInitializationGrid=NULL;
    uInitializationGrid=NULL;
    vInitializationGrid=NULL;
    airTempGrid=NULL;
    cloudCoverGrid=NULL;
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
        u = rhs.u;
        v = rhs.v;
        w = rhs.w;
        u0 = rhs.u0;
        v0 = rhs.v0;
        w0 = rhs.w0;
        L = rhs.L;
        u_star = rhs.u_star;
        bl_height = rhs.bl_height;

        mesh = rhs.mesh;
        input = rhs.input;

        cancel = rhs.cancel;
        alphaH = rhs.alphaH;
        //alphaV = rhs.alphaV;
        alpha = rhs.alpha;
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
        DIAG=NULL;
        PHI=NULL;
        RHS=NULL;
        SK=NULL;
        row_ptr=NULL;
        col_ind=NULL;
        uDiurnal=NULL;
        vDiurnal=NULL;
        wDiurnal=NULL;
        height=NULL;
        aspect=NULL;
        slope=NULL;
        shade=NULL;
        solar=NULL;
        speedInitializationGrid=NULL;
        dirInitializationGrid=NULL;
        uInitializationGrid=NULL;
        vInitializationGrid=NULL;
        airTempGrid=NULL;
        cloudCoverGrid=NULL;
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
	input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);
	#endif

	#ifdef _OPENMP
		startTotal = omp_get_wtime();
	#endif

	#ifdef NINJA_DEBUG
		#ifdef MKL
		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "MKL computational kernals used...");
		#else
		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "MKL computational kernals not available...");
		#endif //MKL
	#endif	//NINJA_DEBUG

	 //taucs_double *SK;

#ifdef SCALAR
    //check if source is within WN mesh extents and transform coords
    if(input.scalarTransportFlag == true){

        //Transform coords from lat/long to DEM space and WN mesh space

        OGRSpatialReference oSRS, *poLatLong;
        char* dstWKT = (char*)input.dem.prjString.c_str();
        oSRS.importFromWkt(&dstWKT);

        OGRCoordinateTransformation *poCT;
        poLatLong = oSRS.CloneGeogCS();

        bool transformed;
        double xCoord, yCoord;
        xCoord = input.scalarSourceXcoord;
        yCoord = input.scalarSourceYcoord;

        poCT = OGRCreateCoordinateTransformation(poLatLong, &oSRS);

        if(poCT==NULL || !poCT->Transform(1, &xCoord, &yCoord))
            throw std::runtime_error("Coordinate transformation failed when converting scalar source coordinates.\n");

        scalarSourceXdem = xCoord;
        scalarSourceYdem = yCoord;

        //cout<<"scalar source x, y in dem coords = "<<xCoord<<", "<<yCoord<<endl;

        if(!input.dem.check_inBounds(xCoord, yCoord)){
            throw std::runtime_error("Scalar source is located outside of the DEM extent.");
        }

        scalarSourceXORD = xCoord - input.dem.xllCorner + 0.5 * input.dem.cellSize; //double check these conversions
        scalarSourceYORD = yCoord - input.dem.yllCorner + 0.5 * input.dem.cellSize; //double check these conversions

        cout<<"scalar source x, y in WN coords = "<<scalarSourceXORD<<", "<<scalarSourceYORD<<endl;

    }
#endif //SCALAR

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
    int max_matching_iters = nMaxMatchingIters;		//maximum number of outer iterations to do (for matching observations)

/*  ----------------------------------------*/
/*  MESH GENERATION                         */
/*  ----------------------------------------*/

	#ifdef _OPENMP
		startMesh = omp_get_wtime();
	#endif

	input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating mesh...");
	//generate mesh
	mesh.buildStandardMesh(input);
	
	u0.allocate(&mesh);		//u is positive toward East
	v0.allocate(&mesh);		//v is positive toward North
	w0.allocate(&mesh);		//w is positive up

	#ifdef _OPENMP
		endMesh = omp_get_wtime();
	#endif

/*  ----------------------------------------*/
/*  START OUTER INTERATIVE LOOP FOR         */
/*	MATCHING INPUT POINTS					*/
/*  ----------------------------------------*/

if(input.initializationMethod == WindNinjaInputs::pointInitializationFlag)
{
	if(input.matchWxStations == true)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaNone, "Starting outer wx station \"matching\" loop...");
		input.Com->noSolverProgress();    //don't print normal solver progress, just "outer iter" "matching" progress
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
do
{
/*  ----------------------------------------*/
/*  VELOCITY INITIALIZATION                 */
/*  ----------------------------------------*/

		if(input.matchWxStations == true)
		{
			matchingIterCount++;
			input.Com->ninjaCom(ninjaComClass::ninjaNone, "\"matching\" loop iteration %i...", matchingIterCount);
		}

		#ifdef _OPENMP
			startInit = omp_get_wtime();
		#endif

		input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initializing flow...");

		//initialize
		if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
		{
			//wxModelInitialization* init;
		    //wxInit = wxModelInitializationFactory::makeWxInitialization(input.forecastFilename);

		    wxInit.reset(wxModelInitializationFactory::makeWxInitialization(input.forecastFilename));
                    wxInit->initializeFields(input, mesh, u0, v0, w0, CloudGrid, L, u_star, bl_height);

		}else if(input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag)
		{
			domainAverageInitialization init;
			init.initializeFields(input, mesh, u0, v0, w0, CloudGrid, L, u_star, bl_height);

		}else if(input.initializationMethod == WindNinjaInputs::pointInitializationFlag)
		{
			pointInitialization init;
			init.initializeFields(input, mesh, u0, v0, w0, CloudGrid, L, u_star, bl_height);

		}else{
			 throw std::logic_error("Incorrect wind initialization.");
		}

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
		discretize(false);

        checkCancel();

/*  ----------------------------------------*/
/*  SET BOUNDARY CONDITIONS                 */
/*  ----------------------------------------*/

		//set boundary conditions
		setBoundaryConditions();

		//#define WRITE_A_B
		#ifdef WRITE_A_B	//used for debugging...
			 write_A_and_b(1000, SK, col_ind, row_ptr, RHS);
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

		//jac_precond_CG_MKL(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol);
		//jac_precond_CG(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol);

		//BELOW IS GOOD SOLVER
		if(solve(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
			throw std::runtime_error("Solver returned false.");

		//if(taucs_solve(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
		//{
		//	input.Com->ninjaCom(ninjaComClass::ninjaFailure, "ERROR!!!  PROBLEM IN SOLVER, RETURN OF FALSE!");
		//	return false;
		//}


		#ifdef _OPENMP
			endSolve = omp_get_wtime();
		#endif

		checkCancel();


		 #ifdef MKL
		 MKL_free(SK);
		 #else
		 if(SK)
		 {
			delete[] SK;
			SK=NULL;
		 }
		 #endif

		 if(col_ind)
		 {
			delete[] col_ind;
			col_ind=NULL;
		 }
		 if(row_ptr)
		 {
			delete[] row_ptr;
			row_ptr=NULL;
		 }
		 if(RHS)
		 {
			delete[] RHS;
			RHS=NULL;
		 }

/*  ----------------------------------------*/
/*  COMPUTE UVW WIND FIELD                   */
/*  ----------------------------------------*/

		 //compute uvw field from phi field
		 computeUVWField();

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
                mesh.meshResolution, smallestInfluenceRadius, smallestInfluenceRadius/mesh.meshResolution, 10.0*mesh.meshResolution);

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
	writeOutputFiles(false);

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
//END OF simulate_wind()-------------------------------------------------------


#ifdef SCALAR
/**@brief Method to start a scalar transport simulation.
 *
 * simulate_wind() must be run before this function is called.
 *
 * @return Returns true if simulation completes without error.
 */
bool ninja::simulate_scalar()
{
    checkCancel();

    scalarTransportSimulation = true;
    concentration.allocate(&mesh);

    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            for(int k = 0; k < mesh.nlayers; k++){
                concentration(i,j,k) = 0.0;
            }
        }
    }
    //cout<<"concentration(0,0,0) = "<<concentration(0,0,0)<<endl;
    //cout<<"mesh.XORD, mesh.YORD, mesh.ZORD"<<mesh.XORD(0,0,0)<<", "<<mesh.YORD(0,0,0)<<", "<<mesh.ZORD(0,0,0)<<endl;

    #ifdef _OPENMP
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Scalar Transport number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);
    #endif

    #ifdef _OPENMP
    startTotal = omp_get_wtime();
    #endif

    #ifdef NINJA_DEBUG
        #ifdef MKL
        input.Com->ninjaCom(ninjaComClass::ninjaDebug, "MKL computational kernals used...");
        #else
        input.Com->ninjaCom(ninjaComClass::ninjaDebug, "MKL computational kernals not available...");
        #endif //MKL
    #endif  //NINJA_DEBUG

/*  ----------------------------------------*/
/*  USER INPUTS                             */
/*  ----------------------------------------*/
     int MAXITS = 100000;             //MAXITS is the maximum number of iterations in the solver
     //double stop_tol = 1E-1;          //stopping criteria for iterations (2-norm of residual)
     //double stop_tol = 2.7E-1;
     double stop_tol = 0.16;  //works for minres with diffusion...
     //double stop_tol = 0.1;  //works for BiCG with diffusion...
     //double stop_tol = 0.4;  //testing
     int print_iters = 10;          //Iterations to print out
         //int max_matching_iters = 30;         //maximum number of outer iterations to do (for matching observations)


/*  ----------------------------------------*/
/*  BUILD "A" ARRAY OF AX=B                 */
/*  ----------------------------------------*/
                #ifdef _OPENMP
                        startBuildEq = omp_get_wtime();
                #endif

                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Building scalar transport equations...");

                //build A arrray
                discretize(true);

        checkCancel();

/*  ----------------------------------------*/
/*  SET BOUNDARY CONDITIONS                 */
/*  ----------------------------------------*/

                //set boundary conditions
                setBoundaryConditions();

                #define WRITE_A_B
                #ifdef WRITE_A_B        //used for debugging...
                         write_A_and_b(1000, SK, col_ind, row_ptr, RHS);
                #endif

                #ifdef _OPENMP
                        endBuildEq = omp_get_wtime();
                #endif

                 checkCancel();

/*  ----------------------------------------*/
/*  CALL SOLVER                             */
/*  ----------------------------------------*/

                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solving scalar transport equations...");
                #ifdef _OPENMP
                        startSolve = omp_get_wtime();
                #endif

                //solver

                //BELOW IS GOOD SOLVER
                //if(solve(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
                        //throw std::runtime_error("Solver returned false.");
        //use MINRES solver for full matrix (works for diffusion only; also works for full matrix wind flow)
                if(solveMinres(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
                        throw std::runtime_error("Solver returned false.");
        //use BiCGSTAB solver since SK in asymmetric for scalar transport
                //if(solveBiCGSTAB(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
                        //throw std::runtime_error("Solver returned false.");
        //if(solveBiCG(SK, RHS, PHI, row_ptr, col_ind, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
                        //throw std::runtime_error("Solver returned false.");


                #ifdef _OPENMP
                        endSolve = omp_get_wtime();
                #endif

                checkCancel();


                 #ifdef MKL
                 MKL_free(SK);
                 #else
                 if(SK)
                 {
                        delete[] SK;
                        SK=NULL;
                 }
                 #endif

                 if(col_ind)
                 {
                        delete[] col_ind;
                        col_ind=NULL;
                 }
                 if(row_ptr)
                 {
                        delete[] row_ptr;
                        row_ptr=NULL;
                 }
                 if(RHS)
                 {
                        delete[] RHS;
                        RHS=NULL;
                 }
/*  ----------------------------------------*/
/*  COMPUTE SCALAR VOLUME FIELD              */
/*  ----------------------------------------*/

                 //fill in concentration 3d field from phi field

                 computeScalarField();

                 /*checkCancel();


/*  ----------------------------------------*/
/*  PREPARE OUTPUT                          */
/*  ----------------------------------------*/

                 /*#ifdef _OPENMP
                        startWriteOut = omp_get_wtime();
                 #endif

                 //prepare output arrays
                 prepareOutput();

                 checkCancel();*/

/*  ----------------------------------------*/
/*  WRITE OUTPUT FILES                      */
/*  ----------------------------------------*/

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing scalar transport output files...");

        //write output files
        writeOutputFiles(true);

        #ifdef _OPENMP
                endWriteOut = omp_get_wtime();
                endTotal = omp_get_wtime();
        #endif
/*  ----------------------------------------*/
/*  WRAP UP...                              */
/*  ----------------------------------------*/

        //write timers
        #ifdef _OPENMP
                        //input.Com->ninjaCom(ninjaComClass::ninjaNone, "Meshing time was %lf seconds.",endMesh-startMesh);
                        //input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initialization time was %lf seconds.",endInit-startInit);
                        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Equation building time was %lf seconds.",endBuildEq-startBuildEq);
                        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solver time was %lf seconds.",endSolve-startSolve);
                        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Output writing time was %lf seconds.",endWriteOut-startWriteOut);
                        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Total simulation time was %lf seconds.",endTotal-startTotal);
        #endif


     input.Com->ninjaCom(ninjaComClass::ninjaNone, "Scalar transport run number %d done!", input.inputsRunNumber);

         /*deleteDynamicMemory();
         if(!input.keepOutGridsInMemory)
         {
             AngleGrid.deallocate();
             VelocityGrid.deallocate();
             CloudGrid.deallocate();
         DustGrid.deallocate();
         }*/

     return true;
} //END OF simulate_scalar()-------------------------------------------------------
#endif //SCALAR



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

/*
void ninja::jac_precond_CG(double *SK, double *RHS, double *PHI, int *row_ptr, int *col_ind, int NUMNP, int MAXITS, int print_iters, double stop_tol)
{
     //This is the Jacobi Preconditioned Conjugate Gradient solver (from Dr. Thompson)
     int i, j, k;
     double *r, *p, *D, A_infin_norm, A_infin_norm2, x_infin_norm, r_infin_norm, b_infin_norm, beta, *Ax, res_check;
     double rAp=0, *Ap, *Anorm, pAp=0., rp=0., s, r_2_norm, residual_percent_complete, time_percent_complete, iter1_residual;

 #ifdef NINJA_DEBUG_VERBOSE
      if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
      {
           input.Com->ninjaCom(ninjaComClass::ninjaWarning, "A convergence_history file to write to cannot be created.\nIt may be in use by another program.");
           exit(0);
      }
      fprintf(convergence_history,"\nIteration\tr_2_norm\tResidual\tres_check");
#endif //NINJA_DEBUG_VERBOSE
     r=new double[NUMNP];
     p=new double[NUMNP];
     D=new double[NUMNP];
     Ap=new double[NUMNP];
     Anorm=new double[NUMNP];
	 Ax=new double[NUMNP];

	 //stuff for sparse BLAS MV multiplication
	 char transa='n';
	 double one=1.E0, zero=0.E0;
	 char matdescra[6];
	 matdescra[0]='s';
	 matdescra[1]='u';
	 matdescra[2]='n';
	 matdescra[3]='c';


     b_infin_norm=0.;
     A_infin_norm=0.;
     A_infin_norm2=0.;

	#pragma omp parallel default(shared) private(i,j)
	 {
	 #pragma omp for
	 for(i=0;i<NUMNP;i++)
     {
          for(j=row_ptr[i];j<row_ptr[i+1];j++)
          {
               if(col_ind[j]==i)   //Check if we're on the diagonal
                    D[i]=1.0/std::sqrt(SK[j]);    //Store the diagonal term
          }
          RHS[i]=D[i]*RHS[i];     //condition the RHS also
     }
	 #pragma omp single
	 {
	 for(i=0;i<NUMNP;i++)
     {
          if(fabs(RHS[i])>b_infin_norm)
               b_infin_norm=fabs(RHS[i]);     //compute the infinity norm of b
     }
	 }	//end single region


     #pragma omp for
     for(i=0;i<NUMNP;i++)
     {
          for(j=row_ptr[i];j<row_ptr[i+1];j++)
          {
               SK[j]=D[i]*SK[j]*D[col_ind[j]];    //condition the SK matrix
//               if(fabs(SK[j])>A_infin_norm)
//                    A_infin_norm=fabs(SK[j]);     //Compute the infinity norm of A (incorrect, should add up whole column)
          }
     }


////////////////////////////////////////////
     #pragma omp for
     for(i=0;i<NUMNP;i++)
          Anorm[i]=0.;
     #pragma omp single
	 {
	 for(i=0;i<NUMNP;i++)
     {
          for(j=row_ptr[i];j<row_ptr[i+1];j++)
          {
//               Anorm[col_ind[j]]=Anorm[col_ind[j]]+SK[j];
               Anorm[i] += SK[j];
			   if(col_ind[j] == i)	//if this is the diagonal, skip next computation
				   continue;
			   Anorm[col_ind[j]] += SK[j];	//since only the upper triangle of A is stored, account for this...
          }
     }
	 for(i=0;i<NUMNP;i++)
	 {
		  if(fabs(Anorm[i])>A_infin_norm2)
			   A_infin_norm2=fabs(Anorm[i]); //compute infinity norm of A correctly...
	 }
	 }	//end single region

////////////////////////////////////////////

	}	//end parallel region


	 //for(i=0;i<NUMNP;i++)
		//  Ax[i] = 0.;

	 //for(i=0;i<NUMNP;i++)
  //   {
  //        for(j=row_ptr[i];j<row_ptr[i+1];j++)
  //        {
  //             Ax[i] += SK[j]*PHI[col_ind[j]];
		//	   if(col_ind[j] == i)	//if this is the diagonal, skip the following computation
		//		   continue;
		//	   Ax[col_ind[j]] += SK[j]*PHI[i];
  //        }
  //
  //   }

	 mkl_dcsrmv(&transa, &NUMNP, &NUMNP, &one, matdescra, SK, col_ind, row_ptr, &row_ptr[1], PHI, &zero, Ax);

	 for(i=0;i<NUMNP;i++)
          r[i]=RHS[i]-Ax[i];                  //calculate the initial residual

     k=0;           //k is the iteration number

     do		//--------------------------begin iteration loops------------------------------------------
     {
		  k++;


		 if(k==1)
          {
			  #pragma omp parallel for default(shared) private(i)
			  for(i=0;i<NUMNP;i++)
                    p[i]=r[i];
          }else
          {
               rAp=0.;
			   #pragma omp parallel for reduction(+:rAp)
			   for(i=0;i<NUMNP;i++)
                    rAp += r[i]*Ap[i];

			   beta=rAp/pAp;

			   #pragma omp parallel for default(shared) private(i)
               for(i=0;i<NUMNP;i++)
                    p[i]=r[i]-beta*p[i];
          }

		  rp=0.;

		  #pragma omp parallel for reduction(+:rp)
          for(i=0;i<NUMNP;i++)
               rp += r[i]*p[i];


		  mkl_dcsrmv(&transa, &NUMNP, &NUMNP, &one, matdescra, SK, col_ind, row_ptr, &row_ptr[1], p, &zero, Ap);
		  //mv_mult(NUMNP, SK, col_ind, row_ptr, row_ptr+1, p, Ap);	//matrix-vector multiplication!!!!!

     //     for(i=0;i<NUMNP;i++)		//matrix-vector multiplication!!!!!
     //     {
     //          for(j=row_ptr[i];j<row_ptr[i+1];j++)
			  // {
     //               Ap[i] += SK[j]*p[col_ind[j]];
					//if(col_ind[j] == i)	//if this is the diagonal, skip the next computation
					//	continue;
					//Ap[col_ind[j]] += SK[j]*p[i];
			  // }
     //     }


		  pAp=0.;
          x_infin_norm=0.;

	  #pragma omp parallel default(shared) private(i,j)
	  {
		  #pragma omp for reduction(+:pAp)
          for(i=0;i<NUMNP;i++)
               pAp += p[i]*Ap[i];

		  #pragma omp single
		  {
				s=rp/pAp;
		  }

		  #pragma omp for
          for(i=0;i<NUMNP;i++)
          {
               PHI[i] += s*p[i];
          }

		  #pragma omp single
		  {
		  for(i=0;i<NUMNP;i++)
		  {
			   if(fabs(PHI[i])>x_infin_norm)
					x_infin_norm=fabs(PHI[i]);
		  }
		  }

          #pragma omp for
          for(i=0;i<NUMNP;i++)
          {
               r[i]=r[i]-s*Ap[i];
          }

	  }	//end parallel section

		   r_infin_norm=0.;

		   for(i=0;i<NUMNP;i++)
		   {
				if(fabs(r[i])>r_infin_norm)
					r_infin_norm=fabs(r[i]);
		   }

		   if(k<(print_iters-1))
				iter1_residual=r_infin_norm;       //save the residual value for the first iteration to use later for % done computations

		   res_check=stop_tol*(A_infin_norm*x_infin_norm+b_infin_norm);

		   if((k%print_iters)==0)
		   {
			   #ifdef NINJA_DEBUG_VERBOSE
				input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %ld\tr_2_norm=%lf\tResidual = %lf\tres_check=%lf",k,r_2_norm,r_infin_norm,res_check);
				fprintf(convergence_history,"\n%ld\t%lf\t%lf\t%lf",k,r_2_norm,r_infin_norm,res_check);
			   #endif //NINJA_DEBUG_VERBOSE

				residual_percent_complete=100-100*((r_infin_norm-res_check)/(iter1_residual-res_check));
				time_percent_complete=0.0000034095*std::pow(residual_percent_complete,4) - 0.0004207753*std::pow(residual_percent_complete,3) + 0.0119452111*std::pow(residual_percent_complete,2) + 0.5394600870*residual_percent_complete;
				if(time_percent_complete<0)
					 time_percent_complete=0;
				//printf("\n%.0lf%% complete...",residual_percent_complete);
				input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete-0.5));


//               input.Com->ninjaCom(ninjaComClass::ninjaDebug, "PHI=%lf\tRHS=%lf",PHI[0*ncols*nrows+20*ncols+20],RHS[0*ncols*nrows+20*ncols+20]);
//               input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration %ld\tx_infin_=%lf\tr_infin=%lf\tratio=%lf\tx_2=%lf\tb_2=%lf\tr_2=%lf",k,x_infin_norm,r_infin_norm,rbratio,x_2_norm,b_2_norm,r_2_norm);
		   }
		   //if(r_infin_norm<=res_check)
		   //{
			//   input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done.");
			//   break;
		   //}

		   if((k==MAXITS)&&(r_infin_norm>res_check))
			   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Solution did not converge.\nMAXITS reached.");


		   if(k==NUMNP)40
			   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Number of iterations equals the number of unknowns.\nIterations have been stopped...");


//          r_2_norm=0;		//jason commented out 10.9.2008
//          for(i=0;i<NUMNP;i++)
//               r_2_norm=r_2_norm+r[i]*r[i];
//          r_2_norm=std::sqrt(r_2_norm);


//          input.Com->ninjaCom(ninjaComClass::ninjaDebug, "A=%lf\tx=%lf\tb=%lf");




                         //Check convergence here
     }while(k<MAXITS && k!=NUMNP && r_infin_norm>=res_check);



	//if(r_infin_norm<=res_check)
	//{
	//	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done.");
	//}


     //Uncondition PHI
	 #pragma omp parallel for private(i)
     for(i=0;i<NUMNP;i++)
     {
          PHI[i]=PHI[i]*D[i];
//          fprintf(testt,"%ld\t%lf\n",i,PHI[i]);
     }
#ifdef NINJA_DEBUG_VERBOSE
	 fclose(convergence_history);
#endif //NINJA_DEBUG_VERBOSE

     delete[] r;
     delete[] p;
     delete[] D;
     delete[] Ap;
	 delete[] Anorm;
	 delete[] Ax;

}*/

/*
void ninja::jac_precond_CG_MKL(double *SK, double *RHS, double *PHI, int *row_ptr, int *col_ind, int NUMNP, int MAXITS, int print_iters, double stop_tol)
{
     //This is the Jacobi Preconditioned Conjugate Gradient solver (from Dr. Thompson)
     int i, j, k;
     double *r, *p, *D, A_infin_norm, A_infin_norm2, x_infin_norm, r_infin_norm, b_infin_norm, beta, *Ax, res_check;
     double rAp=0, *Ap, *Anorm, pAp=0., rp=0., s, r_2_norm, residual_percent_complete, time_percent_complete, iter1_residual;

#ifdef NINJA_DEBUG_VERBOSE
      if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
      {
           input.Com->ninjaCom(ninjaComClass::ninjaFailure, "A convergence_history file to write to cannot be created.\nIt may be in use by another program.");
      }
      fprintf(convergence_history,"\nIteration\tr_2_norm\tResidual\tres_check");
#endif //NINJA_DEBUG_VERBOSE

     r=new double[NUMNP];
     p=new double[NUMNP];
     D=new double[NUMNP];
     Ap=new double[NUMNP];
     Anorm=new double[NUMNP];
	 Ax=new double[NUMNP];

//     for(i=0;i<NUMNP;i++)
//          input.Com->ninjaCom(ninjaComClass::ninjaDebug, "i=%ld\tRHS=%lf",i,RHS[i]);

	 //stuff for sparse BLAS MV multiplication
	 char transa='n';
	 double one=1.E0, zero=0.E0;
	 char matdescra[6];
	 matdescra[0]='s';
	 matdescra[1]='u';
	 matdescra[2]='n';
	 matdescra[3]='c';


     b_infin_norm=0.;
     A_infin_norm=0.;
     A_infin_norm2=0.;

	 #pragma omp parallel for
	 for(i=0;i<NUMNP;i++)
     {
          D[i]=1.0/std::sqrt(SK[row_ptr[i]]);    //Store the diagonal term
          RHS[i]=D[i]*RHS[i];     //condition the RHS also
     }

	 j = cblas_idamax(NUMNP,RHS,1);	//find index of largest absolute value element
	 b_infin_norm = fabs(RHS[j]);	//set to largest value

     #pragma omp parallel for private(i,j)
     for(i=0;i<NUMNP;i++)
     {
          for(j=row_ptr[i];j<row_ptr[i+1];j++)
               SK[j]=D[i]*SK[j]*D[col_ind[j]];    //condition the SK matrix
     }

     #pragma omp parallel for
     for(i=0;i<NUMNP;i++)
          Anorm[i]=0.;
	 for(i=0;i<NUMNP;i++)
     {
          for(j=row_ptr[i];j<row_ptr[i+1];j++)
          {
               Anorm[i] += SK[j];
			   if(col_ind[j] == i)	//if this is the diagonal, skip next computation
				   continue;
			   Anorm[col_ind[j]] += SK[j];	//since only the upper triangle of A is stored, account for this...
          }
     }
	 j = cblas_idamax(NUMNP,Anorm,1);	//find index of largest absolute value element
	 A_infin_norm2 = fabs(Anorm[j]);	//set to largest value

	 //matrix multiplication
	 mkl_dcsrmv(&transa, &NUMNP, &NUMNP, &one, matdescra, SK, col_ind, row_ptr, &row_ptr[1], PHI, &zero, Ax);

	 for(i=0;i<NUMNP;i++)
          r[i]=RHS[i]-Ax[i];                  //calculate the initial residual

     k=0;           //k is the iteration number

     do		//--------------------------begin iteration loops------------------------------------------
     {
		  k++;


		  if(k==1)
          {
			  cblas_dcopy(NUMNP, r, 1, p, 1);	//copy r into p
          }else
          {

			   rAp = cblas_ddot(NUMNP, r, 1, Ap, 1);	//compute dot product
			   beta=rAp/pAp;

			   //cblas_dscal(NUMNP, -beta, p, 1);			//multiply (-beta)*p
			   //cblas_daxpy(NUMNP, 1., r, 1, p, 1);		//add p and r
			   #pragma omp parallel for default(shared) private(i)
               for(i=0;i<NUMNP;i++)
                    p[i]=r[i]-beta*p[i];
          }

		  rp = cblas_ddot(NUMNP, r, 1, p, 1);	//dot product of r and p

		  //matrix-vector multiplication!!!!!
		  mkl_dcsrmv(&transa, &NUMNP, &NUMNP, &one, matdescra, SK, col_ind, row_ptr, &row_ptr[1], p, &zero, Ap);

		  pAp = cblas_ddot(NUMNP, p, 1, Ap, 1);

		  s=rp/pAp;

		  cblas_daxpy(NUMNP, s, p, 1, PHI, 1);	//PHI[] = PHI[] + s*p[]

		  x_infin_norm=0.;
		  for(i=0;i<NUMNP;i++)
		  {
			   if(fabs(PHI[i])>x_infin_norm)
					x_infin_norm=fabs(PHI[i]);
		  }

          cblas_daxpy(NUMNP, -s, Ap, 1, r, 1);

		   r_infin_norm=0.;
		   for(i=0;i<NUMNP;i++)
		   {
				if(fabs(r[i])>r_infin_norm)
					r_infin_norm=fabs(r[i]);
		   }

		   if(k<(print_iters-1))
				iter1_residual=r_infin_norm;       //save the residual value for the first iteration to use later for % done computations

		   res_check=stop_tol*(A_infin_norm*x_infin_norm+b_infin_norm);

		   if((k%print_iters)==0)
		   {
			   #ifdef NINJA_DEBUG_VERBOSE
				input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %ld\tr_2_norm=%lf\tResidual = %lf\tres_check=%lf",k,r_2_norm,r_infin_norm,res_check);
				fprintf(convergence_history,"\n%ld\t%lf\t%lf\t%lf",k,r_2_norm,r_infin_norm,res_check);
			   #endif //NINJA_DEBUG_VERBOSE

				residual_percent_complete=100-100*((r_infin_norm-res_check)/(iter1_residual-res_check));
				time_percent_complete=0.0000034095*std::pow(residual_percent_complete,4) - 0.0004207753*std::pow(residual_percent_complete,3) + 0.0119452111*std::pow(residual_percent_complete,2) + 0.5394600870*residual_percent_complete;
				if(time_percent_complete<0)
					 time_percent_complete=0;
				//printf("\n%.0lf%% complete...",residual_percent_complete);
				input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));

			}

		   if((k==MAXITS)&&(r_infin_norm>res_check))
			   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Solution did not converge.\nMAXITS reached.");


		   if(k==NUMNP)
			   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Number of iterations equals the number of unknowns.\nIterations have been stopped...");

                         //Check convergence here
     }while(k<MAXITS && k!=NUMNP && r_infin_norm>=res_check);



	//if(r_infin_norm<=res_check)
	//{
	//	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done.");
	//}


     //Uncondition PHI
	 #pragma omp parallel for private(i)
     for(i=0;i<NUMNP;i++)
     {
          PHI[i]=PHI[i]*D[i];
//          fprintf(testt,"%ld\t%lf\n",i,PHI[i]);
     }

#ifdef NINJA_DEBUG_VERBOSE
	 fclose(convergence_history);
#endif //NINJA_DEBUG_VERBOSE

     delete[] r;
     delete[] p;
     delete[] D;
     delete[] Ap;
	 delete[] Anorm;
	 delete[] Ax;

}*/

//  CG solver
//    This solver is fastest, but is not monotonic convergence (residual oscillates a bit up and down)
//    If this solver diverges, try MINRES from PetSc below...
/**Method called in ninja::simulate_wind() to solve the matrix equations.
 * This is a congugate gradient solver.
 * It seems to be the fastest, but is not monotonic convergence (residual oscillates a bit up and down).
 * If this solver diverges, try the MINRES from PetSc which is commented out below...
 * @param A Stiffness matrix in Ax=b matrix equation.  Storage is symmetric compressed sparse row storage.
 * @param b Right hand side of matrix equations.
 * @param x Vector to store solution in.
 * @param row_ptr Vector used to index to a row in A.
 * @param col_ind Vector storing the column number of corresponding value in A.
 * @param NUMNP Number of nodal points, so also the size of b, x, and row_ptr.
 * @param max_iter Maximum number of iterations to do.
 * @param print_iters How often to print out solver information.
 * @param tol Convergence tolerance to stop at.
 * @return Returns true if solver converges and completes properly.
 */
bool ninja::solve(double *A, double *b, double *x, int *row_ptr, int *col_ind, int NUMNP, int max_iter, int print_iters, double tol)
{
    //stuff for sparse BLAS MV multiplication
    char transa='n';
    double one=1.E0, zero=0.E0;
    char matdescra[6];
    matdescra[0]='s';	//symmetric
    matdescra[1]='u';	//upper triangle stored
    matdescra[2]='n';	//non-unit diagonal
    matdescra[3]='c';	//c-style array (ie 0 is index of first element, not 1 like in Fortran)

    FILE *convergence_history;
    int i, j;
    double *p, *z, *q, *r;
    double alpha, beta, rho, rho_1, normb, resid;
    double residual_percent_complete, residual_percent_complete_old, time_percent_complete, start_resid;
    residual_percent_complete = 0.0;

    residual_percent_complete_old = -1.;

    Preconditioner M;
    if(M.initialize(NUMNP, A, row_ptr, col_ind, M.SSOR, matdescra)==false)
    {
        input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Initialization of SSOR preconditioner failed, trying Jacobi preconditioner...");
        if(M.initialize(NUMNP, A, row_ptr, col_ind, M.Jacobi, matdescra)==false)
            throw std::runtime_error("Initialization of Jacobi preconditioner failed.");
    }

//#define NINJA_DEBUG_VERBOSE
#ifdef NINJA_DEBUG_VERBOSE
    if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
        throw std::runtime_error("A convergence_history file to write to cannot be created.\nIt may be in use by another program.");

    fprintf(convergence_history,"\nIteration\tResidual\tResidual_check");
#endif //NINJA_DEBUG_VERBOSE

    p=new double[NUMNP];
    z=new double[NUMNP];
    q=new double[NUMNP];
    r=new double[NUMNP];
    //Ax=new double[NUMNP];
    //Ap=new double[NUMNP];
    //Anorm=new double[NUMNP];



    //matrix vector multiplication A*x=Ax
    mkl_dcsrmv(&transa, &NUMNP, &NUMNP, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], x, &zero, r);

    for(i=0;i<NUMNP;i++){
        r[i]=b[i]-r[i];                  //calculate the initial residual
    }

    normb = cblas_dnrm2(NUMNP, b, 1);		//calculate the 2-norm of b
    //normb = nrm2(NUMNP, b);

    if (normb == 0.0)
        normb = 1.;

    //compute 2 norm of r
    resid = cblas_dnrm2(NUMNP, r, 1) / normb;
    //resid = nrm2(NUMNP, r) / normb;

    if (resid <= tol)
    {
        tol = resid;
        max_iter = 0;
        return true;
    }

    //start iterating---------------------------------------------------------------------------------------
    for (int i = 1; i <= max_iter; i++)
    {
        checkCancel();

        M.solve(r, z, row_ptr, col_ind);	//apply preconditioner

        rho = cblas_ddot(NUMNP, z, 1, r, 1);
        //rho = dot(NUMNP, z, r);

        if (i == 1)
        {
            cblas_dcopy(NUMNP, z, 1, p, 1);
        }else {
            beta = rho / rho_1;

#pragma omp parallel for
            for(j=0; j<NUMNP; j++)
                p[j] = z[j] + beta*p[j];
        }

        //matrix vector multiplication!!!		q = A*p;
        mkl_dcsrmv(&transa, &NUMNP, &NUMNP, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], p, &zero, q);

        alpha = rho / cblas_ddot(NUMNP, p, 1, q, 1);
        //alpha = rho / dot(NUMNP, p, q);

        cblas_daxpy(NUMNP, alpha, p, 1, x, 1);	//x = x + alpha * p;
        //axpy(NUMNP, alpha, p, x);

        cblas_daxpy(NUMNP, -alpha, q, 1, r, 1);	//r = r - alpha * q;
        //axpy(NUMNP, -alpha, q, r);

        resid = cblas_dnrm2(NUMNP, r, 1) / normb;	//compute resid
        //resid = nrm2(NUMNP, r) / normb;

        if(i==1)
            start_resid = resid;

        if((i%print_iters)==0)
        {

#ifdef NINJA_DEBUG_VERBOSE
            input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %d\tResidual = %lf\ttol = %lf", i, resid, tol);
            fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,resid,tol);
#endif //NINJA_DEBUG_VERBOSE

            residual_percent_complete=100-100*((resid-tol)/(start_resid-tol));
            if(residual_percent_complete<residual_percent_complete_old)
                residual_percent_complete=residual_percent_complete_old;
            if(residual_percent_complete<0.)
                residual_percent_complete=0.;
            else if(residual_percent_complete>100.)
                residual_percent_complete=100.0;

            time_percent_complete=1.8*exp(0.0401*residual_percent_complete);
            if(time_percent_complete >= 99.0)
                time_percent_complete = 99.0;
            residual_percent_complete_old=residual_percent_complete;
            //fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,residual_percent_complete, time_percent_complete);
            input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));
        }

        if (resid <= tol)	//check residual against tolerance
        {
            break;
        }

        //cout<<"resid = "<<resid<<endl;

        rho_1 = rho;

    }	//end iterations--------------------------------------------------------------------------------------------

    if(p)
    {
        delete[] p;
        p=NULL;
    }
    if(z)
    {
        delete[] z;
        z=NULL;
    }
    if(q)
    {
        delete[] q;
        q=NULL;
    }
    if(r)
    {
        delete[] r;
        r=NULL;
    }

#ifdef NINJA_DEBUG_VERBOSE
    fclose(convergence_history);
#endif //NINJA_DEBUG_VERBOSE

    if(resid>tol)
    {
        throw std::runtime_error("Solution did not converge.\nMAXITS reached.");
    }else{
        time_percent_complete = 100.0;
        input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));
        return true;
    }
}

//  MINRES from PetSc (found in google code search)
//    This solver seems to be monotonic in its convergence (residual always goes down)
//    Could use this if CG diverges, but haven't seen divergence yet...
bool ninja::solveMinres(double *A, double *b, double *x, int *row_ptr, int *col_ind, int NUMNP, int max_iter, int print_iters, double tol)
{

  int i,j, n;
  double alpha,malpha,beta,mbeta,ibeta,betaold,eta,c=1.0,ceta,cold=1.0,coold,s=0.0,sold=0.0,soold;
  double rho0,rho1,irho1,rho2,mrho2,rho3,mrho3,dp = 0.0;
  double rnorm, bnorm;
  double np;	//this is the residual??
  double            *R, *Z, *U, *V, *W, *UOLD, *VOLD, *WOLD, *WOOLD;

  n = NUMNP;

  R = new double[n];
  Z = new double[n];
  U = new double[n];
  V = new double[n];
  W = new double[n];
  UOLD = new double[n];
  VOLD = new double[n];
  WOLD = new double[n];
  WOOLD = new double[n];

  //stuff for sparse BLAS MV multiplication
  char transa='n';
  double one=1.E0, zero=0.E0;
  char matdescra[6];
  //matdescra[0]='s'; //s = symmetric
  matdescra[0]='g'; //g = generic
  matdescra[1]='u';
  matdescra[2]='n';
  matdescra[3]='c';

  FILE *convergence_history;

  if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
  {
	   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "A convergence_history file to write to cannot be created.\nIt may be in use by another program.");
	   exit(0);
  }
  fprintf(convergence_history,"\nIteration\tResidual\tResidual_check");

  Preconditioner precond;

  if(precond.initialize(n, A, row_ptr, col_ind, precond.Jacobi, matdescra)==false)
  {
	  input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Initialization of Jacobi preconditioner failed, trying SSOR preconditioner...");
	  if(precond.initialize(NUMNP, A, row_ptr, col_ind, precond.SSOR, matdescra)==false) // SSOR does not work for full asymmetric matrix !!
	  {
		  input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Initialization of SSOR preconditioner failed, CANNOT SOLVE FOR WINDFLOW!!!");
		  return false;
	  }
  }

  //ksp->its = 0;

  for(j=0;j<n;j++)	UOLD[j] = 0.0;	//  u_old  <-   0
  cblas_dcopy(n, UOLD, 1, VOLD, 1);	//	v_old  <-   0
  cblas_dcopy(n, UOLD, 1, W, 1);	//	w      <-   0
  cblas_dcopy(n, UOLD, 1, WOLD, 1);	//	w_old  <-   0

  mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], x, &zero, R); // r <- b - A*x

  for(j=0;j<n;j++)	R[j] = b[j] - R[j];

  bnorm = cblas_dnrm2(n, b, 1);

  precond.solve(R, Z, row_ptr, col_ind);	//apply preconditioner    M*z = r

  //KSP_PCApply(ksp,R,Z);

  dp = cblas_ddot(n, R, 1, Z, 1);
  if(dp<0.0)
  {
	input.Com->ninjaCom(ninjaComClass::ninjaFailure, "ERROR IN SOLVER!!! SQUARE ROOT OF A NEGATIVE NUMBER...");
	return false;
  }

  dp = std::sqrt(dp);
  beta = dp;		//  beta <- sqrt(r'*z)
  eta = beta;

  //  VecCopy(R,V)   V = R
  cblas_dcopy(n, R, 1, V, 1);
  cblas_dcopy(n, Z, 1, U, 1);
  ibeta = 1.0/beta;
  cblas_dscal(n, ibeta, V, 1);	//  v <- r/beta
  cblas_dscal(n, ibeta, U, 1);	//  u <- z/beta

  np = cblas_dnrm2(n, Z, 1);	//  np <- ||z||

  rnorm = np;

  // TEST FOR CONVERGENCE HERE!!!!

  i = 0;

  do{

	  //Lanczos

	  mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], U, &zero, R); // r <- A*x

	  alpha = cblas_ddot(n, U, 1, R, 1);	//  alpha <- r'*u
	  precond.solve(R, Z, row_ptr, col_ind);	//apply preconditioner    M*z = r

	  malpha = -alpha;
	  cblas_daxpy(n, malpha, V, 1, R, 1);	//  r <- r - alpha*v
	  cblas_daxpy(n, malpha, U, 1, Z, 1);	//  z <- z - alpha*u
	  mbeta = -beta;
	  cblas_daxpy(n, mbeta, VOLD, 1, R, 1);	//  r <- r - beta*v_old
	  cblas_daxpy(n, mbeta, UOLD, 1, Z, 1);	//  z <- z - beta*u_old

	  betaold = beta;

	  dp = cblas_ddot(n, R, 1, Z, 1);

	  beta = std::sqrt(dp);		//  beta <- sqrt(r'*z)

	  coold = cold; cold = c; soold = sold; sold = s;

	  rho0 = cold * alpha - coold * sold * betaold;
      rho1 = std::sqrt(rho0*rho0 + beta*beta);
      rho2 = sold * alpha + coold * cold * betaold;
      rho3 = soold * betaold;

	  //  Givens rotation

	  c = rho0 / rho1;
      s = beta / rho1;

	  //  Update

	  cblas_dcopy(n, WOLD, 1, WOOLD, 1);	//  w_oold <- w_old
	  cblas_dcopy(n, W, 1, WOLD, 1);	//  w_old <- w

	  cblas_dcopy(n, U, 1, W, 1);	//  w <- U
	  mrho2 = -rho2;
	  cblas_daxpy(n, mrho2, WOLD, 1, W, 1);	//  w <- w - rho2 w_old
	  mrho3 = -rho3;
	  cblas_daxpy(n, mrho3, WOOLD, 1, W, 1);	//  w <- w - rho3 w_oold
	  irho1 = 1.0 / rho1;
	  cblas_dscal(n, irho1, W, 1);	//  w <- w/rho1

	  ceta = c * eta;
	  cblas_daxpy(n, ceta, W, 1, x, 1);	//  x <- x - ceta w
	  eta = - s * eta;

	  cblas_dcopy(n, V, 1, VOLD, 1);	//  v_old <- v
	  cblas_dcopy(n, U, 1, UOLD, 1);	//  u_old <- u
	  cblas_dcopy(n, R, 1, V, 1);	//  v <- r
	  cblas_dcopy(n, Z, 1, U, 1);	//  u <- z
	  ibeta = 1.0 / beta;
      cblas_dscal(n, ibeta, V, 1);	//  v <- r / beta
	  cblas_dscal(n, ibeta, U, 1);	//  u <- z / beta

	  np = rnorm * fabs(s);

	  rnorm = np;

	  //  Test for convergence
	  //if(np<tol)	break;

	  //if(rnorm<tol*bnorm)   break;

	  cout<<"residual = "<<np<<endl;

	  if(np<tol)   break;

	  i++;


	  if((i%print_iters)==0)
      {

	    #ifdef NINJA_DEBUG_VERBOSE
		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: n=%d iterations = %d residual norm %12.4e", n,i,rnorm/bnorm);
		//input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %d\tResidual = %lf\ttol = %lf", i, resid, tol);
		fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,rnorm/bnorm,tol);
		#endif //NINJA_DEBUG_VERBOSE

		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: n=%d iterations = %d residual norm %12.4e", n,i,rnorm/bnorm);
		fprintf(convergence_history,"\n%d\t%lf\t%lf",i,rnorm/bnorm,tol);


	    //residual_percent_complete=100-100*((resid-tol)/(start_resid-tol));
		//if(residual_percent_complete<residual_percent_complete_old)
			//residual_percent_complete=residual_percent_complete_old;
		//if(residual_percent_complete<0.)
			 //residual_percent_complete=0.;
		//else if(residual_percent_complete>100.)
			//residual_percent_complete=100.0;

		//time_percent_complete=1.8*exp(0.0401*residual_percent_complete);
		//residual_percent_complete_old=residual_percent_complete;
        //fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,residual_percent_complete, time_percent_complete);
		//input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));
	  }

  }while(i<max_iter);

  if(i >= max_iter)
  {
		input.Com->ninjaCom(ninjaComClass::ninjaFailure, "SOLVER DID NOT CONVERGE WITHIN THE SPECIFIED MAXIMUM NUMBER OF ITERATIONS!!");
		return false;
  }else{
		//input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done.");
		return true;
  }

	fclose(convergence_history);

  if(R)
	delete[] R;
  if(Z)
	delete[] Z;
  if(U)
	delete[] U;
  if(V)
	delete[] V;
  if(W)
	delete[] W;
  if(UOLD)
	delete[] UOLD;
  if(VOLD)
	delete[] VOLD;
  if(WOLD)
	delete[] WOLD;
  if(WOOLD)
	delete[] WOOLD;

  return true;

}


/*
// MINRES from taucs (2 matrix vector products!!!)
bool ninja::solve(double *A, double *b, double *x, int *row_ptr, int *col_ind, int NUMNP, int max_iter, int print_iters, double tol)
{

  double *Xcg, *R, *V, *VV, *Vold, *Volder, *M, *Mold, *Molder;
  double tolb, normr, alpha, beta, beta1, betaold;
  double gamma, gammabar, delta, deltabar, epsilon;
  double cs,sn,snprod, numer, denom;
  int    Iter;
  int    i,n;

  n = NUMNP;

  R      = new double[n];
  Xcg    = new double[n];
  VV     = new double[n];
  V      = new double[n];
  Vold   = new double[n];
  Volder = new double[n];
  M      = new double[n];
  Mold   = new double[n];
  Molder = new double[n];

  //stuff for sparse BLAS MV multiplication
  char transa='n';
  double one=1.E0, zero=0.E0;
  char matdescra[6];
  matdescra[0]='s';
  matdescra[1]='u';
  matdescra[2]='n';
  matdescra[3]='c';

  if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
  {
	   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "A convergence_history file to write to cannot be created.\nIt may be in use by another program.");
	   exit(0);
  }
  fprintf(convergence_history,"\nIteration\tResidual\tResidual_check");

  Preconditioner precond;
  if(precond.initialize(n, A, row_ptr, col_ind, precond.SSOR)==false)
  {
	  input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Initialization of SSOR preconditioner failed, trying Jacobi preconditioner...");
	  if(precond.initialize(NUMNP, A, row_ptr, col_ind, precond.Jacobi)==false)
	  {
		  input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Initialization of Jacobi preconditioner failed, CANNOT SOLVE FOR WINDFLOW!!!");
		  return false;
	  }
  }

  tolb = tol * cblas_dnrm2(n, b, 1);
  input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: residual convergence tolerance %.1e",tolb);

  for (i=0; i<n; i++) x[i] = 0.;    // x = 0
  for (i=0; i<n; i++) R[i] = b[i];  // r = b-A*x

  normr = cblas_dnrm2(n,R,1);
  if ( normr == 0.0 ) {
    input.Com->ninjaCom(ninjaComClass::ninjaFailure, "initial residual == 0");
    return false;
  }

  for (i=0; i<n; i++) V[i]    = R[i];    // v = r
  for (i=0; i<n; i++) Vold[i] = R[i];    // vold = r

  precond.solve(Vold, V, row_ptr, col_ind);	//apply preconditioner   MV = Vold

  beta1 = cblas_ddot(n,Vold,1,V,1);
  if (beta1 < 0.0) {
    input.Com->ninjaCom(ninjaComClass::ninjaFailure, "solver: error (1)");
    return false;
  }
  beta1 = std::sqrt(beta1);

 // { int flag = 0;
 //   for (i=0; i<n; i++) {
 //     if (taucs_isnan(V[i]) && flag < 10)
	//input.Com->ninjaCom(ninjaComClass::ninjaWarning, "V has nan's in position %d",i);
 //     flag++;
 //   }
 // }

  snprod = beta1;

  for (i=0; i<n; i++) VV[i] = V[i] / beta1;

  //matrix vector multiplication A*x=Ax
  mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], VV, &zero, V);   // V = A*VV

  alpha = cblas_ddot(n,VV,1,V,1);

  for (i=0; i<n; i++) V[i] -= (alpha/beta1) * Vold[i];

  // local reorthogonalization

  numer = cblas_ddot(n,VV,1,V,1);
  denom = cblas_ddot(n,VV,1,VV,1);

  for (i=0; i<n; i++) V[i] -= (numer/denom) * VV[i];

  for (i=0; i<n; i++) Volder[i] = Vold[i];
  for (i=0; i<n; i++) Vold[i]   = V[i];

  //(*precond_fn)(precond_args,V,Vold);
  precond.solve(Vold, V, row_ptr, col_ind);	//apply preconditioner   MV = Vold

  betaold = beta1;
  beta = cblas_ddot(n,Vold,1,V,1);
  if (beta < 0.0) {
    input.Com->ninjaCom(ninjaComClass::ninjaFailure, "solver: error (2)");
    return false;
  }
  beta = std::sqrt(beta);

  gammabar = alpha;
  epsilon = 0.0;
  deltabar = beta;
  gamma = std::sqrt(gammabar*gammabar + beta*beta);


  for (i=0; i<n; i++) Mold[i] = 0.0;
  for (i=0; i<n; i++) M[i]    = VV[i] / gamma;

  cs = gammabar / gamma;
  sn = beta / gamma;


  for (i=0; i<n; i++) x[i] += snprod*cs*M[i];
  snprod = snprod * sn;

  // generate CG iterates
  for (i=0; i<n; i++) Xcg[i] = x[i] + snprod*(sn/cs)*M[i];

  // compute residual again

  //matrix vector multiplication A*x=Ax
  mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], x, &zero, R);   // R = A*x
  //taucs_ccs_times_vec(A,x,R);
  for (i=0; i<n; i++) R[i] = b[i] - R[i];  // r = b - A*x
  normr = cblas_dnrm2(n,R,1);

  input.Com->ninjaCom(ninjaComClass::ninjaDebug, "starting iterations, residual norm is %.1e",normr);

  // Begin iterations
  for ( Iter=1; Iter <= max_iter; Iter++ ) {

    for (i=0; i<n; i++) VV[i] = V[i] / beta;
	mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], VV, &zero, V);   // V = A*VV
    //taucs_ccs_times_vec(A,VV,V);
    for (i=0; i<n; i++) V[i] -= (beta/betaold) * Volder[i];
    alpha = cblas_ddot(n,VV,1,V,1);
    for (i=0; i<n; i++) V[i] -= (alpha/beta) * Vold[i];

    for (i=0; i<n; i++) Volder[i] = Vold[i];
    for (i=0; i<n; i++) Vold  [i] = V   [i];

	//(*precond_fn)(precond_args,V,Vold);
	precond.solve(Vold, V, row_ptr, col_ind);	//apply preconditioner   MV = Vold

    betaold = beta;
    beta = cblas_ddot(n,Vold,1,V,1);
    if (beta < 0.0) {
      input.Com->ninjaCom(ninjaComClass::ninjaFailure, "solver: error (3)");
      return false;
    }
    beta = std::sqrt(beta);

    delta = cs*deltabar + sn*alpha;
    for (i=0; i<n; i++) Molder[i] = Mold[i];
    for (i=0; i<n; i++) Mold  [i] = M   [i];
    for (i=0; i<n; i++) M[i] = VV[i] - delta*Mold[i] - epsilon*Molder[i];
    gammabar = sn*deltabar - cs*alpha;
    epsilon = sn*beta;
    deltabar = -cs*beta;
    gamma = std::sqrt(gammabar*gammabar + beta*beta);
    for (i=0; i<n; i++) M[i] = M[i]/ gamma;
    cs = gammabar / gamma;
    sn = beta / gamma;

    // stagnation test; skipped

    for (i=0; i<n; i++) x[i] += snprod*cs*M[i];
    snprod = snprod*sn;
    for (i=0; i<n; i++) Xcg[i] = x[i] + snprod*(sn/cs)*M[i];

	mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], x, &zero, R);   // R = A*x
    for (i=0; i<n; i++) R[i] = b[i] - R[i];  // r = b - A*x
    normr = cblas_dnrm2(n,R,1);


	if((Iter%print_iters)==0)
    {
#ifdef NINJA_DEBUG_VERBOSE
		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: n=%d iterations = %d residual norm %12.4e", n,Iter,normr);
		//input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %d\tResidual = %lf\ttol = %lf", i, resid, tol);
		fprintf(convergence_history,"\n%ld\t%lf\t%lf",Iter,normr,tolb);
#endif //NINJA_DEBUG_VERBOSE

		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "n=%d iterations = %d residual norm %12.4e", n,Iter,normr);
		fprintf(convergence_history,"\n%ld\t%lf\t%lf",Iter,normr,tolb);


	    //residual_percent_complete=100-100*((resid-tol)/(start_resid-tol));
		//if(residual_percent_complete<residual_percent_complete_old)
		//	residual_percent_complete=residual_percent_complete_old;
		//if(residual_percent_complete<0.)
		//	 residual_percent_complete=0.;
		//else if(residual_percent_complete>100.)
		//	residual_percent_complete=100.0;

		//time_percent_complete=1.8*exp(0.0401*residual_percent_complete);
		//residual_percent_complete_old=residual_percent_complete;
//fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,residual_percent_complete, time_percent_complete);
		//input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));

	}

    if (normr <= tolb) break;
  }	// End iterations

  input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: done. n=%d iterations = %d residual norm %12.4e", n,Iter,normr);

  fclose(convergence_history);

  delete[] Molder;
  delete[] Mold;
  delete[] M;
  delete[] Volder;
  delete[] Vold;
  delete[] V;
  delete[] VV;
  delete[] Xcg;
  delete[] R;

  return true;

}*/

/*bool ninja::taucs_solve(taucs_double *A, double *b, double *x, int *row_ptr, int *col_ind, int NUMNP, int max_iter, int print_iters, double tol)
{
	int out;

	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Entering taucs_ccs_create...");
	taucs_ccs_matrix* pMatrix = taucs_ccs_create(NUMNP, NUMNP, row_ptr[NUMNP], TAUCS_DOUBLE|TAUCS_SYMMETRIC|TAUCS_LOWER);
	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Out of taucs_ccs_create, building matrix...");

	pMatrix->values.d = A;
	pMatrix->rowind = col_ind;
	pMatrix->colptr = row_ptr;

	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Matrix built, setting options...");

//	char* options[] = {"taucs.factor.LLT=true", NULL};
//	char* options[] = {"taucs.factor.LLT=true", "taucs.factor.droptol=0", NULL};
//	char* options[] = {"taucs.solve.minres=true", "taucs.solve.convergetol=0.01", NULL};
	char* options[] = {"taucs.solve.minres=true", "taucs.solve.maxits=1000", "taucs.solve.convergetol=1E-1", NULL};
//	char* options[] = {"taucs.solve.cg=true", NULL};
//	char* options[] = {"taucs.factor.mf=true", NULL};
//	char* options[] = {"taucs.solve.minres=true", "taucs.solve.maxits=1000", "taucs.solve.convergetol=1E-10", "taucs.factor.droptol=10000000", "taucs.factor.LLT=true", NULL};
//	char* options[] = {"taucs.approximate.amwb=true","taucs.approximate.amwb.subgraphs=300","taucs.factor.LLT=true","taucs.solve.minres=true","taucs.solve.maxits=1000", "taucs.solve.convergetol=1E-1", NULL};

	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Options set, running solver...");
	out = taucs_linsolve(pMatrix, NULL, 1, x, b, options, NULL);
	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done!! Freeing pMatrix...");

	if (out != TAUCS_SUCCESS)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Solver failed");
		if(out==TAUCS_ERROR)
			input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Error code = TAUCS_ERROR");
		else if(out==TAUCS_ERROR_NOMEM)
			input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Error code = TAUCS_ERROR");
		else if(out==TAUCS_ERROR_BADARGS)
			input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Error code = TAUCS_ERROR_BADARGS");
		else if(out==TAUCS_ERROR_MAXDEPTH)
			input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Error code = TAUCS_ERROR_MAXDEPTH");
		else if(out==TAUCS_ERROR_INDEFINITE)
			input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Error code = TAUCS_ERROR_INDEFINITE");
	}

//	taucs_ccs_free(pMatrix);

	input.Com->ninjaCom(ninjaComClass::ninjaDebug, "pMatrix freed, exiting taucs_solver...");

	  //if(resid>tol)
	  //{
			//input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Solution did not converge.\nMAXITS reached.");
			//return false;
	  //}else{
			//input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done.");
			//return true;
	  //}


	return true;
}*/
/**
 * @brief Applies a plane rotation for GMRES solver.
 * @param dx Point in Hessenburg matrix
 * @param dy Point in Hessenburg matrix
 * @param cs Vector
 * @param sn Vector
 */
void ninja::gmresApplyPlaneRotation(double &dx, double &dy, double &cs, double &sn)
{
     double temp  =  cs * dx + sn * dy;
     dy = -sn * dx + cs * dy;
     dx = temp;
}

/**
 * @brief Generates a plane rotation for GMRES solver.
 * @param dx Point in Hessenburg matrix
 * @param dy Point in Hessenburg matrix
 * @param cs Vector
 * @param sn Vector
 */
void ninja::gmresGeneratePlaneRotation(double &dx, double &dy, double &cs, double &sn)
{
     if (dy == 0.0) {
        cs = 1.0;
        sn = 0.0;
    } else if (abs(dy) > abs(dx)) {
        double temp = dx / dy;
        sn = 1.0 / sqrt( 1.0 + temp*temp );
        cs = temp * sn;
    } else {
        double temp = dy / dx;
        cs = 1.0 / sqrt( 1.0 + temp*temp );
        sn = temp * cs;
    }

}
/**
 * @brief Update function for GMRES solver.
 * @param dx Point in Hessenburg matrix
 * @param dy Point in Hessenburg matrix
 * @param cs Vector
 * @param sn Vector
 */
void ninja::gmresUpdate(double *x, int k, double *h, double *s, double *v)
{
    double *y, *vTemp;
    int m = k + 1;  // k = m - 1

    y = new double[m+1];

    // Backsolve:
    for (int i = k; i >= 0; i--) {
        y[i] /= h[(i * m) + i];
        for (int j = i - 1; j >= 0; j--){
            y[j] -= h[(j * m) + i] * y[i];
        }
    }

    for (int j = 0; j <= k; j++){
        //put col j of v in vTemp
        for(int jj=0;jj<m;jj++){ // m is nrows in v
                   vTemp[jj] = v[jj * (m + 1) + j]; // vTemp <- v[k] (put col j of v in vTemp); m+1 = ncols
        }

        x[j] += vTemp[j] * y[j];
    }

    if(y)
        delete[] y;
}

#ifndef MKL
/**@brief Performs the calculation X = alpha * X.
 * X is an N-element vector and alpha is a scalar.
 * A limited version of the BLAS function dscal().
 * @param N Size of vectors.
 * @param X Source vector.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 */
void ninja::cblas_dscal(const int N, const double alpha, double *X, const int incX)
{
     int i, ix;

     if (incX <= 0) return;

     ix = OFFSET(N, incX);

     for (i = 0; i < N; i++) {
         X[ix] *= alpha;
         ix    += incX;
     }
}
#endif	//MKL


#ifndef MKL
/**Copies values from the X vector to the Y vector.
 * A limited version of the BLAS function dcopy().
 * @param N Size of vectors.
 * @param X Source vector.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @param Y Target vector to copy values to.
 * @param incY Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 */
void ninja::cblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY)
{
	int i;
	for(i=0; i<N; i++)
		Y[i] = X[i];
}
#endif	//MKL

#ifndef MKL
/**Performs the dot product X*Y.
 * A limited version of the BLAS function ddot().
 * @param N Size of vectors.
 * @param X Vector of size N.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @param Y Vector of size N.
 * @param incY Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @return Dot product X*Y value.
 */
double ninja::cblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY)
{
	double val=0.0;
	int i;

	#pragma omp parallel for reduction(+:val)
	for(i=0;i<N;i++)
		val += X[i]*Y[i];

	return val;
}
#endif	//MKL

#ifndef MKL
/**Performs the calculation Y = Y + alpha * X.
 * A limited version of the BLAS function daxpy().
 * @param N Size of vectors.
 * @param alpha
 * @param X Vector of size N.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @param Y Vector of size N.
 * @param incY Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 */
void ninja::cblas_daxpy(const int N, const double alpha, const double *X, const int incX, double *Y, const int incY)
{
	int i;

	#pragma omp parallel for
	for(i=0; i<N; i++)
		Y[i] += alpha*X[i];
}
#endif	//MKL

#ifndef MKL
/**Computes the 2-norm of X.
 * A limited version of the BLAS function dnrm2().
 * @param N Size of X.
 * @param X Vector of size N.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @return Value of the 2-norm of X.
 */
double ninja::cblas_dnrm2(const int N, const double *X, const int incX)
{
	double val=0.0;
	int i;

	//#pragma omp parallel for reduction(+:val)
	for(i=0;i<N;i++)
		val += X[i]*X[i];
	val = std::sqrt(val);

	return val;
}
#endif	//MKL

#ifndef MKL
/**
 * @brief Computes the vector-matrix product A*x=y.
 *
 * This is a limited version of the BLAS function dcsrmv().
 * It is limited in that the matrix must be in compressed sparse row
 * format, and symmetric with only the upper triangular part stored.
 * ALPHA=1 and BETA=0 must also be true.
 *
 * @note My version of MKL's compressed sparse row (CSR) matrix vector product function
 * MINE ONLY WORKS FOR A SYMMETRICALLY STORED, UPPER TRIANGULAR MATRIX!!!!!!
 * AND ALPHA==1 AND BETA==0
 *
 * @param transa Not used here, but included to stay with the BLAS.
 * @param m Number of rows in "A" matrix. Must equal k in this implementation.
 * @param k Number of columns in "A" matrix. Must equal m in this implementation.
 * @param alpha Must be one for this implementation.
 * @param matdescra Not used here, but included to stay with the BLAS.
 * @param val This is the "A" array.  Must be in compressed sparse row format, and symmetric with only the upper triangular part stored.
 * @param indx An array describing the column index of "A" (sometimes called "col_ind").
 * @param pntrb A pointer containing indices of "A" of the starting locations of the rows.
 * @param pntre A pointer containg indices of "A" of the ending locations of the rows.
 * @param x Vector of size m (and k) in the A*x=y computation.
 * @param beta Not used here, but included to stay with the BLAS.
 * @param y Vector of size m (and k) in the A*x=y computation.
 */
void ninja::mkl_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y)
{	// My version of MKL's compressed sparse row (CSR) matrix vector product function
	// MINE ONLY WORKS FOR A SYMMETRICALLY STORED, UPPER TRIANGULAR MATRIX!!!!!!
	// AND ALPHA==1 AND BETA==0

		//function multiplies a sparse matrix "val" times a vector "x", result is stored in "y"
		//		ie. Ax = y
		//"m" and "k" are equal to the number of rows and columns in "A" (must be equal in mine)
		//"indx" is an array describing the column index of "A" (sometimes called "col_ind")
		//"pntrb" is a pointer containing indices of "A" of the starting locations of the rows
		//"pntre" is a pointer containg indices of "A" of the ending locations of the rows
		int i,j,N;
		N=*m;

    #pragma omp parallel private(i,j)
    {
        #pragma omp for
        for(i=0;i<N;i++)
            y[i]=0.0;

        #pragma omp for
        for(i=0;i<N;i++)
        {
            y[i] += val[pntrb[i]]*x[i];	// diagonal
            for(j=pntrb[i]+1;j<pntre[i];j++)
            {
                y[i] += val[j]*x[indx[j]];
            }
        }
    }	//end parallel region

    for(i=0;i<N;i++)
    {
        for(j=pntrb[i]+1;j<pntre[i];j++)
        {
            {
                y[indx[j]] += val[j]*x[i];
            }
        }
    }
}
#endif

#ifndef MKL
/**
 * @brief Computes the vector-matrix product A^T*x=y.
 *
 * This is a modified version of mkl_dcsrmv().
 * It is modiefied to compute the product of the traspose of the matrix and
 * the x vector. ALPHA=1 and BETA=0 must be true.
 *
 * @note My version of MKL's compressed sparse row (CSR) matrix vector product function
 * THIS FUNCTION ONLY WORKS FOR A FULLY STORED MATRIX!!!!!!
 * AND ALPHA==1 AND BETA==0
 *
 * @param transa Not used here, but included to stay with the BLAS.
 * @param m Number of rows in "A" matrix. Must equal k in this implementation.
 * @param k Number of columns in "A" matrix. Must equal m in this implementation.
 * @param alpha Must be one for this implementation.
 * @param matdescra Not used here, but included to stay with the BLAS.
 * @param val This is the "A" array.  Must be in compressed sparse row format, and symmetric with only the upper triangular part stored.
 * @param indx An array describing the column index of "A" (sometimes called "col_ind").
 * @param pntrb A pointer containing indices of "A" of the starting locations of the rows.
 * @param pntre A pointer containg indices of "A" of the ending locations of the rows.
 * @param x Vector of size m (and k) in the A*x=y computation.
 * @param beta Not used here, but included to stay with the BLAS.
 * @param y Vector of size m (and k) in the A*x=y computation.
 */
void ninja::mkl_trans_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y)
{
		//function multiplies the traspose of a sparse matrix "val" times a vector "x", result is stored in "y"
		//		ie. A^Tx = y
		//"m" and "k" are equal to the number of rows and columns in "A" (must be equal in mine)
		//"indx" is an array describing the column index of "A" (sometimes called "col_ind")
		//"pntrb" is a pointer containing indices of "A" of the starting locations of the rows
		//"pntre" is a pointer containg indices of "A" of the ending locations of the rows
		int i,j,N;
		N=*m;
		double *temp;

		temp = new double[N];  //vector to store (A[j] * x[j]) for each row in A (rows are really cols in A^T)

		for(int i = 0; i < N; i++){
            temp[i] = 0.0;
		}

    #pragma omp parallel private(i,j)
    {

        #pragma omp for
        for(i=0;i<N;i++){
            y[i]=0.0;
        }

        #pragma omp for
        for(i=0;i<N;i++)
        {
            // calculates A^Tx "in place" (A^T does not physically exist)
            if(matdescra[0] == 'g'){
                for(j=pntrb[i];j<pntre[i];j++) // iterate over all columns
                {
                    temp[indx[j]] += val[j]*x[i];  // for entire row: add element[j] * x[i] to element[j] in temp vector
                }
            }
        }

        cblas_dcopy(N, temp, 1, y, 1); // y <- temp

    }	//end parallel region

    if(temp)
        delete[] temp;
}
#endif

//double ninja::norm_residual(double *SK, double *PHI, double *RHS, int *row_ptr, int *col_ind, int NUMNP)
//{
//     //Calculates the 2-norm of the residual vector = b - Ax (not formulated for symmetrically stored matrix yet!!!)
//     double residual=0;
//     double Axi, *r;
//     int i, j;
//
//     r=new double[NUMNP];
//
//     for(i=0;i<NUMNP;i++)
//     {
//          r[i]=0.0;
//     }
//
//     for(i=0;i<NUMNP;i++)
//     {
//          Axi=0;
//          for(j=row_ptr[i];j<row_ptr[i+1];j++)
//               Axi=Axi+SK[j]*PHI[col_ind[j]];     //Multiply Ax
////          input.Com->ninjaCom(ninjaComClass::ninjaDebug, "2_norm_Axi=%lf",Axi);
////          inter=(RHS[i]-Axi);
////          input.Com->ninjaCom(ninjaComClass::ninjaDebug, "inter=%lf",inter);
//          r[i]=RHS[i]-Axi;
////          input.Com->ninjaCom(ninjaComClass::ninjaDebug, "r[%ld]=%lf\tRHS=%lf\tAxi=%lf",i,r[i],RHS[i],Axi);
//          r[i]=r[i]*r[i];
//     }
//     for(i=0;i<NUMNP;i++)
//          residual=residual+r[i];
//
//     residual=std::sqrt(residual);
//
//	 if(r)
//	 {
//		delete[] r;
//		r=NULL;
//	 }
//
//     return residual;
//}

//double ninja::norm_residual_infin(double *SK, double *PHI, double *RHS, int *row_ptr, int *col_ind, int NUMNP)
//{
//     //Calculates the infinity-norm of the residual vector = b - Ax
//     double residual=0;
//     double Axi;
//     int i, j;
//     double answer=0;
//
//     for(i=0;i<NUMNP;i++)
//     {
//          Axi=0;
//          for(j=row_ptr[i];j<row_ptr[i+1];j++)
//               Axi=Axi+SK[j]*PHI[col_ind[j]];     //Multiply Ax
////          ninjaCom(ninjaDebug, "infin_norm_Axi=%lf",Axi);
//          residual=fabs(RHS[i]-Axi);
//          if(residual>answer)
//               answer=residual;
//     }
//
//     return answer;
//}

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
                    profile.ObukovLength = L(i,j);
                    profile.ABL_height = bl_height(i,j);
                    profile.Roughness = input.surface.Roughness(i,j);
                    profile.Rough_h = input.surface.Rough_h(i,j);
                    profile.Rough_d = input.surface.Rough_d(i,j);
                    profile.inputWindHeight = h2 - input.surface.Rough_h(i,j);

                    profile.AGL=input.outputWindHeight + input.surface.Rough_h(i,j);			//this is height above THE GROUND!! (not "z=0" for the log profile)

                    profile.inputWindSpeed = u(i, j, k);
                    uu = profile.getWindSpeed();

                    profile.inputWindSpeed = v(i, j, k);
                    vv = profile.getWindSpeed();

                    profile.inputWindSpeed = w(i, j, k);
                    ww = profile.getWindSpeed();

                    VelocityGrid(i,j)=std::pow((uu*uu+vv*vv),0.5);       //calculate velocity magnitude (in x,y plane; I decided to NOT include z here so the wind is the horizontal wind)

                }else{  //else use linear interpolation
                    slopeu=(u(i, j, k)-u(i, j, k-1))/(h2-h1);
                    slopev=(v(i, j, k)-v(i, j, k-1))/(h2-h1);
                    slopew=(w(i, j, k)-w(i, j, k-1))/(h2-h1);
                    uu=slopeu*(input.outputWindHeight + input.surface.Rough_h(i,j))+u(i, j, k-1)-slopeu*h1;
                    vv=slopev*(input.outputWindHeight + input.surface.Rough_h(i,j))+v(i, j, k-1)-slopev*h1;
                    ww=slopew*(input.outputWindHeight + input.surface.Rough_h(i,j))+w(i, j, k-1)-slopew*h1;
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

/**Function used to write the A matrix and b vector to a file.
 * Used for debugging purposes.
 * @param NUMNP Number of node points.
 * @param A "A" matrix in A*x=b computation. Stored in compressed row storage format.
 * @param col_ind Column index vector for compressed row stored matrix A.
 * @param row_ptr Row pointer vector for compressed row stored matrix A.
 * @param b "b" vector in A*x=b computation.  Size is NUMNP.
 */
void ninja::write_A_and_b(int NUMNP, double *A, int *col_ind, int *row_ptr, double *b)
{
    FILE *A_file;
    FILE *b_file;

    A_file = fopen("A.txt", "w");
	b_file = fopen("b.txt", "w");

	int i,j;

	for(i=0;i<NUMNP;i++)
	{
		for(j=row_ptr[i];j<row_ptr[i+1];j++)
			fprintf(A_file,"%d = %lf , %d\n", j, A[j], col_ind[j]);
		fprintf(b_file,"%d = %lf\n", i, b[i]);
	}

	fclose(A_file);
	fclose(b_file);
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
		if(u0(i) != 0.0)
			isNullRun = false;
	}
	for(i=0;i<mesh.NUMNP;i++)
	{
		if(v0(i) != 0.0)
			isNullRun = false;
	}
	for(i=0;i<mesh.NUMNP;i++)
	{
		if(w0(i) != 0.0)
			isNullRun = false;
	}

	return isNullRun;
}

/**Function to build discretized equations.
 *
 * Discretizes wind flow or scalar transport govering equations. Scalar transport equations must be
 * discretized after the flow field is solved. If scalarTransportSimulation == true, the scalar transport
 * equations will be discretized, otherwise the flow equations will be discretized.
 *
 * @param scalarTransportSimulation flag to indicate whether flow or scalar transport equations should be discretized.

 */
void ninja::discretize(bool scalarTransportSimulation)
{
    //The governing equation to solve is
    //
    //    d        dPhi      d        dPhi      d        dPhi
    //   ---- ( Rx ---- ) + ---- ( Ry ---- ) + ---- ( Rz ---- ) + H = 0.0
    //    dx        dx       dy        dy       dz        dz
    //
    //        where
    //
    //                    1                          1
    //    Rx = Ry =  ------------          Rz = ------------
    //                2*alphaH^2                 2*alphaV^2
    //
    //         du0     dv0     dz0
    //    H = ----- + ----- + -----
    //         dx      dy      dz

    /*
     * Governing equation for scalar transport:
     *
     * d       dC      d       dC      d       dC        dC       dC       dC
     * -- ( Rx -- ) +  -- ( Ry -- ) +  -- ( Rz -- ) + Bx --  + By --  + Bz -- + H = 0
     * dx      dx      dy      dy      dz      dz        dx       dy       dz
     *
     *      1               2               3           4         5        6    7
     *
     * Rx, Ry, Rz = eddy diffusivities
     * Bx = u, By = v, Bz = w
     * H = volume source term (with units of mass/volume)
     *
     * terms 1-3 = diffusion
     * terms 4-6 = advection
     *
     */

	//Set array values to zero----------------------------
	if(PHI == NULL)
		PHI=new double[mesh.NUMNP];

     int interrows=input.dem.get_nRows()-2;
     int intercols=input.dem.get_nCols()-2;
     int interlayers=mesh.nlayers-2;
	 int i, ii, j, jj, k, kk, l;
                         //NZND is the # of nonzero elements in the SK stiffness array that are stored
     int NZND=(8*8)+(intercols*4+interrows*4+interlayers*4)*12+(intercols*interlayers*2+interrows*interlayers*2+intercols*interrows*2)*18+(intercols*interrows*interlayers)*27;

     #ifdef SCALAR
     if(scalarTransportSimulation == false){ // if scalar transport run, need to store full matrix since it is asymmetric
        NZND = (NZND - mesh.NUMNP)/2 + mesh.NUMNP;      //this is because we will only store the upper half of the SK matrix since it's symmetric
     }
     #endif //SCALAR
     #ifndef SCALAR
     NZND = (NZND - mesh.NUMNP)/2 + mesh.NUMNP;	//this is because we will only store the upper half of the SK matrix since it's symmetric
     #endif

	 #ifdef MKL
	 SK = (double*)MKL_malloc(NZND*sizeof(double),16);
	 #else

	 SK = new double[NZND];	//This is the final global stiffness matrix in Compressed Row Storage (CRS) and symmetric (symmetric if not a scalar transport run)

	 //SK = new taucs_double[NZND];
	 #endif

	 col_ind=new int[NZND];      //This holds the global column number of the corresponding element in the CRS storage
	 row_ptr=new int[mesh.NUMNP+1];     //This holds the element number in the SK array (CRS) of the first non-zero entry for the global row (the "+1" is so we can use the last entry to quit loops; ie. so we know how many non-zero elements are in the last node)
	 RHS=new double[mesh.NUMNP];       //This is the final right hand side (RHS) matrix

     int type;                     //This is the type of node (corner, edge, side, internal)
     int temp,temp1;

     #pragma omp parallel for default(shared) private(i)
	 for(i=0;i<mesh.NUMNP;i++)
     {
          PHI[i]=0.;
          RHS[i]=0.;
          row_ptr[i]=0;
     }

	 #pragma omp parallel for default(shared) private(i)
     for(i=0;i<NZND;i++)
     {
          col_ind[i]=0;
          SK[i]=0.;
     }

	 int row, col;

     //Set up Compressed Row Storage (CRS) format (only store upper triangular SK matrix)
     temp=0;        //temp stores the location (in the SK and col_ind arrays) where the first non-zero element for the current row is located
	 for(k=0;k<mesh.nlayers;k++)
     {
          for(i=0;i<input.dem.get_nRows();i++)
          {
               for(j=0;j<input.dem.get_nCols();j++)     //Looping over all nodes using i,j,k notation
               {
				   type=mesh.get_node_type(i,j,k);
                    if(type==0)         //internal node
                    {
                         row = k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j;
						 row_ptr[row]=temp;
                         temp1=temp;
                         for(kk=-1;kk<2;kk++)
                         {
                              for(ii=-1;ii<2;ii++)
                              {
                                   for(jj=-1;jj<2;jj++)
                                   {
                                       #ifdef SCALAR
                                       if(scalarTransportSimulation == true){
                                           col_ind[temp1]=(k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                           temp1++;
                                       }
                                       if(scalarTransportSimulation == false){
                                           col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
									       if(col >= row)	//only do if we're on the upper triangular part of SK
									       {
											    col_ind[temp1]=col;
											    temp1++;
									       }
                                       }
                                       #endif //SCALAR
                                       #ifndef SCALAR
                                       col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                       if(col >= row)	//only do if we're on the upper triangular part of SK
									   {
                                           col_ind[temp1]=col;
                                           temp1++;
                                       }
                                       #endif
                                   }
                              }
                         }
                         #ifdef SCALAR
                         if(scalarTransportSimulation == true){
                             temp=temp+27;
                         }
                         if(scalarTransportSimulation == false){
                             temp=temp1;
                         }
                         #endif
                         #ifndef SCALAR
                         //temp=temp+27;
						 temp=temp1;
						 #endif
                    }else if(type==1)   //face node
                    {
                         row = k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j;
						 row_ptr[row]=temp;
						 temp1=temp;
                         for(kk=-1;kk<2;kk++)
                         {
                              for(ii=-1;ii<2;ii++)
                              {
                                   for(jj=-1;jj<2;jj++)
                                   {
                                        if(((i+ii)<0)||((i+ii)>(input.dem.get_nRows()-1)))
                                             continue;
                                        if(((j+jj)<0)||((j+jj)>(input.dem.get_nCols()-1)))
                                             continue;
                                        if(((k+kk)<0)||((k+kk)>(mesh.nlayers-1)))
                                             continue;

                                        #ifdef SCALAR
                                        if(scalarTransportSimulation == true){
                                            col_ind[temp1]=(k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                            temp1++;
                                        }
                                        if(scalarTransportSimulation == false){
                                            col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                            if(col >= row)	//only do if we're on the upper triangular part of SK
                                            {
                                                col_ind[temp1]=col;
                                                temp1++;
                                            }
                                        }
                                        #endif
                                        #ifndef SCALAR
                                        col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                        if(col >= row)	//only do if we're on the upper triangular part of SK
                                        {
                                            col_ind[temp1]=col;
                                            temp1++;
                                        }
                                        #endif
                                   }
                              }
                         }
                         #ifdef SCALAR
                         if(scalarTransportSimulation == true){
                             temp=temp+18;
                         }
                         if(scalarTransportSimulation == false){
                             temp=temp1;
                         }
                         #endif
                         #ifndef SCALAR
                         //temp=temp+18;
						 temp=temp1;
						 #endif
                    }else if(type==2)   //edge node
                    {
                         row = k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j;
						 row_ptr[row]=temp;
						 temp1=temp;
                         for(kk=-1;kk<2;kk++)
                         {
                              for(ii=-1;ii<2;ii++)
                              {
                                   for(jj=-1;jj<2;jj++)
                                   {
                                        if(((i+ii)<0)||((i+ii)>(input.dem.get_nRows()-1)))
                                             continue;
                                        if(((j+jj)<0)||((j+jj)>(input.dem.get_nCols()-1)))
                                             continue;
                                        if(((k+kk)<0)||((k+kk)>(mesh.nlayers-1)))
                                             continue;

                                        #ifdef SCALAR
                                        if(scalarTransportSimulation == true){
                                            col_ind[temp1]=(k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                            temp1++;
                                        }
                                        if(scalarTransportSimulation == false){
                                            col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                            if(col >= row)	//only do if we're on the upper triangular part of SK
                                            {
                                                col_ind[temp1]=col;
                                                temp1++;
                                            }
                                        }
                                        #endif
                                        #ifndef SCALAR
										col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
									    if(col >= row)	//only do if we're on the upper triangular part of SK
									    {
											col_ind[temp1]=col;
											temp1++;
									    }
									    #endif
                                   }
                              }
                         }
                         #ifdef SCALAR
                         if(scalarTransportSimulation == true){
                             temp=temp+12;
                         }
                         if(scalarTransportSimulation == false){
                                                     temp=temp1;
                         }
                         #endif
                         #ifndef SCALAR
                         //temp=temp+12;
						 temp=temp1;
						 #endif
                    }else if(type==3)   //corner node
                    {
                         row = k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j;
						 row_ptr[row]=temp;
						 temp1=temp;
                         for(kk=-1;kk<2;kk++)
                         {
                              for(ii=-1;ii<2;ii++)
                              {
                                   for(jj=-1;jj<2;jj++)
                                   {
                                        if(((i+ii)<0)||((i+ii)>(input.dem.get_nRows()-1)))
                                             continue;
                                        if(((j+jj)<0)||((j+jj)>(input.dem.get_nCols()-1)))
                                             continue;
                                        if(((k+kk)<0)||((k+kk)>(mesh.nlayers-1)))
                                             continue;

                                        #ifdef SCALAR
                                        if(scalarTransportSimulation == true){
                                            col_ind[temp1]=(k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                            temp1++;
                                        }
                                        if(scalarTransportSimulation == false){
                                            col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
                                            if(col >= row)  //only do if we're on the upper triangular part of SK
                                            {
                                                col_ind[temp1]=col;
                                                temp1++;
                                            }
                                        }
                                        #endif
                                        #ifndef SCALAR
										col = (k+kk)*input.dem.get_nCols()*input.dem.get_nRows()+(i+ii)*input.dem.get_nCols()+(j+jj);
									    if(col >= row)	//only do if we're on the upper triangular part of SK
									    {
											col_ind[temp1]=col;
											temp1++;
									    }
									    #endif
                                   }
                              }
                         }
                         #ifdef SCALAR
                         if(scalarTransportSimulation == true){
                             temp=temp+8;
                         }
                         if(scalarTransportSimulation == false){
                             temp=temp1;
                         }
                         #endif
                         #ifndef SCALAR
                         //temp=temp+8;
						 temp=temp1;
						 #endif
                    }
					else
						throw std::logic_error("Error arranging SK array.  Exiting...");
               }
          }
     }
     row_ptr[mesh.NUMNP]=temp;     //Set last value of row_ptr, so we can use "row_ptr+1" to use to index to in loops

	 checkCancel();

    #ifdef STABILITY
    CPLDebug("STABILITY", "input.initializationMethod = %i\n", input.initializationMethod);
    CPLDebug("STABILITY", "input.stabilityFlag = %i\n", input.stabilityFlag);
    Stability stb(input);
    alphaVfield.allocate(&mesh);

    if(input.stabilityFlag==0){  // if stabilityFlag not set
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows;i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/1.0;
                }
            }
        }
    }
    else if(input.stabilityFlag==1 && input.alphaStability!=-1) // if the alpha was specified directly in the CLI
    {
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/input.alphaStability;
                }
            }
        }
    }
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::domainAverageInitializationFlag) //it's a domain-average run
	{
        stb.SetDomainAverageAlpha(input, mesh);  //sets alpha based on incident solar radiation
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0;j<input.dem.get_nCols();j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::pointInitializationFlag) //it's a point-initialization run
	{
        stb.SetPointInitializationAlpha(input, mesh);
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    //If the run is a 2D WX model run
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag &&
            wxInit->getForecastIdentifier()!="WRF-3D") //it's a 2D wx model run
    {
        stb.Set2dWxInitializationAlpha(input, mesh, CloudGrid);

        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
	//If the run is a WX model run where the full WX vertical profile is used and
	//the potential temp (theta) is available, then use the method below
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag &&
            wxInit->getForecastIdentifier()=="WRF-3D") //it's a 3D wx model run
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Calculating stability...");

        stb.Set3dVariableAlpha(input, mesh, wxInit->air3d, u0, v0);
        wxInit->air3d.deallocate();

        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    else{
        throw logic_error( string("Can't determine how to set atmophseric stability.") );
    }

    CPLDebug("STABILITY", "alphaVfield(0,0,0) = %lf\n", alphaVfield(0,0,0));

    #endif // STABILITY


#pragma omp parallel default(shared) private(i,j,k,l)
	 {
		 element elem(&mesh);
		 int pos;

		 #ifdef STABILITY
		 int ii, jj, kk;
         double alphaV; //alpha vertical from governing equation, weighting for change in vertical winds
		 #endif
		 #ifdef SCALAR
         if(scalarTransportSimulation == true){

            int scalarCell_i, scalarCell_j, scalarCell_k;
            double scalar_u, scalar_v, scalar_w;

            scalar.allocate(mesh);
            scalar.computeDiffusivity(input, mesh, u, v);  //creates wn_3dScalarFields scalar.Rx, scalar.Ry, scalar.Rz
            // add w component and compute shear with normal vector to ground


            /*
             * Find cell where scalar source is located
             * and compute the volume of this cell.
             *
             * This is for a point source (only doing calculations for
             * one or two cells), for area sources (e.g., PM10 emissions),
             * this should probably happen below in the loop over the
             * elements, just looking in each surface element to see
             * if it is a source cell or not.
             */

            //get ground elevation at source location
            double scalarSourceZORD;
            int scalar_elemNum;

            elem.get_ij(scalarSourceXORD, scalarSourceYORD, scalarCell_i, scalarCell_j);
            scalar_elemNum = mesh.get_elemNum(scalarCell_i, scalarCell_j, 0);
            elem.get_uv(scalarSourceXORD, scalarSourceYORD, scalarCell_i, scalarCell_j, scalar_u, scalar_v);

            //get elevation at midpoint betweeen layer 0 and 1 (w=0) in mesh at x,y location
            elem.get_xyz(scalar_elemNum, scalar_u, scalar_v, 0, scalarSourceXORD, scalarSourceYORD, scalarSourceZORD);

            cout<<"scalarSourceXORD, YORD = "<<scalarSourceXORD<<", "<<scalarSourceYORD<<endl;
            cout<<"scalarSourceZORD = "<<scalarSourceZORD<<endl;


            elem.get_uvw(scalarSourceXORD, scalarSourceYORD, scalarSourceZORD,
                       scalarCell_i, scalarCell_j, scalarCell_k, scalar_u, scalar_v, scalar_w);


            cout<<"cell_i, j, k = "<<scalarCell_i<<", "<<scalarCell_j<<", "<<scalarCell_k<<endl;
            cout<<"nrows, ncols = "<<mesh.nrows<<", "<<mesh.ncols<<endl;
            /// the above returns k=3, but should be k=0 ?? I think this is bc of the difference in the
            /// interpolation methods used above (bilinear) and in get_uvw(). Maybe this is an issue
            /// in the 3-d interoplations too??

            //compute volume of this cell, make k=0 or 1 no matter what.
            scalarCell_k = 1; // if source is in 0, ground nodes will be reset in setBoundaryConds(); may not be an issue...
            scalar.sourceElemNum = mesh.get_elemNum(scalarCell_i, scalarCell_j, scalarCell_k); // set point source cell

            for(j=0;j<elem.NUMQPTV;j++)   //loop over quadrature points in the element
            {
                elem.computeJacobianQuadraturePoint(j, scalar.sourceElemNum);
            }

            scalar.volumeSource = input.scalarSourceStrength / elem.DETJ; // DETJ is ratio of Vxxy/Vuvw--> divide (or multiply ?) by 8
            cout<<"elem.DETJ = "<<elem.DETJ<<endl;
            cout<<"scalar.volumeSource = "<<scalar.volumeSource<<endl;

            //testing--------------------------------------------
            std::string filename;
            AsciiGrid<double> testGrid;
            testGrid.set_headerData(input.dem);
            testGrid.set_noDataValue(-9999.0);
            int elemNum;

            for(int i = 0; i <mesh.nrows; i++){
                for(int j = 0; j < mesh.ncols; j++ ){
                    elemNum = mesh.get_elemNum(i, j, 1);

                    if(elemNum == scalar.sourceElemNum){
                        cout<<"scalar.sourceElemNum = "<< scalar.sourceElemNum<<endl;
                        cout<<"making source grid now..."<<endl;
                        testGrid(i,j) = 1;
                    }
                    else{
                        testGrid(i,j) = 0;
                    }
                }
            }
            testGrid.write_Grid("source_location", 2);
            testGrid.deallocate();
            //end testing:-----------------------------------------------
        } // end scalar
        #endif //SCALAR

#pragma omp for
		 for(i=0;i<mesh.NUMEL;i++)                    //Start loop over elements
		 {
			 /*-----------------------------------------------------*/
			 /*      NO SURFACE QUADRATURE NEEDED SINCE NONE OF     */
			 /*      THE BOUNDARY CONDITIONS HAVE A NON-ZERO FLUX   */
			 /*      SPECIFICATION:                                 */
			 /*      Flow through =>  Phi = 0                       */
			 /*      Ground       =>  normal flux = 0               */
			 /*-----------------------------------------------------*/



			 //elem.computeElementStiffnessMatrix(i, u0, v0, w0, alpha);







			 //Given the above parameters, function computes the element stiffness matrix

			 if(elem.SFV == NULL)
				 elem.initializeQuadPtArrays();

			 for(j=0;j<mesh.NNPE;j++)
			 {
				 elem.QE[j]=0.0;
				 for(int k=0;k<mesh.NNPE;k++)
					 elem.S[j*mesh.NNPE+k]=0.0;

			 }
			 //Begin quadrature for current element

			 elem.node0=mesh.get_node0(i);  //get the global nodal number of local node 0 of element i


			 for(j=0;j<elem.NUMQPTV;j++)             //Start loop over quadrature points in the element
			 {

				 elem.computeJacobianQuadraturePoint(j, i);

				 //Calculate the coefficient H here and the alpha-squared term in front of the second partial of z in governing equation (we are still on element i, quadrature point j)
				 //
				 //           d u0   d v0   d w0
				 //     H = ( ---- + ---- + ---- )
				 //           d x    d y    d z
				 //
				 //                and
                 //
                 //                     1                          1
                 //     Rx = Ry =  ------------          Rz = ------------
                 //                 2*alphaH^2                 2*alphaV^2

                 /*
				  * Governing equation for scalar transport:
				  *
				  * d    Rx dC   + d   Ry dC  + d   Rz dC  + Bx dC + By dC + Bz dC + H = 0
				  * -- (-------)   -- (-----)   -- (-----)      --      --      --
				  * dx    dx       dy   dy      dz    dz        dx      dy      dz
				  *
				  * set coefficients for scalar transport
				  *
				  * Rx = function of Rz
				  * Ry = function of Rz
				  * Rz = calculate from mixing length theory (from u*, height above ground, stability)
				  * for neutral conditions -->
				  *
				  * Rz(z) = lm * du/dz
				  * lm = 0.4 * z where z is distance to the wall
				  * Rx = Ry = 2 * Rz
				  *
				  * Bx = u-component of wind
				  * By = v-component of wind
				  * Bz = w-component of wind
				  *
                  * H = source term (mass/volume)
				  */


				 elem.HVJ=0.0;

				 double alphaV = 1;

				 #ifdef STABILITY
				 alphaV = 0;
				 #endif

				 #ifdef SCALAR
				 elem.RZ = 0.0;
				 elem.RX = 0.0;
				 elem.RY = 0.0;

				 elem.BX = 0.0;
				 elem.BY = 0.0;
				 elem.BZ = 0.0;
				 #endif

				 for(k=0;k<mesh.NNPE;k++)          //Start loop over nodes in the element
				 {
					 elem.NPK=mesh.get_global_node(k, i);            //NPK is the global nodal number

					 #ifdef SCALAR
					 if(scalarTransportSimulation == true){ // for scalar transport

					     // calculate H (the source term) for area or multiple point sources (do it all here instead of above)

					     //elem.HVJ = elem.HVJ + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*scalar.volumeSource;  //if don't assume source is evenly distributed in cell

                         //diffusion terms for scalar equation; need at quad point, only have these at the nodes, so need to sum to get at quad pt.
                         elem.RX = elem.RX + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*scalar.Rx(elem.NPK);  //elem.NPK is the global node number
                         elem.RY = elem.RY + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*scalar.Ry(elem.NPK);
                         elem.RZ = elem.RY + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*scalar.Rz(elem.NPK);

                         //populate advection terms
                         elem.BX = elem.BX + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j] * u(elem.NPK);
                         elem.BY = elem.BY + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j] * v(elem.NPK);
                         elem.BZ = elem.BZ + elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j] * w(elem.NPK);

                         /*cout<<"k (local node), j (quad point)"<<k<<", "<<j<<endl;
                         cout<<"elem.NPK (global node) = "<<elem.NPK<<endl;
                         cout<<"elemSFV[...] = "<<elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]<<endl;
                         cout<<"scalar.RX(elem.NPK), scalar.RY, scalar.RZ = "<<scalar.Rx(elem.NPK)<<", "<<scalar.Ry(elem.NPK)<<", "<<scalar.Rz(elem.NPK)<<endl;
                         cout<<"elem.RX, elem.RY, elem.RZ = "<<elem.RX<<", "<<elem.RY<<", "<<elem.RZ<<endl;
                         cout<<"elem.BX, elem.BY, elem.BZ = "<<elem.BX<<", "<<elem.BY<<", "<<elem.BZ<<endl;*/

					 }
					 if(scalarTransportSimulation == false){ // for wind flow
					     elem.HVJ=elem.HVJ+((elem.DNDX[k]*u0(elem.NPK))+(elem.DNDY[k]*v0(elem.NPK))+(elem.DNDZ[k]*w0(elem.NPK)));
					 }
					 #endif
                     #ifndef SCALAR
					 elem.HVJ=elem.HVJ+((elem.DNDX[k]*u0(elem.NPK))+(elem.DNDY[k]*v0(elem.NPK))+(elem.DNDZ[k]*w0(elem.NPK)));
					 #endif

					 #ifdef STABILITY
					 alphaV=alphaV+elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*alphaVfield(elem.NPK);
					 //cout<<"alphaV = "<<alphaV<<endl;
                     #endif
				 }                             //End loop over nodes in the element
				 //elem.HVJ=2*elem.HVJ;                    //This is the H for quad point j (the 2* comes from governing equation)

				 //elem.RZ=alpha*alpha;               //This is the RZ from the governing equation

				 #ifdef SCALAR //scalar
				 if(scalarTransportSimulation == true){

				     int elemi, elemj, elemk;
                     mesh.get_elemIndex(i, elemi, elemj, elemk); // i = elemNum
				     if(i == scalar.sourceElemNum){ //i = elemNum
                        elem.HVJ = scalar.volumeSource;  //assuming source is evenly distributed in cell; otherwise calc. at quad pt. (above)
				     }
				     else if(elemk == 1 && elemi == 85 && elemj == 70){ //testing
                        //elem.HVJ = scalar.volumeSource;
                        //elem.HVJ = 0.0001;
				     }
				     else{
                        //elem.HVJ = 0.0;  //0 if not a source cell
                        //elem.HVJ = 2.0;
				     }
				     elem.DV=elem.DETJ;  //DV is the DV for the volume integration
				 }
				 if(scalarTransportSimulation == false){  // for wind flow
                     elem.RX = 1.0/(2.0*alphaH*alphaH);
                     elem.RY = 1.0/(2.0*alphaH*alphaH);
                     elem.RZ = 1.0/(2.0*alphaV*alphaV);
                     elem.DV=elem.DETJ;  //DV is the DV for the volume integration (could be eliminated and just use DETJ everywhere)
                 }
				 #endif
				 #ifndef SCALAR
				 elem.RX = 1.0/(2.0*alphaH*alphaH);
				 elem.RY = 1.0/(2.0*alphaH*alphaH);
				 elem.RZ = 1.0/(2.0*alphaV*alphaV);
				 elem.DV=elem.DETJ;                      //DV is the DV for the volume integration (could be eliminated and just use DETJ everywhere)
				 #endif


				 if(elem.NUMQPTV==27)
				 {
					 if(j<=7)
					 {
						 elem.WT=elem.WT1;
					 }else if(j<=19)
					 {
						 elem.WT=elem.WT2;
					 }else if(j<=25)
					 {
						 elem.WT=elem.WT3;
					 }else
					 {
						 elem.WT=elem.WT4;
					 }
				 }

				 //Create element stiffness matrix---------------------------------------------
				 for(k=0;k<mesh.NNPE;k++)          //Start loop over nodes in the element
				 {
					 elem.QE[k]=elem.QE[k]+elem.WT*elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*elem.HVJ*elem.DV;
					 for(l=0;l<mesh.NNPE;l++)
					 {
					     #ifdef SCALAR //scalar
                         if(scalarTransportSimulation == true){  // for scalar transport


                             //modify elem.S for scalar equation
                             // 3 diffusion terms - 3 advection terms ([K] = [S1] - [S2])
                             elem.S[k*mesh.NNPE+l] = elem.S[k*mesh.NNPE+l]+
                                                     elem.WT*(elem.DNDX[k]*elem.RX*elem.DNDX[l] +
                                                     elem.DNDY[k]*elem.RY*elem.DNDY[l] +
                                                     elem.DNDZ[k]*elem.RZ*elem.DNDZ[l])*elem.DV; //- omit advection for now
                                                     //(elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*(elem.BX*elem.DNDX[l] +
                                                      //elem.BY*elem.DNDY[l] +
                                                      //elem.BZ*elem.DNDZ[l])))*elem.DV;

                             /*cout<<"u(elem.NPK), v, w = "<<u(elem.NPK)<<", "<<v(elem.NPK)<<", "<<w(elem.NPK)<<endl;
                             cout<<"elem.RX, elem.RY, elem.RZ = "<<elem.RX<<", "<<elem.RY<<", "<<elem.RZ<<endl;
                             cout<<"elem.DNDX[k], elem.DNDX[l] = "<<elem.DNDX[k]<<", "<<elem.DNDX[l]<<endl;
                             cout<<"elem.DNDY[k], elem.DNDY[l] = "<<elem.DNDY[k]<<", "<<elem.DNDY[l]<<endl;
                             cout<<"elem.DNDZ[k], elem.DNDZ[l] = "<<elem.DNDZ[k]<<", "<<elem.DNDZ[l]<<endl;
                             cout<<"elem.DV = "<<elem.DV<<endl;
                             cout<<"elem.S[...] = "<<elem.S[k*mesh.NNPE+l]<<endl;*/

                         }
                         if(scalarTransportSimulation == false){  // for wind flow
                             elem.S[k*mesh.NNPE+l]=elem.S[k*mesh.NNPE+l]+elem.WT*(elem.DNDX[k]*elem.RX*elem.DNDX[l] + elem.DNDY[k]*elem.RY*elem.DNDY[l] + elem.DNDZ[k]*elem.RZ*elem.DNDZ[l])*elem.DV;
					     }
                         #endif
                         #ifndef SCALAR
                         elem.S[k*mesh.NNPE+l]=elem.S[k*mesh.NNPE+l]+elem.WT*(elem.DNDX[k]*elem.RX*elem.DNDX[l] + elem.DNDY[k]*elem.RY*elem.DNDY[l] + elem.DNDZ[k]*elem.RZ*elem.DNDZ[l])*elem.DV;
                         #endif
					 }

				 }                            //End loop over nodes in the element

			 }                                  //End loop over quadrature points in the element



















			 //Place completed element matrix in global SK and Q matrices

			 for(j=0;j<mesh.NNPE;j++)                          //Start loop over nodes in the element (also, it is the row # in S[])
			 {
				 elem.NPK=mesh.get_global_node(j, i);            //elem.NPK is the global row number of the element stiffness matrix

#pragma omp atomic
				 RHS[elem.NPK] += elem.QE[j];

				 for(k=0;k<mesh.NNPE;k++)           //k is the local column number in S[]
				 {
					 elem.KNP=mesh.get_global_node(k, i);

					 #ifdef SCALAR
					 if(scalarTransportSimulation == true){ //storing full matrix

                         pos=-1;   //pos is the position # in SK[] to place S[j*mesh.NNPE+k]
                         l=0;      //l increments through col_ind[] starting from where row_ptr[] says until we find the column number we're looking for
                         do
                         {
                             if(col_ind[row_ptr[elem.NPK]+l]==elem.KNP)   //Check if we're at the correct position
                                 pos=row_ptr[elem.NPK]+l;    //If so, save that position in pos
                             l++;
                         }while(pos<0);
#pragma omp atomic
                         SK[pos] += elem.S[j*mesh.NNPE+k];     //Here is the final global stiffness matrix
                     }
                     if(scalarTransportSimulation == false){

					     if(elem.KNP >= elem.NPK)	//do only if we're on the upper triangular region of SK[]
					     {
						     pos=-1;                  //pos is the position # in SK[] to place S[j*mesh.NNPE+k]
						     l=0;                     //l increments through col_ind[] starting from where row_ptr[] says until we find the column number we're looking for
						     do
						     {
							     if(col_ind[row_ptr[elem.NPK]+l]==elem.KNP)   //Check if we're at the correct position
								     pos=row_ptr[elem.NPK]+l;           //If so, save that position in pos
							     l++;
						     }while(pos<0);

#pragma omp atomic
						     SK[pos] += elem.S[j*mesh.NNPE+k];     //Here is the final global stiffness matrix in symmetric storage
					     }
				     }
                     #endif
                     #ifndef SCALAR
					 if(elem.KNP >= elem.NPK)	//do only if we're on the upper triangular region of SK[]
					 {
						 pos=-1;                  //pos is the position # in SK[] to place S[j*mesh.NNPE+k]
						 l=0;                     //l increments through col_ind[] starting from where row_ptr[] says until we find the column number we're looking for
						 do
						 {
							 if(col_ind[row_ptr[elem.NPK]+l]==elem.KNP)   //Check if we're at the correct position
								 pos=row_ptr[elem.NPK]+l;           //If so, save that position in pos
							 l++;
						 }while(pos<0);

#pragma omp atomic
						 SK[pos] += elem.S[j*mesh.NNPE+k];     //Here is the final global stiffness matrix in symmetric storage
					 }
					 #endif
				 }

			 }                             //End loop over nodes in the element
		 }                                  //End loop over elements
	 }		//End parallel region

     #ifdef STABILITY
     stb.alphaField.deallocate();
     #endif
}

/**Sets up boundary conditions for the simulation.
 *
 */
void ninja::setBoundaryConditions()
{
     //Specify known values of PHI
     //This is done by replacing the particular node equation (row) with all zeros except a "1" on the diagonal of SK[].
     //Then the corresponding row in RHS[] is replaced with the value of the known PHI.
     //The other nodes that are "connected" to the known node need to have their equations adjusted accordingly.
     //This is done by moving the term with the known value to the RHS.

	 int NPK, KNP;
	 int i, j, k, l;


	  bool *isBoundaryNode;
	  isBoundaryNode=new bool[mesh.NUMNP];       //flag to specify if it's a boundary node

	  #pragma omp parallel default(shared) private(i,j,k,l,NPK,KNP)
	  {
	  #pragma omp for
	  for(k=0;k<mesh.nlayers;k++)
      {
           for(i=0;i<input.dem.get_nRows();i++)
           {
                for(j=0;j<input.dem.get_nCols();j++)          //loop over nodes using i,j,k notation
                {
					if(j==0||j==(input.dem.get_nCols()-1)||i==0||i==(input.dem.get_nRows()-1)||k==(mesh.nlayers-1))  //Check the node to see if it is a boundary node
						isBoundaryNode[k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j]=true;
					else
						isBoundaryNode[k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j]=false;
				}
		   }
	  }
	  #pragma omp for
      for(k=0;k<mesh.nlayers;k++)
      {
           for(i=0;i<input.dem.get_nRows();i++)
           {
                for(j=0;j<input.dem.get_nCols();j++)          //loop over nodes using i,j,k notation
                {
                     NPK=k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j;            //NPK is the global row number (also the node # we're on)
                     for(l=row_ptr[NPK];l<row_ptr[NPK+1];l++)     //loop through all non-zero elements for row NPK
                     {
                          KNP=col_ind[l];       //KNP is the global column number we're on
						  if(isBoundaryNode[KNP]==true)
							  SK[l]=0.;              //Set the value on the known PHI's column to zero
						  if(isBoundaryNode[NPK]==true)
						  {
							  if(KNP==NPK)        //Check if we're at the diagonal
								SK[l]=1.;         //If so, set the value to 1
							  else
								SK[l]=0.;              //Set the value on the known PHI's row to zero
						  }
                     }
                     if(isBoundaryNode[NPK]==true)
						RHS[NPK]=0.;    //Phi is zero on "flow through boundaries"
                }
           }
      }
	  }	//end parallel region
	  if(isBoundaryNode)
	  {
		delete[] isBoundaryNode;
		isBoundaryNode=NULL;
	  }

}

/**
 * @brief Computes the scalar concentration volume field.
 *
 */
#ifdef SCALAR
void ninja::computeScalarField()
{
    //element elem(&mesh);
    //for(int i = 0; i < mesh.NUMEL; i++){//Start loop over elements^M

        //elem.node0 = mesh.get_node0(i);  //get the global node number of local node 0 of element i

        //for(int j = 0; j < elem.NUMQPTV; j++){  //Start loop over quadrature points in the element

        //for(int k = 0; k < mesh.NNPE; k++){ // start loop over nodes in element
        for(int k = 0; k < mesh.NUMNP; k++){ // start loop over nodes in mesh

            //elem.NPK=mesh.get_global_node(k, i);  //NPK is the global node number
            //concentration(elem.NPK) = PHI[elem.NPK];
            concentration(k) = PHI[k];
            //cout<<"elem.NPK = "<<elem.NPK<<endl;
            //cout<<"PHI = "<<PHI[elem.NPK]<<endl;

        } // end loop over nodes
        //} // end loop over quad points
    //} // end loop over elements

    // testing: -------------------------------------------------------------------------------

    cout<<"PHI[0] = "<<PHI[0]<<endl;
    cout<<"PHI[1] = "<<PHI[1]<<endl;
    cout<<"PHI[1000] = "<<PHI[1000]<<endl;
    cout<<"PHI[30634] = "<<PHI[30634]<<endl;


    cout<<"C(20,20,1) = "<<concentration(30,20,1)<<endl;
    cout<<"C(12,13,15) = "<<concentration(12,13,15)<<endl;
    cout<<"C(12,13,0) = "<<concentration(12,13,0)<<endl;

    AsciiGrid<double> testGrid;
    testGrid.set_headerData(input.dem);

    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            testGrid(i,j) = concentration(i,j,10);
        }
    }
    testGrid.write_Grid("concentration_10", 2);

    std::string outFilename = "c10.png";
    std::string scalarLegendFilename = "c10_legend";
    std::string legendTitle = "c10";
    std::string legendUnits = "(g/m3)";
    bool writeLegend = FALSE;

    testGrid.replaceNan( -9999.0 );
    testGrid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );

    testGrid.deallocate();
    //end testing-----------------------------------------------------------------------------
}
#endif //SCALAR

/**
 * @brief Computes the u,v,w 3d volume wind field.
 *
 * Calculate @f$u@f$, @f$v@f$, and @f$w@f$ from derivitives of @f$\Phi@f$. Where:
 *
 * @f$ u = u_{0} + \frac{1}{2} * \frac{d\Phi}{dx} @f$
 *
 * @f$ v = v_{0} + \frac{1}{2} * \frac{d\Phi}{dy} @f$
 *
 * @f$ w = w_{0} + \frac{\alpha^{2}}{2} * \frac{d\Phi}{dz} @f$
 *
 * @note Since the derivatives cannot be directly calculated because they are
 * located at the nodal points(the derivatives across element boundaries are
 * discontinuous), another method must be used.  The method used here is that
 * used in Thompson's book on page 228 called "stress smoothing".  It is
 * basically an inverse-distance weighted average from the gauss points of the
 * surrounding cells.
 *
 * This is the result of the @f$ A*x=b @f$ calculation.
 */
void ninja::computeUVWField()
{

     /*-----------------------------------------------------*/
     /*      Calculate u,v, and w from derivatives of PHI   */
     /*                     1         d PHI                 */
     /*     u =  u  +  -----------  * ----                  */
     /*           0     2*alphaH^2     dx                   */
     /*                                                     */
     /*                     1         d PHI                 */
     /*     v =  v  +  -----------  * ----                  */
     /*           0     2*alphaH^2     dy                   */
     /*                                                     */
     /*                     1         d PHI                 */
     /*     w =  w  +  -----------  * ----                  */
     /*           0     2*alphaV^2     dz                   */
     /*                                                     */
     /*     Since the derivatives cannot be directly        */
     /*     calculated because they are located at the      */
     /*     nodal points(the derivatives across element     */
     /*     boundaries are discontinuous), another method   */
     /*     must be used.  The method used here is that     */
     /*     used in Thompson's book on page 228 called      */
     /*     "stress smoothing".  It is basically an inverse-*/
     /*     distance weighted average from the gauss points */
     /*     of the surrounding cells.                       */
     /*-----------------------------------------------------*/

	 int i, j, k;

	 u.allocate(&mesh);           //u is positive toward East
	 v.allocate(&mesh);           //v is positive toward North
	 w.allocate(&mesh);           //w is positive up
	 if(DIAG == NULL)
		 DIAG=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];        //DIAG is the sum of the weights at each nodal point; eventually, dPHI/dx, etc. are divided by this value to get the "smoothed" (or averaged) value of dPHI/dx at each node point

	 for(i=0;i<mesh.NUMNP;i++)                         //Initialize u,v, and w
     {
          u(i)=0.;
          v(i)=0.;
          w(i)=0.;
	    DIAG[i]=0.;
     }

	#pragma omp parallel default(shared) private(i,j,k)
	{

	 element elem(&mesh);

     double DPHIDX, DPHIDY, DPHIDZ;
	 double XJ, YJ, ZJ;
     double wght, XK, YK, ZK;
	 double *uScratch, *vScratch, *wScratch, *DIAGScratch;
	 uScratch=NULL;
	 vScratch=NULL;
	 wScratch=NULL;
	 DIAGScratch=NULL;

	 uScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
	 vScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
	 wScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
	 DIAGScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];

     for(i=0;i<mesh.NUMNP;i++)                         //Initialize scratch u,v, and w
     {
          uScratch[i]=0.;
          vScratch[i]=0.;
          wScratch[i]=0.;
	      DIAGScratch[i]=0.;
     }

	 #pragma omp for
     for(i=0;i<mesh.NUMEL;i++)                  //Start loop over elements
     {
          elem.node0 = mesh.get_node0(i);  //get the global node number of local node 0 of element i
          for(j=0;j<elem.NUMQPTV;j++)             //Start loop over quadrature points in the element
          {

			   DPHIDX=0.0;     //Set DPHI/DX, etc. to zero for the new quad point
               DPHIDY=0.0;
               DPHIDZ=0.0;

			   elem.computeJacobianQuadraturePoint(j, i, XJ, YJ, ZJ);

               //Calculate dN/dx, dN/dy, dN/dz (Remember we're using the transpose of the inverse!)
               for(k=0;k<mesh.NNPE;k++)
               {
                    elem.NPK=mesh.get_global_node(k, i);            //NPK is the global node number

                    DPHIDX=DPHIDX+elem.DNDX[k]*PHI[elem.NPK];       //Calculate the DPHI/DX, etc. for the quad point we are on
                    DPHIDY=DPHIDY+elem.DNDY[k]*PHI[elem.NPK];
                    DPHIDZ=DPHIDZ+elem.DNDZ[k]*PHI[elem.NPK];
               }

               //Now we know DPHI/DX, etc. for quad point j.  We will distribute this inverse distance weighted average to each nodal point for the cell we're on
               for(k=0;k<mesh.NNPE;k++)          //Start loop over nodes in the element
               {                            //Calculate the Jacobian at the quad point
                    elem.NPK=mesh.get_global_node(k, i);            //NPK is the global nodal number

                    XK=mesh.XORD(elem.NPK);            //Coodinates of the nodal point
                    YK=mesh.YORD(elem.NPK);
                    ZK=mesh.ZORD(elem.NPK);

                    wght=std::pow((XK-XJ),2)+std::pow((YK-YJ),2)+std::pow((ZK-ZJ),2);
                    wght=1.0/(std::sqrt(wght));
//c				    #pragma omp critical
				    {
                    uScratch[elem.NPK]=uScratch[elem.NPK]+wght*DPHIDX;   //Here we store the summing values of DPHI/DX, etc. in the u,v,w arrays for later use (to actually calculate u,v,w)
                    vScratch[elem.NPK]=vScratch[elem.NPK]+wght*DPHIDY;
                    wScratch[elem.NPK]=wScratch[elem.NPK]+wght*DPHIDZ;
                    DIAGScratch[elem.NPK]=DIAGScratch[elem.NPK]+wght;     //Store the sum of the weights for the node
					}

               }                             //End loop over nodes in the element


          }                                  //End loop over quadrature points in the element
     }                                       //End loop over elements

	 #pragma omp critical
	 {
     for(i=0;i<mesh.NUMNP;i++)
     {
          u(i) += uScratch[i];      //Dividing by the DIAG[NPK] gives the value of DPHI/DX, etc. (still stored in the u,v,w arrays)
          v(i) += vScratch[i];
          w(i) += wScratch[i];
		  DIAG[i] += DIAGScratch[i];
	 }
	 } //end critical

	 if(uScratch)
	 {
		delete[] uScratch;
		uScratch=NULL;
	 }
	 if(vScratch)
	 {
		delete[] vScratch;
		vScratch=NULL;
	 }
	 if(wScratch)
	 {
		delete[] wScratch;
		wScratch=NULL;
	 }
	 if(DIAGScratch)
	 {
		delete[] DIAGScratch;
		DIAGScratch=NULL;
	 }

     #pragma omp barrier

     double alphaV = 1.0;

     #ifdef STABILITY
     alphaV = 1.0; //should be 1 unless stability parameters are set
     #endif

     #pragma omp for

     for(i=0;i<mesh.NUMNP;i++)
     {
          u(i)=u(i)/DIAG[i];      //Dividing by the DIAG[NPK] gives the value of DPHI/DX, etc. (still stored in the u,v,w arrays)
          v(i)=v(i)/DIAG[i];
          w(i)=w(i)/DIAG[i];

          //Finally, calculate u,v,w

          #ifdef STABILITY
          alphaV = alphaVfield(i); //set alphaV for stability
		  #endif

		  u(i)=u0(i)+1.0/(2.0*alphaH*alphaH)*u(i);         //Remember, dPHI/dx is stored in u
		  v(i)=v0(i)+1.0/(2.0*alphaH*alphaH)*v(i);
		  w(i)=w0(i)+1.0/(2.0*alphaV*alphaV)*w(i);


     }
     }		//end parallel section

     #ifdef STABILITY
     alphaVfield.deallocate();

    // testing
    /*std::string filename;
    AsciiGrid<double> testGrid;
    testGrid.set_headerData(input.dem);
    testGrid.set_noDataValue(-9999.0);

    for(int k = 0; k < mesh.nlayers; k++){
        for(int i = 0; i <mesh.nrows; i++){
            for(int j = 0; j < mesh.ncols; j++ ){
                testGrid(i,j) = u(i,j,k);
                filename = "u" + boost::lexical_cast<std::string>(k);
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
    }
    testGrid.deallocate();*/

     #endif

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
                times = (wxInit->getTimeList(input.ninjaTimeZone));
                dt =  boost::lexical_cast<std::string>(input.ninjaTime);
            }

            FILE *output;

            if(input.outputPointsFilename != "!set"){ //if output file name is specified
                if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){// if it's a wx model run
                    if(input.ninjaTime == times[0]){ //if it's the first time step write headers to new file
                        output = fopen(input.outputPointsFilename.c_str(), "w");
                        if(wxInit->getForecastIdentifier()=="WRF-3D"){ //it's a 2D wx model run
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
                        if(wxInit->getForecastIdentifier()=="WRF-3D"){ //it's a 3D wx model run
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
                    profile.ObukovLength = L(elem_i,elem_j);
                    profile.ABL_height = bl_height(elem_i,elem_j);
                    profile.Roughness = input.surface.Roughness(elem_i,elem_j);
                    profile.Rough_h = input.surface.Rough_h(elem_i,elem_j);
                    profile.Rough_d = input.surface.Rough_d(elem_i,elem_j);
                    profile.AGL = height_above_ground;  // height above the ground

                    elem.get_xyz(elem_num, u_coord, v_coord, 1, x, y, z_temp); // get z at first layer

                    profile.inputWindHeight = z_temp - z_ground - input.surface.Rough_h(elem_i, elem_j); // height above vegetation
                    profile.inputWindSpeed = u.interpolate(x, y, z_temp);

                    new_u = profile.getWindSpeed();
                    profile.inputWindSpeed = v.interpolate(x, y, z_temp);
                    new_v = profile.getWindSpeed();
                    profile.inputWindSpeed = w.interpolate(x, y, z_temp);
                    new_w = profile.getWindSpeed();
                }
                else{//else use linear interpolation
                    new_u = u.interpolate(x,y,z);
                    new_v = v.interpolate(x,y,z);
                    new_w = w.interpolate(x,y,z);
                }

                if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){ //if wx model run

                    if(wxInit->getForecastIdentifier() == "WRF-3D"){
                        fprintf(output,"%s,%lf,%lf,%lf,%s,%lf,%lf,%lf,%lf,%lf,%lf\n", pointName.c_str(), lat, lon, height_above_ground, dt.c_str(),
                                new_u, new_v, new_w, wxInit->u_wxList[i], wxInit->v_wxList[i], wxInit->w_wxList[i]);
                    }
                    else{
                    fprintf(output,"%s,%lf,%lf,%lf,%s,%lf,%lf,%lf,%lf,%lf\n", pointName.c_str(), lat, lon, height_above_ground, dt.c_str(),
                            new_u, new_v, new_w, wxInit->u10List[i], wxInit->v10List[i]);
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
            try_output_u = u.interpolate(elem, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);
            try_output_v = v.interpolate(elem, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);
            try_output_w = w.interpolate(elem, cell_i, cell_j, cell_k, u_loc, v_loc, w_loc);

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
                input.stationsOldInput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldInput[i].set_direction(dir);

                wind_uv_to_sd(old_output_u, old_output_v, &spd, &dir);
                input.stationsOldOutput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldOutput[i].set_direction(dir);

            }else if(u_keep_old==true && v_keep_old==false)
            {
                wind_uv_to_sd(old_input_u, try_input_v, &spd, &dir);
                input.stationsOldInput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldInput[i].set_direction(dir);

                wind_uv_to_sd(old_output_u, try_output_v, &spd, &dir);
                input.stationsOldOutput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldOutput[i].set_direction(dir);

            }else if(u_keep_old==false && v_keep_old==true)
            {
                wind_uv_to_sd(try_input_u, old_input_v, &spd, &dir);
                input.stationsOldInput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldInput[i].set_direction(dir);

                wind_uv_to_sd(try_output_u, old_output_v, &spd, &dir);
                input.stationsOldOutput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldOutput[i].set_direction(dir);

            }else
            {
                wind_uv_to_sd(try_input_u, try_input_v, &spd, &dir);
                input.stationsOldInput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldInput[i].set_direction(dir);

                wind_uv_to_sd(try_output_u, try_output_v, &spd, &dir);
                input.stationsOldOutput[i].set_speed(spd, velocityUnits::metersPerSecond);
                input.stationsOldOutput[i].set_direction(dir);
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
            input.stationsScratch[i].set_speed(spd, velocityUnits::metersPerSecond);
			input.stationsScratch[i].set_direction(dir);
			//input.stationsScratch[i].set_w_speed(new_input_w, velocityUnits::metersPerSecond);
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

void ninja::writeOutputFiles(bool scalarTransportSimulation)
{
    
    set_outputFilenames(mesh.meshResolution, mesh.meshResolutionUnits);

	//Write volume data to VTK format (always in m/s?)
	if(input.volVTKOutFlag)
	{
		try{
			volVTK VTK(u, v, w, mesh.XORD, mesh.YORD, mesh.ZORD, input.dem.get_nCols(), input.dem.get_nRows(), mesh.nlayers, input.volVTKFile);
		}catch (exception& e)
		{
			input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during volume VTK file writing: %s", e.what());
		}catch (...)
		{
			input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during volume VTK file writing: Cannot determine exception type.");
		}
	}


    #ifdef SCALAR
    if(input.scalarTransportFlag == false){ // deallocate if don't need for a scalar transport simulation
        u.deallocate();                 // otherwsie, these are deallocated after scalar equations are discretized
        v.deallocate();
        w.deallocate();
    }

    if(scalarTransportSimulation == true){ //if this iteration through write() is for scalar run
        scalar.deallocate();
    }
    #endif
    #ifndef SCALAR
	u.deallocate();
	v.deallocate();
	w.deallocate();
	#endif

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

			angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.angResolution, AsciiGrid<double>::order0));
			velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.velResolution, AsciiGrid<double>::order0));

			AsciiGrid<double> tempCloud(CloudGrid);
			tempCloud *= 100.0;  //Change to percent, which is what FARSITE needs
			tempCloud.write_Grid(input.cldFile.c_str(), 1);
			angTempGrid->write_Grid(input.angFile.c_str(), 0);
			velTempGrid->write_Grid(input.velFile.c_str(), 2);

			#ifdef FRICTION_VELOCITY
			if(input.frictionVelocityFlag == 1){
                AsciiGrid<double> *ustarTempGrid;
                ustarTempGrid=NULL;

                ustarTempGrid = new AsciiGrid<double> (UstarGrid.resample_Grid(input.velResolution, AsciiGrid<double>::order0));

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

                dustTempGrid = new AsciiGrid<double> (DustGrid.resample_Grid(input.velResolution, AsciiGrid<double>::order0));

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
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during ascii file writing: %s", e.what());
	}catch (...)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during ascii file writing: Cannot determine exception type.");
	}

	}//end omp section


	//write text file comparing measured to simulated winds (measured read from file, filename, etc. hard-coded in function)
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
			    std::vector<boost::local_time::local_date_time> times(wxInit->getTimeList(input.ninjaTimeZone));
			    ninjaKmlFiles.setWxModel(wxInit->getForecastIdentifier(), times[0]);
			}

			if(ninjaKmlFiles.writeKml(input.googSpeedScaling))
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

			angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.shpResolution, AsciiGrid<double>::order0));
			velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.shpResolution, AsciiGrid<double>::order0));

			output.setDirGrid(*angTempGrid);
			output.setSpeedGrid(*velTempGrid);
            output.setDEMfile(input.pdfDEMFileName);
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
			output.setSpeedGrid(VelocityGrid);
			
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
	if(PHI)
	{	delete[] PHI;
		PHI=NULL;
	}
	if(SK)
	{	delete[] SK;
		SK=NULL;
	}
	if(col_ind)
	{	delete[] col_ind;
		col_ind=NULL;
	}
	if(row_ptr)
	{	delete[] row_ptr;
		row_ptr=NULL;
	}
	if(RHS)
	{	delete[] RHS;
		RHS=NULL;
	}
	//if(u)
	//{	delete[] u;
	//	u=NULL;
	//}
	//if(v)
	//{	delete[] v;
	//	v=NULL;
	//}
	//if(w)
	//{	delete[] w;
	//	w=NULL;
	//}
	if(DIAG)
	{	delete[] DIAG;
		DIAG=NULL;
	}
	if(speedInitializationGrid)
	{	delete speedInitializationGrid;
		speedInitializationGrid = NULL;
	}
	if(dirInitializationGrid)
	{	delete dirInitializationGrid;
		dirInitializationGrid = NULL;
	}
	if(uInitializationGrid)
	{	delete uInitializationGrid;
		uInitializationGrid = NULL;
	}
	if(vInitializationGrid)
	{	delete vInitializationGrid;
		vInitializationGrid = NULL;
	}
	if(airTempGrid)
	{	delete airTempGrid;
		airTempGrid = NULL;
	}
	if(cloudCoverGrid)
	{	delete cloudCoverGrid;
		cloudCoverGrid = NULL;
	}

	u0.deallocate();
	v0.deallocate();
	w0.deallocate();
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

#ifdef STABILITY
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
#endif //STABILITY

#ifdef SCALAR
void ninja::set_scalarTransportFlag(bool flag)
{
    input.scalarTransportFlag = flag;
}

void ninja::set_scalarSourceStrength(double strength)
{
    input.scalarSourceStrength = strength;
}

void ninja::set_scalarSourceXcoord(double xCoord)
{
    input.scalarSourceXcoord = xCoord;
}

void ninja::set_scalarSourceYcoord(double yCoord)
{
    input.scalarSourceYcoord = yCoord;
}
#endif //SCALAR

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
    if(meshChoice == WindNinjaInputs::coarse){
        input.meshCount = 100000;
    }
    else if(meshChoice == WindNinjaInputs::medium){
        input.meshCount = 500000;
    }
    else if(meshChoice == WindNinjaInputs::fine){
        input.meshCount = 1e6;
    }
    else{
        throw std::range_error("The mesh resolution choice has been set improperly.");
    }
}
void ninja::set_NonEqBc(bool flag)
{
    input.nonEqBc = flag;
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

WindNinjaInputs::eMeshType ninja::get_eMeshType(std::string meshType)
{
    if(meshType == "MDM"){
        return WindNinjaInputs::MDM;
    }
    else if(meshType == "SHM"){
        return WindNinjaInputs::SHM;
    }
    else{
        throw std::logic_error("Problem with mesh type string in ninja::get_eMeshType().");
    }
}

void ninja::set_MeshType(WindNinjaInputs::eMeshType meshType)
{
    if( (meshType != WindNinjaInputs::MDM) &&
        (meshType != WindNinjaInputs::SHM) )
        throw std::logic_error("Problem with mesh type in ninja::set_MeshType().");

    input.meshType = meshType;
}

void ninja::set_StlFile(std::string stlFile)
{
    input.stlFile = stlFile;
}

#endif


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
//	std::vector<std::string> regions;
//	regions = tz_db.region_list();

    input.ninjaTimeZone = input.tz_db.time_zone_from_region( timeZoneString.c_str() );
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
    input.stations = wxStation::readStationFile(input.wxStationFilename, input.dem.fileName);	//read wxStation(s) info from file
    input.stationsScratch = input.stations;
    input.stationsOldInput = input.stations;
    input.stationsOldOutput = input.stations;
    for (unsigned int i = 0; i < input.stations.size(); i++)
    {
        if (!input.stations[i].check_station())
        {
            throw std::range_error("Error in weather station parameters.");
        }
    }
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
        if (!input.stations[i].check_station())
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

    double lonLat[2];

    if( GDALGetCenter( poDS, lonLat ) )
    {
        set_position(lonLat[1], lonLat[0]);
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
            #ifdef MKL
            mkl_set_num_threads(1);
            #endif //MKL
            //ninjaCom(ninjaComClass::ninjaDebug, "IN OMP PARALLEL REGION AND SHOULDN'T BE!!!!");
        }else{
            //omp_set_nested(true);
            //omp_set_dynamic(true);
            omp_set_num_threads(numberCPUs);
            //#ifdef MKL
            //mkl_set_num_threads(numberCPUs);
            //#endif //MKL
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
    input.shpUnits = units;
    lengthUnits::toBaseUnits(Resolution, units);

    input.pdfResolution = Resolution;
}

void ninja::set_pdfDEM(std::string dem_file_name)
{
    if(!CPLCheckForFile((char*)dem_file_name.c_str(), NULL))
        throw std::runtime_error(std::string("The file ") +
                dem_file_name + " does not exist or may be in use by another program.");
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
        timestream << input.ninjaTime;
#ifdef STABILITY
    else if( input.stabilityFlag == true && input.alphaStability == -1 )
        timestream << input.ninjaTime;
#endif

    std::string pathName;
    if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag )	//prepend directory paths to rootFile for wxModel run
    {
        std::string baseName(CPLGetBasename(input.dem.fileName.c_str()));
        pathName = CPLGetPath(input.forecastFilename.c_str());
        
        //if it's a .tar, write to directory containing the .tar file
        if( strstr(pathName.c_str(), ".tar") ){
            pathName.erase( pathName.rfind("/") );
            rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);
        }
        else{
            rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);
        }
    }else{
        std::string baseName(CPLGetBasename(input.dem.fileName.c_str()));
        pathName = CPLGetPath(input.dem.fileName.c_str());
        rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);
    }

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
    if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
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
    if( input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag )
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
    lengthUnits::fromBaseUnits(kmzResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(shpResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(velResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(pdfResolutionTemp, meshResolutionUnits);

    os << "_" << timeAppend << (long) (meshResolutionTemp+0.5)  << mesh_units;
    os_kmz << "_" << timeAppend << (long) (kmzResolutionTemp+0.5)  << kmz_mesh_units;
    os_shp << "_" << timeAppend << (long) (shpResolutionTemp+0.5)  << shp_mesh_units;
    os_ascii << "_" << timeAppend << (long) (velResolutionTemp+0.5)  << ascii_mesh_units;
    os_pdf << "_" << timeAppend << (long) (pdfResolutionTemp+0.5)    << pdf_mesh_units;

    #ifdef STABILITY
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
    #endif

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
