/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Semi lagrangian solver
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

#include "ninjaSemiLagrangian.h"

NinjaSemiLagrangian::NinjaSemiLagrangian() : ninja()
{
    pszVrtMem = NULL;
    pszGridFilename = NULL;

    boundary_name = "";
    type = "";
    value = "";
    gammavalue = "";
    pvalue = "";
    inletoutletvalue = "";
    template_ = "";
    
    foamRoughness = 0.01; 

    meshResolution = -1.0;
    initialFirstCellHeight = -1.0;
    oldFirstCellHeight = -1.0;
    finalFirstCellHeight = -1.0;
    latestTime = 0;
    cellCount = 0; 
    simpleFoamEndTime = 1000; //initial value in controlDict_simpleFoam

    startTotal = 0.0;
    endTotal = 0.0;
    startMesh = 0.0;
    endMesh = 0.0;
    startInit = 0.0;
    endInit = 0.0;
    startSolve = 0.0;
    endSolve = 0.0;
    startWriteOut = 0.0;
    endWriteOut = 0.0;
    startFoamFileWriting = 0.0;
    endFoamFileWriting = 0.0;
    startOutputSampling = 0.0;
    endOutputSampling = 0.0;
    startStlConversion = 0.0;
    endStlConversion = 0.0;
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

NinjaSemiLagrangian::NinjaSemiLagrangian(NinjaSemiLagrangian const& A ) : ninja(A), U00(A.U00), transport(A.transport)
{

}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

NinjaSemiLagrangian& NinjaSemiLagrangian::operator= (NinjaSemiLagrangian const& A)
{
    if(&A != this) {
        ninja::operator=(A);
        U00 = A.U00;
        transport = A.transport;
    }
    return *this;
}

NinjaSemiLagrangian::~NinjaSemiLagrangian()
{
    deleteDynamicMemory();
}

/**Method to start a wind simulation.
 * This is the method used to start the "number crunching" part of a simulation.
 * WindNinjaInputs should be completely filled in before running this method.
 * @return Returns true if simulation completes without error.
 */
bool NinjaSemiLagrangian::simulate_wind()
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
    if(transport.transportType == TransportSemiLagrangian::settls)
        U00.allocate(&mesh);

#ifdef _OPENMP
    endMesh = omp_get_wtime();
#endif

/*  ----------------------------------------*/
/*  START OUTER INTERATIVE LOOP FOR         */
/*  MATCHING INPUT POINTS		    */
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
    do
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
        FEM.SetStability(mesh, input, U0, CloudGrid, init);
        FEM.Discretize(mesh, input, U0);

        checkCancel();

/*  ----------------------------------------*/
/*  SET BOUNDARY CONDITIONS                 */
/*  ----------------------------------------*/

        //set boundary conditions
        FEM.SetBoundaryConditions(mesh, input);

//#define WRITE_A_B
#ifdef WRITE_A_B	//used for debugging...
        FEM.Write_A_and_b(1000);
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
        if(FEM.Solve(input, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
            if(FEM.SolveMinres(input, mesh.NUMNP, MAXITS, print_iters, stop_tol)==false)
                throw std::runtime_error("Solver returned false.");

#ifdef _OPENMP
        endSolve = omp_get_wtime();
#endif

checkCancel();

/*  ----------------------------------------*/
/*  COMPUTE UVW WIND FIELD                   */
/*  ----------------------------------------*/

        //compute uvw field from phi field
        FEM.ComputeUVWField(mesh, input, U0, U);

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


void NinjaSemiLagrangian::SetOutputFilenames()
{
    //Do file naming string stuff for all output files
    std::string rootFile, rootName, timeAppend, wxModelTimeAppend, fileAppend, kmz_fileAppend, \
        shp_fileAppend, ascii_fileAppend, mesh_units, kmz_mesh_units, \
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

    if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag ){
        timestream << input.ninjaTime;
    }

    std::string pathName;
    std::string baseName(CPLGetBasename(input.dem.fileName.c_str()));

    if(input.customOutputPath == "!set"){
        //prepend directory paths to rootFile for wxModel run
        if( input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag ){
            pathName = CPLGetPath(input.forecastFilename.c_str());
            //if it's a .tar, write to directory containing the .tar file
            if( strstr(pathName.c_str(), ".tar") ){
                pathName.erase( pathName.rfind("/") );
            }
        }else{
            pathName = CPLGetPath(input.dem.fileName.c_str());
        }
    }
    else{
        pathName = input.customOutputPath;
    }
    
    rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);

    /* set the output path member variable */
    input.outputPath = pathName;

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
    mesh_units = "m";
    kmz_mesh_units = lengthUnits::getString( input.kmzUnits );
    shp_mesh_units = lengthUnits::getString( input.shpUnits );
    ascii_mesh_units = lengthUnits::getString( input.velOutputFileDistanceUnits );
    pdf_mesh_units   = lengthUnits::getString( input.pdfUnits );

    ostringstream os, os_kmz, os_shp, os_ascii, os_pdf;

    if( input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag ){
        double tempSpeed = input.inputSpeed;
        velocityUnits::fromBaseUnits(tempSpeed, input.inputSpeedUnits);
        os << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_kmz << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_shp << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_ascii << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
        os_pdf << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
    }

    double meshResolutionTemp = input.dem.get_cellSize();
    double kmzResolutionTemp = input.kmzResolution;
    double shpResolutionTemp = input.shpResolution;
    double velResolutionTemp = input.velResolution;
    double pdfResolutionTemp = input.pdfResolution;

    lengthUnits::eLengthUnits meshResolutionUnits = lengthUnits::meters;

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

    fileAppend = os.str();
    kmz_fileAppend = os_kmz.str();
    shp_fileAppend = os_shp.str();
    ascii_fileAppend = os_ascii.str();
    pdf_fileAppend   = os_pdf.str();

    input.kmlFile = rootFile + kmz_fileAppend + ".kml";
    input.kmzFile = rootFile + kmz_fileAppend + ".kmz";

    input.shpFile = rootFile + shp_fileAppend + ".shp";
    input.dbfFile = rootFile + shp_fileAppend + ".dbf";

    input.pdfFile = rootFile + pdf_fileAppend + ".pdf";

    input.cldFile = rootFile + ascii_fileAppend + "_cld.asc";
    input.velFile = rootFile + ascii_fileAppend + "_vel.asc";
    input.angFile = rootFile + ascii_fileAppend + "_ang.asc";
    input.atmFile = rootFile + ascii_fileAppend + ".atm";

    input.legFile = rootFile + kmz_fileAppend + ".bmp";
    if( input.ninjaTime.is_not_a_date_time() )	//date and time not set?
        input.dateTimeLegFile = "";
    else
        input.dateTimeLegFile = rootFile + kmz_fileAppend + ".date_time" + ".bmp";
}



int NinjaSemiLagrangian::WriteOutputFiles()
{
   
    /*-------------------------------------------------------------------*/
    /* prepare output                                                    */
    /*-------------------------------------------------------------------*/
    
    //Clip off bounding doughnut if desired
    VelocityGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
    AngleGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);

    //change windspeed units back to what is specified by speed units switch
    velocityUnits::fromBaseUnits(VelocityGrid, input.outputSpeedUnits);

    //resample to requested output resolutions
    SetOutputResolution();

    //set up filenames
    SetOutputFilenames();

    /*-------------------------------------------------------------------*/
    /* write output files                                                */
    /*-------------------------------------------------------------------*/

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
                        
                        //Set cloud grid
                        int longEdge = input.dem.get_nRows();
                        if(input.dem.get_nRows() < input.dem.get_nCols())
                            longEdge = input.dem.get_nCols();
                        double tempCloudCover;
                        if(input.cloudCover < 0){
                            tempCloudCover = 0.0;
                        }
                        else{
                            tempCloudCover = input.cloudCover;
                        }

                        CloudGrid.set_headerData(1, 1, input.dem.get_xllCorner(),
                                input.dem.get_yllCorner(), (longEdge * input.dem.cellSize),
                                -9999.0, tempCloudCover, input.dem.prjString);

			AsciiGrid<double> tempCloud(CloudGrid);
			tempCloud *= 100.0;  //Change to percent, which is what FARSITE needs

                        //ensure grids cover original DEM extents for FARSITE
                        tempCloud.BufferGridInPlace();
                        angTempGrid->BufferGridInPlace();
                        velTempGrid->BufferGridInPlace();

			tempCloud.write_Grid(input.cldFile.c_str(), 1);
			angTempGrid->write_Grid(input.angFile.c_str(), 0);
			velTempGrid->write_Grid(input.velFile.c_str(), 2);

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

	//write text file comparing measured to simulated winds (measured read from file, filename, etc. hard-coded in function)
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

	//write shape files
	try{
		if(input.shpOutFlag==true)
		{
			AsciiGrid<double> *velTempGrid, *angTempGrid;
			velTempGrid=NULL;
			angTempGrid=NULL;

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

	//write kmz file
	try{
		if(input.googOutFlag==true)

		{
			AsciiGrid<double> *velTempGrid, *angTempGrid;
			velTempGrid=NULL;
			angTempGrid=NULL;

			KmlVector ninjaKmlFiles;

			angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.kmzResolution, AsciiGrid<double>::order0));
			velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.kmzResolution, AsciiGrid<double>::order0));

			ninjaKmlFiles.setKmlFile(input.kmlFile);
			ninjaKmlFiles.setKmzFile(input.kmzFile);
			ninjaKmlFiles.setDemFile(input.dem.fileName);

			ninjaKmlFiles.setLegendFile(input.legFile);
			ninjaKmlFiles.setDateTimeLegendFile(input.dateTimeLegFile, input.ninjaTime);
			ninjaKmlFiles.setSpeedGrid(*velTempGrid, input.outputSpeedUnits);
			ninjaKmlFiles.setDirGrid(*angTempGrid);

            ninjaKmlFiles.setLineWidth(input.googLineWidth);
			ninjaKmlFiles.setTime(input.ninjaTime);

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

	return NINJA_SUCCESS;
}

/**Deletes allocated dynamic memory.
 *
 */
void ninjaSemiLagrangian::deleteDynamicMemory()
{
    ninja::deleteDynamicMemory();

    U00.deallocate();
}

