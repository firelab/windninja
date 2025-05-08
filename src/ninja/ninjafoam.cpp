/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  OpenFOAM interaction
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#include "ninjafoam.h"

const char* NinjaFoam::pszFoamPath = NULL;

NinjaFoam::NinjaFoam() : ninja()
{
    foamVersion = "";
    
    pszVrtMem = NULL;
    pszGridFilename = NULL;
    pszTurbulenceGridFilename = NULL;

    boundary_name = "";
    type = "";
    value = "";
    gammavalue = "";
    pvalue = "";
    inletoutletvalue = "";
    template_ = "";
    
    meshResolution = -1.0;
    initialFirstCellHeight = -1.0;
    oldFirstCellHeight = -1.0;
    finalFirstCellHeight = -1.0;
    latestTime = 0;
    cellCount = 0; 
    nRoundsRefinement = 0;
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

    writeMassMesh = false;
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

NinjaFoam::NinjaFoam(NinjaFoam const& A ) : ninja(A)
{

}


/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

NinjaFoam& NinjaFoam::operator= (NinjaFoam const& A)
{
    if(&A != this) {
        ninja::operator=(A);
    }
    return *this;
}

NinjaFoam::~NinjaFoam()
{
    CPLFree( (void*)pszVrtMem );
    CPLFree( (void*)pszGridFilename );
    CPLFree( (void*)pszTurbulenceGridFilename );
}

void NinjaFoam::set_meshResolution(double resolution, lengthUnits::eLengthUnits units)
{
    //set mesh resolution, always stored in meters
    if(resolution<0.0)
        throw std::range_error("Mesh resolution out of range in NinjaFoam::set_meshResolution().");

    meshResolutionUnits = units;
    lengthUnits::toBaseUnits(resolution, units);
    meshResolution = resolution;
}

double NinjaFoam::get_meshResolution()
{
    return meshResolution;
}

bool NinjaFoam::simulate_wind()
{
    #ifdef WIN32
    foamVersion = "2.2.0";
    #else
    foamVersion = CPLGetConfigOption("WM_PROJECT_VERSION", "");
    #endif
    CPLDebug("NINJAFOAM", CPLSPrintf("foamVersion = \"%s\"",foamVersion.c_str()));
    
    if(CSLTestBoolean(CPLGetConfigOption("WRITE_TURBULENCE", "FALSE")))
    {
        CPLDebug("NINJAFOAM", "Writing turbulence output...");
        set_writeTurbulenceFlag("true");
    }
    
    if(input.writeTurbulence == true)
    {
        std::string found_colHeightAGL_str = CPLGetConfigOption("COLMAX_HEIGHT_AGL", "");
        // if read it, but no value, don't want a 0.0 put in, leave it as default value
        // Well at least it didn't break with a 0.0, just grabbed surf values, but still, want it as the default value unless they specify it with an actual value
        if( found_colHeightAGL_str != "" )
        {
            double found_colHeightAGL = atof(found_colHeightAGL_str.c_str());
            std::string found_colHeightAGL_units = CPLGetConfigOption("COLMAX_HEIGHT_AGL_UNITS", "m");
            if( found_colHeightAGL_units == "" ){
                found_colHeightAGL_units = "m";  // default value if not set
            }
            std::cout << "found CPL config option COLMAX_HEIGHT_AGL, setting colMax_colHeightAGL to " << found_colHeightAGL << " " << found_colHeightAGL_units << std::endl;
            set_colMaxSampleHeightAGL( found_colHeightAGL, lengthUnits::getUnit(found_colHeightAGL_units) );
        }
        CPLDebug("NINJAFOAM", "Writing turbulence colMax output, using colHeightAGL %f %s",input.colMax_colHeightAGL,lengthUnits::getString(input.colMax_colHeightAGL_units).c_str());
    }
    
    
    if( input.volVTKOutFlag == true || input.writeTurbulence == true )
    {
        writeMassMesh = true;
    }


    #ifdef _OPENMP
    startTotal = omp_get_wtime();
    #endif

    checkCancel();

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reading elevation file...");

    readInputFile();
    set_position();

    SetMeshResolutionAndResampleDem();

    checkInputs();

    /* 
     * if it's a domainAverageInitialization, set the boundary layer information
     * which is needed for interpolation of the output wind to the user-requested
     * output height
     */
    if(input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag)
    {
        init.reset(initializationFactory::makeInitialization(input));
        init->ninjaFoamInitializeFields(input, CloudGrid);
    }

    /* 
     * if it's a wxModelInitialization, get the average speed,
     * direction, T, cloud cover from the wx model
     */
    if(input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
    {
        init.reset(initializationFactory::makeInitialization(input));
        init->ninjaFoamInitializeFields(input, CloudGrid);
    }
    /* 
     * if it's a griddedInitialization, get the average speed and
     * direction from the input grids 
     */
    if(input.initializationMethod == WindNinjaInputs::griddedInitializationFlag)
    {
        init.reset(initializationFactory::makeInitialization(input));
        init->ninjaFoamInitializeFields(input, CloudGrid);
    }

    if(!input.ninjaTime.is_not_a_date_time())
    {
        std::ostringstream out;
        out << "Simulation time is " << input.ninjaTime;
        input.Com->ninjaCom(ninjaComClass::ninjaNone, out.str().c_str());
    }

    ComputeDirection(); //convert wind direction to unit vector notation
    SetInlets();
    SetBcs();

    input.meshCount = atoi(CPLGetConfigOption("NINJAFOAM_MESH_COUNT", CPLSPrintf("%d",input.meshCount)));
    input.nIterations = atoi(CPLGetConfigOption("NINJAFOAM_ITERATIONS", CPLSPrintf("%d",input.nIterations)));
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);

    CPLDebug("NINJAFOAM", "meshCount = %d", input.meshCount);
    CPLDebug("NINJAFOAM", "Rd = %lf", input.surface.Rough_d(0,0));
    CPLDebug("NINJAFOAM", "z0 = %lf", input.surface.Roughness(0,0));
    CPLDebug("NINJAFOAM", "input wind height = %lf", input.inputWindHeight);
    CPLDebug("NINJAFOAM", "input speed = %lf", input.inputSpeed);
    CPLDebug("NINJAFOAM", "input direction = %lf", input.inputDirection);
    CPLDebug("NINJAFOAM", "foam direction = (%lf, %lf, %lf)", direction[0], direction[1], direction[2]);
    CPLDebug("NINJAFOAM", "number of inlets = %ld", inlets.size());
    CPLDebug("NINJAFOAM", "Roughness = %f", input.surface.Roughness.get_meanValue());
    CPLDebug("NINJAFOAM", "Rough_d = %f", input.surface.Rough_d.get_meanValue());
    CPLDebug("NINJAFOAM", "Rough_h = %f", input.surface.Rough_h.get_meanValue());
    CPLDebug("NINJAFOAM", "input.nIterations = %d", input.nIterations);

    // start the foam file writing, run, and sample processes, where if it fails at any point, then restart and try smoothing the dem
    // Don't smooth existing cases, just throw an error, as we don't want to edit the existing case mesh files.

    bool solutionStatus = false;

    int nTries = 3;
    std::string found_nTries_str = CPLGetConfigOption("SIMPLEFOAM_NTRIES", "");
    if( found_nTries_str != "" )
    {
        nTries = atof(found_nTries_str.c_str());
    }
    CPLDebug("NINJAFOAM", "simpleFoam nTries = %d",nTries);

    double startRestartVal;
    double endRestartVal;

    // try 0 is the initial try, 1 to nTries are dem smoothing attempts
    int tryIdx = 0;
    while( solutionStatus == false && tryIdx <= nTries )
    {
        #ifdef _OPENMP
        startRestartVal = omp_get_wtime();
        startRestart.push_back( startRestartVal );
        #endif

        /*------------------------------------------*/
        /*  write OpenFOAM files                    */
        /*------------------------------------------*/

        // if pszFoamPath is not valid, create a new case
        if(!CheckForValidCaseDir(pszFoamPath)){
            GenerateNewCase();
        }
        else{ // otherwise, we're just updating an existing case
            UpdateExistingCase();

            // the mesh is re-used so just re-set the meshing timers
            #ifdef _OPENMP
            startMesh = omp_get_wtime();
            endMesh = omp_get_wtime();
            #endif
        }

        /*-------------------------------------------------------------------*/
        /* Apply initial conditions                                          */
        /*-------------------------------------------------------------------*/

        #ifdef _OPENMP
        startInit = omp_get_wtime();
        #endif

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Applying initial conditions...");
        ApplyInit();

        checkCancel();

        /*-------------------------------------------------------------------*/
        /* Solve for the flow field                                          */
        /*-------------------------------------------------------------------*/

        #ifdef _OPENMP
        endInit = omp_get_wtime();
        startSolve = omp_get_wtime();
        #endif

        if(input.numberCPUs > 1){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel flow calculations...");
            DecomposePar();
        }

        checkCancel();

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solving for the flow field...");
        // skip and go directly to sampling from the initial conditions case directory if a zero input wind speed case
        if( input.inputSpeed != 0.0 )
        {
            solutionStatus = SimpleFoam();
            if( solutionStatus == false )
            {
                if(input.existingCaseDirectory != "!set"){
                    // no smoothing of the dem if this is an existing case
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam(). Can't generate new mesh from smoothed elevation file "
                            "for existing case directory. Try again without using an existing case.");
                    return false;
                }
                // try smoothing the dem, starting at a smoothDist of 1, incrementing the smoothDist by 1 every 2 tries,
                // regardless of the value set during smoothing if the dem was manually smoothed from the start
                tryIdx++;
                if( tryIdx <= nTries )
                {
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam(). Smoothing elevation file and starting over with new mesh...");
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "simpleFoam tryIdx = %d",tryIdx);
                } else
                {
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam()...");
                    break;
                }

                int smoothDist = int((tryIdx+1)/2);  // increments every other tryIdx
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Smoothing elevation file, smoothDist = %d",smoothDist);
                input.dem.smooth_elevation(smoothDist);

                CPLDebug("NINJAFOAM", "unlinking %s", CPLSPrintf( "%s", pszFoamPath ));
                NinjaUnlinkTree( CPLSPrintf( "%s", pszFoamPath ) );
                CPLDebug("NINJAFOAM", "generating new NINJAFOAM directory");
                // force temp dir to DEM location
                CPLSetConfigOption("CPL_TMPDIR", CPLGetDirname(input.dem.fileName.c_str()));
                CPLSetConfigOption("CPLTMPDIR", CPLGetDirname(input.dem.fileName.c_str()));
                CPLSetConfigOption("TEMP", CPLGetDirname(input.dem.fileName.c_str()));
                int retval = GenerateFoamDirectory(input.dem.fileName);
                if(retval != 0){
                    throw std::runtime_error("Error generating the NINJAFOAM directory.");
                }

                // reset to original values
                latestTime = 0;
                CPLDebug("NINJAFOAM", "stepping back to time = %d", latestTime);
                // very important, without this the simulations go on past 300 iterations for simpleFoam
                simpleFoamEndTime = 1000;
                // thankfully, the meshResolution remains unchanged as just firstCellHeight was altered at each previous step
                CPLDebug("NINJAFOAM", "meshResolution= %f", meshResolution);

                #ifdef _OPENMP
                endRestartVal = omp_get_wtime();
                endRestart.push_back( endRestartVal );
                #endif

                continue;

            }  // if(!SimpleFoam())
        }  // if( input.inputSpeed != 0 )
        CPLDebug("NINJAFOAM", "meshResolution= %f", meshResolution);

        if(input.numberCPUs > 1){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");
                ReconstructPar();
        }

        checkCancel();

        /*-------------------------------------------------------------------*/
        /* Sample at requested output height                                 */
        /*-------------------------------------------------------------------*/

        #ifdef _OPENMP
        endSolve = omp_get_wtime();
        startOutputSampling = omp_get_wtime();
        #endif

        //Update the sampleDict interpolation scheme. If the the output wind height is not
        //resolved (if we are sampling in the lowest cell), then we will use a log
        //interpolation from the cell-center value in the lowest cell in the mesh. Otherwise,
        //we use cellPoint for a linear interpolation.
        std::string scheme;
        if(CheckIfOutputWindHeightIsResolved()){
            scheme = "cellPoint";
        }
        else{
            scheme = "cell";
        }
        const char *pszInput;
        const char *pszOutput;
        if ( foamVersion == "2.2.0" ) {
            pszInput = CPLFormFilename(pszFoamPath, "system/sampleDict", "");
            pszOutput = CPLFormFilename(pszFoamPath, "system/sampleDict", "");
        } else {
            pszInput = CPLFormFilename(pszFoamPath, "system/surfaces", "");
            pszOutput = CPLFormFilename(pszFoamPath, "system/surfaces", "");
        }
        CopyFile(pszInput, pszOutput, "$interpolationScheme$", scheme);

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Sampling at requested output height...");
        //suppress libXML warnings
        CPLPushErrorHandler(CPLQuietErrorHandler);
        Sample();
        solutionStatus = SampleRawOutput();
        if( solutionStatus == false )
        {
            if(input.existingCaseDirectory != "!set"){
                // no smoothing of the dem if this is an existing case
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during SampleRawOutput(). Can't generate new mesh from smoothed elevation file "
                        "for existing case directory. Try again without using an existing case.");
            }
            // try smoothing the dem, starting at a smoothDist of 1, incrementing the smoothDist by 1 every 2 tries,
            // regardless of the value set during smoothing if the dem was manually smoothed from the start
            tryIdx++;
            if( tryIdx <= nTries )
            {
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during SampleRawOutput(). Smoothing elevation file and starting over with new mesh...");
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "simpleFoam tryIdx = %d",tryIdx);
            } else
            {
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during SampleRawOutput()...");
                break;
            }

            int smoothDist = int((tryIdx+1)/2);  // increments every other tryIdx
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Smoothing elevation file, smoothDist = %d",smoothDist);
            input.dem.smooth_elevation(smoothDist);

            CPLDebug("NINJAFOAM", "unlinking %s", CPLSPrintf( "%s", pszFoamPath ));
            NinjaUnlinkTree( CPLSPrintf( "%s", pszFoamPath ) );
            CPLDebug("NINJAFOAM", "generating new NINJAFOAM directory");
            // force temp dir to DEM location
            CPLSetConfigOption("CPL_TMPDIR", CPLGetDirname(input.dem.fileName.c_str()));
            CPLSetConfigOption("CPLTMPDIR", CPLGetDirname(input.dem.fileName.c_str()));
            CPLSetConfigOption("TEMP", CPLGetDirname(input.dem.fileName.c_str()));
            int retval = GenerateFoamDirectory(input.dem.fileName);
            if(retval != 0){
                throw std::runtime_error("Error generating the NINJAFOAM directory.");
            }

            // reset to original values
            latestTime = 0;
            CPLDebug("NINJAFOAM", "stepping back to time = %d", latestTime);
            // very important, without this the simulations go on past 300 iterations for simpleFoam
            simpleFoamEndTime = 1000;
            // thankfully, the meshResolution remains unchanged as just firstCellHeight was altered at each previous step
            CPLDebug("NINJAFOAM", "meshResolution= %f", meshResolution);

            #ifdef _OPENMP
            endRestartVal = omp_get_wtime();
            endRestart.push_back( endRestartVal );
            #endif

            continue;

        }  // if(!SampleRawOutput())
        CPLPopErrorHandler();

        #ifdef _OPENMP
        endOutputSampling = omp_get_wtime();
        #endif

        /*-------------------------------------------------------------------*/
        /* Generate and Sample mass mesh                                     */
        /*-------------------------------------------------------------------*/
        #ifdef _OPENMP
        startGenerateAndSampleMassMesh = omp_get_wtime();
        #endif
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating and sampling mass mesh...");
        GenerateAndSampleMassMesh();
        #ifdef _OPENMP
        endGenerateAndSampleMassMesh = omp_get_wtime();
        #endif
    
        // update the tryIdx for the next loop
        tryIdx++;

    } // while( solutionStatus == false && tryIdx <= nTries )
    if( solutionStatus == false ){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam() or other foam processes (sampling foam files). The flow solution failed.");
        return false;
    }

    /*----------------------------------------*/
    /*  write output files                    */
    /*----------------------------------------*/

    #ifdef _OPENMP
    startWriteOut = omp_get_wtime();
    #endif

    if(input.diurnalWinds == false){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing output files...");
        WriteOutputFiles();
    }
           
    #ifdef _OPENMP
    endWriteOut = omp_get_wtime();
    endTotal = omp_get_wtime();
    #endif

    /*----------------------------------------*/
    /*  wrap up                               */
    /*----------------------------------------*/

    #ifdef _OPENMP
    for( int t = 0; t < endRestart.size(); t++ )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "try %d time was %lf seconds.", t, endRestart[t]-startRestart[t]);
    }
    if( endRestart.size() > 0 )
    {
        int t = endRestart.size();
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "try %d time was %lf seconds.", t, endTotal-startRestart[t]);
    }
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "File writing time was %lf seconds.", endFoamFileWriting-startFoamFileWriting);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "STL conversion time was %lf seconds.", endStlConversion-startStlConversion);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Meshing time was %lf seconds.",endMesh-startMesh);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initialization time was %lf seconds.",endInit-startInit);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solver time was %lf seconds.",endSolve-startSolve);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Output sampling time was %lf seconds.", endOutputSampling-startOutputSampling);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "generate and sample mass mesh time was %lf seconds.", endGenerateAndSampleMassMesh-startGenerateAndSampleMassMesh);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Output writing time was %lf seconds.",endWriteOut-startWriteOut);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Total simulation time was %lf seconds.",endTotal-startTotal);
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d done!", input.inputsRunNumber);

    if(!input.keepOutGridsInMemory && input.diurnalWinds == false)
    {
       CloudGrid.deallocate();
       AngleGrid.deallocate();
       VelocityGrid.deallocate();
       TurbulenceGrid.deallocate();
       colMaxGrid.deallocate();

       massMesh_u.deallocate();
	   massMesh_v.deallocate();
	   massMesh_w.deallocate();
       massMesh_k.deallocate();
    }

    if(input.diurnalWinds == true){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Adding diurnal winds...");
    }   

    return true;
}

void NinjaFoam::AddBcBlock(std::string &dataString)
{
    const char *pszPath =  CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    const char *pszTemplateFile;
    const char *pszPathToFile;

    if(template_ == ""){
        if(gammavalue != ""){
            if ( foamVersion == "2.2.0" ) {
                pszPathToFile = CPLSPrintf("ninjafoam/2.2.0/0/%s", "genericTypeVal.tmp");
            } else {
                pszPathToFile = CPLSPrintf("ninjafoam/8/0/%s", "genericTypeVal.tmp");
            }
        }
        else if(inletoutletvalue != ""){
            if ( foamVersion == "2.2.0" ) {
                pszPathToFile = CPLSPrintf("ninjafoam/2.2.0/0/%s", "genericType.tmp");
            } else {
                pszPathToFile = CPLSPrintf("ninjafoam/8/0/%s", "genericType.tmp");
            }
        }
        else{
            if ( foamVersion == "2.2.0" ) {
                pszPathToFile = CPLSPrintf("ninjafoam/2.2.0/0/%s", "genericType-kep.tmp");
            } else {
                pszPathToFile = CPLSPrintf("ninjafoam/8/0/%s", "genericType-kep.tmp");
            }
        }
    }
    else{
        if ( foamVersion == "2.2.0" ) {
            pszPathToFile = CPLSPrintf("ninjafoam/2.2.0/0/%s", template_.c_str());
        } else {
            pszPathToFile = CPLSPrintf("ninjafoam/8/0/%s", template_.c_str());
        }
    }

    pszTemplateFile = CPLFormFilename(pszPath, pszPathToFile, "");

    char *data;
    VSILFILE *fin;
    fin = VSIFOpenL( pszTemplateFile, "r" );

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin); //read in the template file
    data[offset] = '\0';

    std::string s(data);

    ReplaceKeys(s, "$boundary_name$", boundary_name);
    ReplaceKeys(s, "$type$", type);
    ReplaceKeys(s, "$value$", value);
    ReplaceKeys(s, "$gammavalue$", gammavalue);
    ReplaceKeys(s, "$pvalue$", pvalue);
    ReplaceKeys(s, "$U_freestream$", boost::lexical_cast<std::string>(input.inputSpeed));
    ReplaceKeys(s, "$direction$", CPLSPrintf("(%.4lf %.4lf %.4lf)", direction[0],
                                                              direction[1],
                                                              direction[2]));
    ReplaceKeys(s, "$InputWindHeight$", boost::lexical_cast<std::string>(input.inputWindHeight)); //input wind height in ninjafoam mesh is always height above canopy
    ReplaceKeys(s, "$z0$", boost::lexical_cast<std::string>( input.surface.Roughness.get_meanValue() ));
    ReplaceKeys(s, "$Rd$", boost::lexical_cast<std::string>( input.surface.Rough_d.get_meanValue() ));
    ReplaceKeys(s, "$inletoutletvalue$", inletoutletvalue);

    dataString.append(s);

    CPLFree(data);
    VSIFCloseL(fin);
}

void NinjaFoam::WriteZeroFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
{
    int pos;
    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin); //read in full template file
    data[offset] = '\0';

    // write to first dictionary value
    std::string dataString;
    std::string s(data);
    pos = s.find("$boundaryField$");
    if(pos != s.npos){
        s.erase(pos);
        dataString.append(s);
    }

    // add boundary field values from .tmp files
    if(std::string(pszFilename) == "p"){
        WritePBoundaryField(dataString);
    }

    if(std::string(pszFilename) == "U"){
        WriteUBoundaryField(dataString);
    }

    if(std::string(pszFilename) == "k"){
        WriteKBoundaryField(dataString);
    }

    if(std::string(pszFilename) == "epsilon"){
        WriteEpsilonBoundaryField(dataString);
    }

    // writing remaining fields from template file
    s = data;
    pos = s.find("$boundaryField$");
    int len = std::string("$boundaryField$").length();
    if(pos != s.npos){
        s.erase(0, pos+len);
    }

    else if(std::string(pszFilename) == "nut"){
        ReplaceKeys(s, "$z0$", boost::lexical_cast<std::string>( input.surface.Roughness.get_meanValue() ));
    }

    dataString.append(s);

    const char * d = dataString.c_str();
    int nSize = strlen(d);

    VSIFWriteL( d, nSize, 1, fout );

    CPLFree(data);

    VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
    VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()
}

void NinjaFoam::WriteSystemFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
{
    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin); //read in full template file
    data[offset] = '\0';

    std::string s(data);

    if(std::string(pszFilename) == "decomposeParDict"){
        ReplaceKeys(s, "$nProc$", boost::lexical_cast<std::string>(input.numberCPUs));
        const char * d = s.c_str();
        int nSize = strlen(d);
        VSIFWriteL(d, nSize, 1, fout);
    }
    else if(std::string(pszFilename) == "sampleDict" || std::string(pszFilename) == "surfaces"){
        std::string t = NinjaSanitizeString(std::string(CPLGetBasename(input.dem.fileName.c_str())));
        t += "_out.stl";
        ReplaceKeys(s, "$stlFileName$", t);
        const char * d = s.c_str();
        int nSize = strlen(d);
        VSIFWriteL(d, nSize, 1, fout);
    }
    else if(std::string(pszFilename) == "controlDict_simpleFoam"){
        #ifdef WIN32
        ReplaceKeys(s, "$lib$", "libWindNinja");
        #else
        ReplaceKeys(s, "$lib$", "libWindNinja.so");
        #endif
        ReplaceKeys(s, "$nIterations$",boost::lexical_cast<std::string>(input.nIterations));
        const char * d = s.c_str();
        int nSize = strlen(d);
        VSIFWriteL(d, nSize, 1, fout);
    }
    else{
        VSIFWriteL(data, offset, 1, fout);
    }

    CPLFree(data);

    VSIFCloseL(fin); // reopened for each file in writeFoamFiles()
    VSIFCloseL(fout); // reopened for each file in writeFoamFiles()
}

void NinjaFoam::WriteConstantFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
{
    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin); //read in full template file
    data[offset] = '\0';

    VSIFWriteL(data, offset, 1, fout);

    CPLFree(data);

    VSIFCloseL(fin); // reopened for each file in )writeFoamFiles()
    VSIFCloseL(fout); // reopened for each file in writeFoamFiles()
}

void NinjaFoam::WriteFoamFiles()
{
    const char *pszPath;
    const char *pszArchive;
    char **papszFileList;
    std::string osFullPath;
    const char *pszFilename;
    const char *pszOutput;
    const char *pszInput;
    const char *pszTempFoamPath;
    //write temporary OpenFOAM directories
    pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    if ( foamVersion == "2.2.0" ) {
        pszArchive = CPLSPrintf("%s/ninjafoam/2.2.0", pszPath);
    } else {
        pszArchive = CPLSPrintf("%s/ninjafoam/8", pszPath);
    }
    //papszFileList = VSIReadDirRecursive( pszArchive );
    papszFileList = NinjaVSIReadDirRecursive( pszArchive );
    for(int i = 0; i < CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename(papszFileList[i]);
        osFullPath = papszFileList[i];
        if(std::string(pszFilename) == ""){
            pszTempFoamPath = CPLFormFilename(pszFoamPath, osFullPath.c_str(), "");
            VSIMkdir(pszTempFoamPath, 0777);
        }
    }

    //write temporary OpenFOAM files
    VSILFILE *fin;
    VSILFILE *fout;

    for(int i = 0; i < CSLCount( papszFileList ); i++){
        osFullPath = papszFileList[i];
        pszFilename = CPLGetFilename(papszFileList[i]);
        if(std::string(pszFilename) != "" &&
           std::string(CPLGetExtension(pszFilename)) != "tmp" &&
           std::string(pszFilename) != "pointDisplacement"){
            
            pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
            if ( foamVersion == "2.2.0" ) {
                pszArchive = CPLSPrintf("%s/ninjafoam/2.2.0", pszPath);
            } else {
                pszArchive = CPLSPrintf("%s/ninjafoam/8", pszPath);
            }
            
            pszInput = CPLFormFilename(pszArchive, osFullPath.c_str(), "");
            pszOutput = CPLFormFilename(pszFoamPath, osFullPath.c_str(), "");

            fin = VSIFOpenL( pszInput, "r" );
            fout = VSIFOpenL( pszOutput, "w" );

            if( osFullPath.find("0") == 0){
                WriteZeroFiles(fin, fout, pszFilename);
            }
            else if( osFullPath.find("system") == 0 ){
                WriteSystemFiles(fin, fout, pszFilename);
            }
            else if( osFullPath.find("constant") == 0 ){
                WriteConstantFiles(fin, fout, pszFilename);
            }
        }
    }

    CSLDestroy( papszFileList );
}

void NinjaFoam::SetFoamPath(const char* pszPath)
{
    pszFoamPath = pszPath;

}

int NinjaFoam::GenerateFoamDirectory(std::string demName)
{
    std::string t = NinjaSanitizeString(std::string(CPLGetBasename(demName.c_str())));
    pszFoamPath = CPLStrdup(CPLGenerateTempFilename( CPLSPrintf("NINJAFOAM_%s", t.c_str())));
    VSIMkdir( pszFoamPath, 0777 );

    return NINJA_SUCCESS;
}

void NinjaFoam::SetBcs()
{
    bcs.push_back("east_face");
    bcs.push_back("north_face");
    bcs.push_back("south_face");
    bcs.push_back("west_face");
}

void NinjaFoam::SetInlets()
{
    double d = input.inputDirection;
    if(d == 0 || d == 360){
        inlets.push_back("north_face");
    }
    else if(d == 90){
        inlets.push_back("east_face");
    }
    else if(d == 180){
        inlets.push_back("south_face");
    }
    else if(d == 270){
        inlets.push_back("west_face");
    }
    else if(d > 0 && d < 90){
        inlets.push_back("north_face");
        inlets.push_back("east_face");
    }
    else if(d > 90 && d < 180){
        inlets.push_back("east_face");
        inlets.push_back("south_face");
    }
    else if(d > 180 && d < 270){
        inlets.push_back("south_face");
        inlets.push_back("west_face");
    }
    else if(d > 270 && d < 360){
        inlets.push_back("west_face");
        inlets.push_back("north_face");
    }
}

void NinjaFoam::ComputeDirection()
{
    double d, d1, d2, dx, dy; //CW, d1 is first angle, d2 is second angle

    d = input.inputDirection - 180; //convert wind direction from --> wind direction to
    if(d < 0){
        d += 360;
    }

    if(d > 0 && d < 90){ //quadrant 1
        d1 = d;
        d2 = 90 - d;
        dx = sin(d1 * PI/180);
        dy = sin(d2 * PI/180);
    }
    else if(d > 90 && d < 180){ //quadrant 2
        d -= 90;
        d1 = d;
        d2 = 90 - d;
        dx = sin(d2 * PI/180);
        dy = -sin(d1 * PI/180);
    }
    else if(d > 180 && d < 270){ //quadrant 3
        d -= 180;
        d1 = d;
        d2 = 90 - d;
        dx = -sin(d1 * PI/180);
        dy = -sin(d2 * PI/180);
    }
    else if(d > 270 && d < 360){ //quadrant 4
        d -= 270;
        d1 = d;
        d2 = 90 - d;
        dx = -sin(d2 * PI/180);
        dy = sin(d1 * PI/180);
    }
    else if(d == 0 || d == 360){
        dx = 0;
        dy = 1;
    }
    else if(d == 90){
        dx = 1;
        dy = 0;
    }
    else if(d == 180){
        dx = 0;
        dy = -1;
    }
    else if(d == 270){
        dx = -1;
        dy = 0;
    }

    direction.push_back(dx);
    direction.push_back(dy);
    direction.push_back(0);
}

void NinjaFoam::WriteEpsilonBoundaryField(std::string &dataString)
{
    //append BC blocks from template files
    for(int i = 0; i < bcs.size(); i++){
        boundary_name = bcs[i];
        //check if boundary_name is an inlet
        if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
            template_ = "inlet.tmp";
            type = "logProfileDissipationRateInlet";
            value = "";
            gammavalue = "";
            pvalue = "";
            inletoutletvalue = "";
        }
        else{
            template_ = "";
            type = "zeroGradient";
            value = "";
            gammavalue = "";
            pvalue = "";
            inletoutletvalue = "";
        }
        //append BC block for current face
        AddBcBlock(dataString);
    }
}

void NinjaFoam::WriteKBoundaryField(std::string &dataString)
{

    //append BC blocks from template files
    for(int i = 0; i < bcs.size(); i++){
        boundary_name = bcs[i];
        //check if boundary_name is an inlet
        if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
            template_ = "inlet.tmp";
            type = "logProfileTurbulentKineticEnergyInlet";
            value = "";
            gammavalue = "";
            pvalue = "";
            inletoutletvalue = "";
        }
        else{
            template_ = "";
            type = "zeroGradient";
            value = "";
            gammavalue = "";
            pvalue = "";
            inletoutletvalue ="";
        }
        //append BC block for current face
        AddBcBlock(dataString);
    }
}

void NinjaFoam::WritePBoundaryField(std::string &dataString)
{
    //append BC blocks from template files
    for(int i = 0; i < bcs.size(); i++){
        boundary_name = bcs[i];
        //check if boundary_name is an inlet
        if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
            template_ = "";
            type = "zeroGradient";
            value = "";
            gammavalue = "";
            pvalue = "";
            inletoutletvalue = "";
        }
        else{
            template_ = "";
            type = "totalPressure";
            value = "0";
            gammavalue = "1";
            pvalue = "0";
            inletoutletvalue = "";
        }
        //append BC block for current face
        AddBcBlock(dataString);
    }
}

void NinjaFoam::WriteUBoundaryField(std::string &dataString)
{
    //append BC blocks from template files
    for(int i = 0; i < bcs.size(); i++){
        boundary_name = bcs[i];
        //check if boundary_name is an inlet
        if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
            template_ = "inlet.tmp";
            type = "logProfileVelocityInlet";
            value = "";
            gammavalue = "";
            pvalue = "";
            inletoutletvalue = "";
        }
        else{
            template_ = "";
            type = "pressureInletOutletVelocity";
            inletoutletvalue = "(0 0 0)";
            value = "";
            gammavalue = "";
            pvalue = "";
        }
        AddBcBlock(dataString);
    }
}

void NinjaFoam::SetBlockMeshParametersFromDem()
{
    double dz = input.dem.get_maxValue() - input.dem.get_minValue();
    double dx = input.dem.get_xDimension();
    double dy = input.dem.get_yDimension();

    double blockMeshResolution = meshResolution*2*nRoundsRefinement;
    
    int minNumVerticalLayers = 11;
    double blockMeshDz = max((0.1 * max(dx, dy)), (dz + 0.1 * dz));
    blockMeshDz = max(blockMeshDz, minNumVerticalLayers*blockMeshResolution);

    //set blockMesh parameters based on the meshResolution
    //input.dem has already been re-sampled to meshResolution
    //the blockMesh is built directly on top of this re-sampled DEM
    bbox.push_back( input.dem.get_xllCorner() ); //xmin 
    bbox.push_back( input.dem.get_yllCorner() ); //ymin
    bbox.push_back( input.dem.get_maxValue() + 0.05 * blockMeshDz ); //zmin (above highest point in DEM for MDM)
    bbox.push_back( input.dem.get_xllCorner() + input.dem.get_xDimension() ); //xmax
    bbox.push_back( input.dem.get_yllCorner() + input.dem.get_yDimension() ); //ymax
    bbox.push_back( input.dem.get_maxValue() + blockMeshDz ); //zmax

    nCells.push_back(int( (bbox[3] - bbox[0]) / (blockMeshResolution))); // Nx1
    nCells.push_back(int( (bbox[4] - bbox[1]) / (blockMeshResolution))); // Ny1
    nCells.push_back(int( (bbox[5] - bbox[2]) / (blockMeshResolution))); // Nz1

    //we need several cells on all sides of the blockMesh
    //the blockMesh is at least twice as coarse as the refined mesh (meshResolution)
    if(nCells[0] < 10 || nCells[1] < 10 || nCells[2] < 10)
    {
        throw std::runtime_error("The requested mesh resolution is too coarse.");
    }

    initialFirstCellHeight = blockMeshResolution; //height of first cell

    //firstCellheight will be used when decomposing domain for moveDynamicMesh
    CopyFile(CPLFormFilename(pszFoamPath, "0/U", ""), 
            CPLFormFilename(pszFoamPath, "0/U", ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", initialFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/k", ""), 
            CPLFormFilename(pszFoamPath, "0/k", ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", initialFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/epsilon", ""), 
            CPLFormFilename(pszFoamPath, "0/epsilon", ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", initialFirstCellHeight));
    
    CPLDebug("NINJAFOAM", "cellCount = %d", cellCount);
    CPLDebug("NINJAFOAM", "blockMeshDz = %f", blockMeshDz);
    CPLDebug("NINJAFOAM", "firstCellHeight = %f", initialFirstCellHeight);
    
    CPLDebug("NINJAFOAM", "Nx1 = %d", nCells[0]);
    CPLDebug("NINJAFOAM", "Ny1 = %d", nCells[1]);
    CPLDebug("NINJAFOAM", "Nz1 = %d", nCells[2]);
    
    CPLDebug("NINJAFOAM", "xmin = %f", bbox[0]);
    CPLDebug("NINJAFOAM", "ymin = %f", bbox[1]);
    CPLDebug("NINJAFOAM", "zmin = %f", bbox[2]);
    CPLDebug("NINJAFOAM", "xmax = %f", bbox[3]);
    CPLDebug("NINJAFOAM", "ymax = %f", bbox[4]);
    CPLDebug("NINJAFOAM", "zmax = %f", bbox[5]);
}

void NinjaFoam::writeBlockMesh()
{
    const char *pszInput;
    const char *pszOutput;
    const char *pszPath;
    const char *pszArchive;
    double ratio_ = 1.0; //expansion ratio in blockMesh

    SetBlockMeshParametersFromDem();

    pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    if ( foamVersion == "2.2.0" ) {
        pszArchive = CPLSPrintf("%s/ninjafoam/2.2.0", pszPath);
    } else {
        pszArchive = CPLSPrintf("%s/ninjafoam/8", pszPath);
    }

    pszInput = CPLFormFilename(pszArchive, "constant/polyMesh/blockMeshDict", "");
    pszOutput = CPLFormFilename(pszFoamPath, "constant/polyMesh/blockMeshDict", "");

    VSILFILE *fin;
    VSILFILE *fout;

    fin = VSIFOpenL( pszInput, "r" );
    fout = VSIFOpenL( pszOutput, "w" );

    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';

    std::string s(data);
    int pos;
    int len;

    bboxField.push_back("$xmin$");
    bboxField.push_back("$ymin$");
    bboxField.push_back("$zmin$");
    bboxField.push_back("$xmax$");
    bboxField.push_back("$ymax$");
    bboxField.push_back("$zmax$");
     
    cellField.push_back("$Nx1$");
    cellField.push_back("$Ny1$");
    cellField.push_back("$Nz1$");

    for(int i = 0; i<bbox.size(); i++){
        pos = s.find(bboxField[i]);
        len = std::string(bboxField[i]).length();
        while(pos != s.npos){
            std::string t = boost::lexical_cast<std::string>(bbox[i]);
            s.replace(pos, len, t);
            pos = s.find(bboxField[i], pos);
            len = std::string(bboxField[i]).length();
        }
    }
    for(int i = 0; i<nCells.size(); i++){
        pos = s.find(cellField[i]);
        len = std::string(cellField[i]).length();
        while(pos != s.npos){
            std::string t = boost::lexical_cast<std::string>(nCells[i]);
            s.replace(pos, len, t);
            pos = s.find(cellField[i], pos);
            len = std::string(cellField[i]).length();
        }
    }

    ReplaceKeys(s, "$Ratio$", boost::lexical_cast<std::string>(ratio_));

    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);

    CPLFree(data);
    VSIFCloseL(fin);
    VSIFCloseL(fout);
}

void NinjaFoam::writeMoveDynamicMesh()
{
    VSILFILE *fin;
    VSILFILE *fout;

    const char *pszPath;
    const char *pszArchive;
    const char *pszInput;
    const char *pszOutput;

    pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    if ( foamVersion == "2.2.0" ) {
        pszArchive = CPLSPrintf("%s/ninjafoam/2.2.0", pszPath);
    } else {
        pszArchive = CPLSPrintf("%s/ninjafoam/8", pszPath);
    }

    pszInput = CPLFormFilename(pszArchive, "0/pointDisplacement", "");
    pszOutput = CPLFormFilename(pszFoamPath, "0/pointDisplacement", "");

    fin = VSIFOpenL( pszInput, "r" );
    fout = VSIFOpenL( pszOutput, "w" );

    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';

    std::string s(data);

    std::string t = NinjaSanitizeString((std::string(CPLGetBasename(input.dem.fileName.c_str()))));

    ReplaceKeys(s, "$stlName$", t);
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);

    CPLFree(data);
    VSIFCloseL(fin);
    VSIFCloseL(fout);
    
    pszInput = CPLFormFilename(pszFoamPath, "0/pointDisplacement", "");
    pszOutput = CPLFormFilename(pszFoamPath, "0/pointDisplacement", "");
    
    /*
     * Check firstCellHeight in the block mesh. 
     * We have same distance between all layers, since expansionRatio = 1
     * deltaT * velocity must be less than distance between layers, otherwise cells
     * above may move too quickly toward the surface, casuing cells to get turned
     * inside-out. deltaT is set to 1.0 in controlDict.
     */
    double displacementVelocity = 0.5 * initialFirstCellHeight;
    CopyFile(pszInput, pszOutput, "$vx$", CPLSPrintf("%.2f", displacementVelocity));
    CopyFile(pszInput, pszOutput, "$vy$", CPLSPrintf("%.2f", displacementVelocity));
    CopyFile(pszInput, pszOutput, "$vz$", CPLSPrintf("%.2f", displacementVelocity));

    CPLDebug("NINJAFOAM", "firstCellHeight = %f", initialFirstCellHeight);
    CPLDebug("NINJAFOAM", "displacementVelocity = %f", displacementVelocity);
}

/*
 * Replace key k with value v in string s.  Return 1 if value was replaced, 0
 * if the key was not found
 */
int NinjaFoam::ReplaceKey(std::string &s, std::string k, std::string v)
{
    int i, n;
    i = s.find(k);
    if( i != std::string::npos )
    {
        n = k.length();
        s.replace(i, n, v);
        return TRUE;
    }
    else
        return FALSE;
}

int NinjaFoam::ReplaceKeys(std::string &s, std::string k, std::string v, int n)
{
    int rc = FALSE;
    int c = 0;
    do
    {
        rc = ReplaceKey(s, k, v);
        c++;
    } while(rc && c < n);
    return rc;
}

void NinjaFoam::CopyFile(const char *pszInput, const char *pszOutput, std::string key, std::string value)
{
    VSILFILE *fin;
    VSILFILE *fout;

    fin = VSIFOpenL( pszInput, "r" );

    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';

    std::string s(data);
    
    CPLFree(data);
    VSIFCloseL(fin);
    
    if(key != ""){
        ReplaceKeys(s, key, value, 100);
    }

    fout = VSIFOpenL( pszOutput, "w" );
    
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
    
    VSIFCloseL(fout);
}

void NinjaFoam::MoveDynamicMesh()
{
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Running blockMesh...");

    BlockMesh();

    VSILFILE *fout;

    const char *pszInput;
    const char *pszOutput;
    int nRet = -1;
    
    std::string s, ss;

    if(input.numberCPUs > 1){

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel mesh calculations...");

        DecomposePar();

        //re-write controlDict for moveDynamicMesh
        pszInput = CPLFormFilename(pszFoamPath, "system/controlDict_moveDynamicMesh", "");
        pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
        CopyFile(pszInput, pszOutput);

#ifdef WIN32
        const char *const papszArgv[] = { "mpiexec",
                                      "-env",
                                      "MPI_BUFFER_SIZE",
                                      "20000000",
                                      "-n",
                                      CPLSPrintf("%d", input.numberCPUs),
                                      "moveDynamicMesh",
                                      "-case",
                                      pszFoamPath,
                                      "-parallel",
                                      NULL };
#else
        const char *const papszArgv[] = { "mpiexec",
                                      "-np",
                                      CPLSPrintf("%d", input.numberCPUs),
                                      "--allow-run-as-root",  // will need to comment this out for foam 2.2.0 runs
                                      "moveDynamicMesh",
                                      "-case",
                                      pszFoamPath,
                                      "-parallel",
                                      NULL };
#endif

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Running moveDynamicMesh...");

        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);

        char data[PIPE_BUFFER_SIZE + 1];
        int pos, nchar, startPos;

        /* Track progress */
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            checkCancel();
            data[sizeof(data)-1] = '\0';
            //CPLDebug("NINJAFOAM", "moveDynamicMesh: %s", data);
            s.append(data);

            if(s.rfind("GAMG") != s.npos){
                if(s.rfind("Time = ") != s.npos){
                    startPos = s.rfind("GAMG", s.npos);
                    pos = s.rfind("Time = ", startPos);
                    nchar = s.find('\n', pos) - (pos+7);
                    ss = s.substr( (pos+7), nchar );
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(moveDynamicMesh) %.0f%% complete...", atof(ss.c_str())*2);
                }
            }
        }
        
        CPLSpawnAsyncCloseInputFileHandle(sp);
        
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
        if(nRet != 0){
            // will need to comment this out for foam 8 runs on ubuntu 16.04, 
            // where simpleFoam and moveDynamicMesh still run but return code 127 at the command line, and code 32512 here
            throw std::runtime_error("Error during moveDynamicMesh().");
        }

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");

        ReconstructPar();
    }

    else{ // single processor
        //re-write controlDict for moveDynamicMesh
        pszInput = CPLFormFilename(pszFoamPath, "system/controlDict_moveDynamicMesh", "");
        pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
        CopyFile(pszInput, pszOutput);

        const char *const papszArgv[] = { "moveDynamicMesh",
                                          "-case",
                                          pszFoamPath,
                                          NULL };
        
        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);

        char data[PIPE_BUFFER_SIZE + 1];
        int pos;
        int nchar, startPos;

        /* Track progress */
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            data[sizeof(data)-1] = '\0';
            CPLDebug("NINJAFOAM", "moveDynamicMesh: %s", data);
            s.append(data);
            if(s.rfind("GAMG") != s.npos){
                if(s.rfind("Time = ") != s.npos){
                    startPos = s.rfind("GAMG", s.npos);
                    pos = s.rfind("Time = ", startPos);
                    nchar = s.find('\n', pos) - (pos+7);
                    ss = s.substr( (pos+7), nchar );
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(moveDynamicMesh) %.0f%% complete...", atof(ss.c_str())*2);
                }
            }
        }
        
        CPLSpawnAsyncCloseInputFileHandle(sp);
        
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
        if(nRet != 0){
            // will need to comment this out for foam 8 runs on ubuntu 16.04, 
            // where simpleFoam and moveDynamicMesh still run but return code 127 at the command line, and code 32512 here
            throw std::runtime_error("Error during moveDynamicMesh().");
        }
    }
    
    // write moveDynamicMesh stdout to a log file 
    fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.moveDynamicMesh", ""), "w");
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
    VSIFCloseL(fout);
    
    //re-write controlDict for flow
    pszInput = CPLFormFilename(pszFoamPath, "system/controlDict_simpleFoam", "");
    pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    CopyFile(pszInput, pszOutput); 
    
    //update dict files
    latestTime = 50;
    finalFirstCellHeight = initialFirstCellHeight;
    oldFirstCellHeight = finalFirstCellHeight;
    UpdateSimpleFoamControlDict();
    UpdateTimeDirFiles();
}

void NinjaFoam::RefineSurfaceLayer()
{    
    const char *pszInput;
    const char *pszOutput;
    
    //write topoSetDict
    pszInput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");
    pszOutput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");

    //create the minZpatch.stl for the current time, to refine to
    createMinZpatchStl();

    std::string stlName = CPLSPrintf("minZpatch_time%d", latestTime);

    CopyFile(pszInput, pszOutput, "$terrain$", 
            CPLFormFilename(CPLSPrintf("%s/constant/triSurface", pszFoamPath), stlName.c_str(), ""));
    CopyFile(pszInput, pszOutput, "$xout$", CPLSPrintf("%.2f", (bbox[0] + 10)));
    CopyFile(pszInput, pszOutput, "$yout$", CPLSPrintf("%.2f", (bbox[1] + 10)));
    CopyFile(pszInput, pszOutput, "$zout$", CPLSPrintf("%.2f", (bbox[5] - 10)));
    CopyFile(pszInput, pszOutput, "$nearDistance$", CPLSPrintf("%.2f", finalFirstCellHeight)); //refines cells within this distance from the ground
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Refining surface cells in mesh...");
    
    /*----------------------------------------------*/
    /*  refine in all 3 directions                  */
    /*----------------------------------------------*/
    
    //write refineMeshDict for 3-D
    pszInput = CPLFormFilename(pszFoamPath, "system/refineMeshDict_xyz", "");
    pszOutput = CPLFormFilename(pszFoamPath, "system/refineMeshDict", "");
    CopyFile(pszInput, pszOutput);
    
    pszInput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");
    pszOutput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(refineMesh) 10%% complete...");

    CPLDebug("NINJAFOAM", "before refinement, cellCount = %d", cellCount);
    CPLDebug("NINJAFOAM", "target number of cells = %d", input.meshCount);
    
    double percentDone = 0.0;

    for(int i = 0; i < nRoundsRefinement; i++){
        TopoSet();
        RefineMesh();
        
        //update time, near-wall cell height, BC files, topoSetDict file
        latestTime += 1;
        oldFirstCellHeight = finalFirstCellHeight;
        finalFirstCellHeight /= 2.0; //keep track of first cell height
        //meshResolution /= 2.0;
        
        //update dict files
        UpdateSimpleFoamControlDict();
        UpdateTimeDirFiles();

        pszInput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");
        pszOutput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");
        
        CopyFile(pszInput, pszOutput, 
                CPLSPrintf("nearDistance    %.2f", oldFirstCellHeight),
                CPLSPrintf("nearDistance    %.2f", finalFirstCellHeight));
        
        CPLDebug("NINJAFOAM", "finalFirstCellHeght = %f", finalFirstCellHeight);

        percentDone = 100.0 - i/nRoundsRefinement * 100.0;

        if(percentDone < 100.0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "(refineMesh) %.0f%% complete...", percentDone);
        }
    }

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(refineMesh) 99%% complete...");
    
    CPLDebug("NINJAFOAM", "firstCellHeight = %f", initialFirstCellHeight);
    CPLDebug("NINJAFOAM", "finalFirstCellHeight = %f", finalFirstCellHeight);
}

void NinjaFoam::UpdateTimeDirFiles()
{
    /* copy files to latestTime and update firstCellHeight */   
    CopyFile(CPLFormFilename(pszFoamPath, "0/U", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%s/U", boost::lexical_cast<std::string>(latestTime).c_str()),  ""),
            CPLSPrintf("firstCellHeight %.2f;", initialFirstCellHeight),
            CPLSPrintf("firstCellHeight %.2f;", finalFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/k", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%s/k", boost::lexical_cast<std::string>(latestTime).c_str()),  ""),
            CPLSPrintf("firstCellHeight %.2f;", initialFirstCellHeight),
            CPLSPrintf("firstCellHeight %.2f;", finalFirstCellHeight)); 
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/epsilon", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%s/epsilon", boost::lexical_cast<std::string>(latestTime).c_str()),  ""),
            CPLSPrintf("firstCellHeight %.2f;", initialFirstCellHeight),
            CPLSPrintf("firstCellHeight %.2f;", finalFirstCellHeight)); 
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/nut", ""),
            CPLFormFilename(pszFoamPath, CPLSPrintf("%s/nut", boost::lexical_cast<std::string>(latestTime).c_str()),  ""));

    CopyFile(CPLFormFilename(pszFoamPath, "0/p", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%s/p", boost::lexical_cast<std::string>(latestTime).c_str()),  ""));
}

void NinjaFoam::UpdateSimpleFoamControlDict()
{
    //update simpleFoam controlDict endTime
    int oldSimpleFoamEndTime = simpleFoamEndTime; 
    simpleFoamEndTime = latestTime + input.nIterations; //only write final timestep
    CPLDebug("NINJAFOAM", "simpleFoamEndTime = %d", simpleFoamEndTime);
    const char *pszInput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    const char *pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    //update endTime based on latestTime
    CopyFile(pszInput, pszOutput, 
        CPLSPrintf("endTime         %d", oldSimpleFoamEndTime),
        CPLSPrintf("endTime         %d", simpleFoamEndTime));

}

void NinjaFoam::TopoSet()
{
    int nRet = -1;
       
    const char *const papszArgv[] = { "topoSet",
                                    "-case",
                                    pszFoamPath,
                                    "-dict",
                                    "system/topoSetDict",
                                    "-latestTime",
                                    NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.topoSet", ""), "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during topoSet().");

    VSIFCloseL(fout);
}

void NinjaFoam::RefineMesh()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "refineMesh",
                                    "-case",
                                    pszFoamPath,
                                    "-dict",
                                    "system/refineMeshDict", 
                                    NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.refineMesh", ""), "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during refineMesh().");

    VSIFCloseL(fout);
}

void NinjaFoam::BlockMesh()
{
    int nRet = -1;

    const char *const papszArgv[] = { "blockMesh", 
                                    "-case",
                                    pszFoamPath,
                                    NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.blockMesh", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during blockMesh().");

    VSIFCloseL(fout);
}

void NinjaFoam::DecomposePar()
{
    int nRet = -1;

    const char *const papszArgv[] = { "decomposePar", 
                                      "-case",
                                      pszFoamPath,
                                      "-force", 
                                      NULL };
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.decomposePar", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during decomposePar().");

    VSIFCloseL(fout);
}

void NinjaFoam::ReconstructPar()
{
    int nRet = -1;

    const char *const papszArgv[] = { "reconstructPar", 
                                      "-case",
                                      pszFoamPath,
                                      "-latestTime",
                                      NULL };
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.reconstructPar", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during reconstructPar().");

    VSIFCloseL(fout);
}

void NinjaFoam::RenumberMesh()
{
    int nRet = -1;

    const char *const papszArgv[] = { "renumberMesh", 
                                      "-case",
                                      pszFoamPath,
                                      "-latestTime",
                                      "-overwrite", 
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.renumberMesh", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during renumberMesh().");

    VSIFCloseL(fout);
}

void NinjaFoam::ApplyInit()
{
    int nRet = -1;

    const char *const papszArgv[] = { "applyInit", 
                                      "-case",
                                      pszFoamPath,
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.applyInit", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    if(nRet != 0)
        throw std::runtime_error("Error during applyInit().");

    VSIFCloseL(fout);
}

bool NinjaFoam::SimpleFoam()
{
    int nRet = -1;
    
    char data[PIPE_BUFFER_SIZE + 1];
    int pos, startPos;
    std::string s, t;
    double p;

    if(input.numberCPUs > 1){
        #ifdef WIN32
        const char *const papszArgv[] = { "mpiexec",
                                      "-env",
                                      "MPI_BUFFER_SIZE",
                                      "20000000",
                                      "-n",
                                      CPLSPrintf("%d", input.numberCPUs),
                                      "simpleFoam",
                                      "-case",
                                      pszFoamPath,
                                      "-parallel",
                                       NULL };
        #else
        CPLSetConfigOption("MPI_BUFFER_SIZE", "20000000");
        const char *const papszArgv[] = { "mpiexec",
                                      "-np",
                                      CPLSPrintf("%d", input.numberCPUs),
                                      "--allow-run-as-root",  // will need to comment this out for foam 2.2.0 runs
                                      "simpleFoam",
                                      "-case",
                                      pszFoamPath,
                                      "-parallel",
                                       NULL };
        #endif

        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);

        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            checkCancel();
            data[sizeof(data)-1] = '\0';
            s.append(data);
            //CPLDebug("NINJAFOAM", "simpleFoam: %s", data);
            if(s.rfind("smoothSolver") != s.npos){
                startPos = s.rfind("smoothSolver");
                pos = s.rfind("Time = ", startPos);
                if(pos != s.npos && s.npos > (pos + 12) && s.rfind("\n", pos) == (pos-1)){
                    t = s.substr(pos+7, (s.find("\n", pos+7) - (pos+7)));
                    //number of iterations is set equal to the write interval
                    p = atof(t.c_str()) / simpleFoamEndTime * 100 * input.Com->progressWeight;//progress Weight is so that if we do a diurnal sims can contribute to the total progress.
                    input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d", (int)p);
                }
            }
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
        if ( foamVersion == "2.2.0" ) {
            if(nRet != 0)
                return false;
        } else {
            if(nRet != 0){
                // will need to comment this out for foam 8 runs on ubuntu 16.04, 
                // where simpleFoam and moveDynamicMesh still run but return code 127 at the command line, and code 32512 here
                return false;
            }
        }
    }
    else{
        const char *const papszArgv[] = { "simpleFoam",
                                       "-case",
                                       pszFoamPath,
                                       NULL };

        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);

        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            data[sizeof(data)-1] = '\0';
            s.append(data);
            
            if(s.rfind("smoothSolver") != s.npos){
                startPos = s.rfind("smoothSolver");
                pos = s.rfind("Time = ", startPos);
                if(pos != s.npos && s.npos > (pos + 12) && s.rfind("\n", pos) == (pos-1)){
                    t = s.substr(pos+7, (s.find("\n", pos+7) - (pos+7)));
                    //number of iterations is set equal to the write interval
                    p = atof(t.c_str()) / simpleFoamEndTime * 100;
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(solver) %.0f%% complete...", p);
                }
            }
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
        if ( foamVersion == "2.2.0" ) {
            if(nRet != 0)
                return false;
        } else {
            if(nRet != 0){
                // will need to comment this out for foam 8 runs on ubuntu 16.04, 
                // where simpleFoam and moveDynamicMesh still run but return code 127 at the command line, and code 32512 here
                return false;
            }
        }
    }
    
    // write simpleFoam stdout to a log file 
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.simpleFoam", ""), "w");
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
    VSIFCloseL(fout);

    return true;
}

void NinjaFoam::Sample()
{
    int nRet = -1;
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.sample", ""), "w");
    
    if ( foamVersion == "2.2.0" ) {
        
        const char *const papszArgv[] = { "sample", 
                                          "-case",
                                          pszFoamPath,
                                          "-latestTime", 
                                          NULL };
        
        nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
        
    } else {
        
        const char *const papszArgv[] = { "simpleFoam", 
                                          "-case",
                                          pszFoamPath,
                                          "-postProcess",
                                          "-func",
                                          "surfaces",
                                          "-latestTime", 
                                          NULL };
        
        nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
        
    }

    if(nRet != 0)
        throw std::runtime_error("Error during sample().");

    VSIFCloseL(fout);
}

void NinjaFoam::createMinZpatchStl()
{
    int nRet = -1;

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.createMinZpatchStl", ""), "w");

    const char *const papszArgv[] = { "surfaceMeshTriangulate",
                                      "-case",
                                      pszFoamPath,
                                      "-patches",
                                      "\(minZ\)",
                                      CPLSPrintf("constant/triSurface/minZpatch_time%d.stl",latestTime),
                                      NULL };

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE );

    if(nRet != 0)
    {
        VSIFCloseL(fout);
        throw std::runtime_error("Error during createMinZpatchStl().");
    }

    VSIFCloseL(fout);
}

void NinjaFoam::createOutputSurfSampleStl()
{
    //create the minZpatch.stl for the current time, to raise up to the output wind height
    createMinZpatchStl();

    const char *pszMinZpatchStlFileName = CPLStrdup((CPLSPrintf("%s/constant/triSurface/minZpatch_time%d.stl", pszFoamPath, latestTime)));

    std::string demName = NinjaSanitizeString(CPLGetBasename(input.dem.fileName.c_str()));
    const char *pszSurfOutStlFileName = CPLStrdup((CPLSPrintf("%s/constant/triSurface/%s_out.stl", pszFoamPath, demName.c_str())));

    int nRet = -1;

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.createOutputSurfSampleStl", ""), "w");

    const char *const papszArgv[] = { "surfaceTransformPoints",
                                      "-translate",
                                      CPLSPrintf("(0 0 %f)",input.outputWindHeight),
                                      pszMinZpatchStlFileName,
                                      pszSurfOutStlFileName,
                                      NULL };

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE );

    if(nRet != 0)
    {
        VSIFCloseL(fout);
        throw std::runtime_error("Error during createOutputSurfSampleStl().");
    }

    VSIFCloseL(fout);

    // no need to keep the original surface used in the translation
    VSIUnlink(pszMinZpatchStlFileName);

    CPLFree((void*)pszMinZpatchStlFileName);
    CPLFree((void*)pszSurfOutStlFileName);
}

/*
** Sanitize OpenFOAM output so OGR can consume the data using a VRT.
**
** We need to:
**      Open raw output.
**      Remove 1st header line.
**      Remove leading # on next line
**      Remove leading '  '
**      Change 5 '  ' to ','
**      Remove trailing '  '
**
** This essentially copies the data and changes it inline.
**
*/

int NinjaFoam::SanitizeOutput()
{
    /*
    ** Note that fin is a normal FILE used with VSI*, not VSI*L.  This is for
    ** the VSIFGets functions.
    */
    FILE *fin, *fin2;
    //VSILFILE *fin, *fin2;
    //FILE *fout, *fvrt;
    VSILFILE *fout, *fvrt;
    char buf[512];
    char buf2[512];
    int rc;
    const char *pszVrtFile;
    const char *pszVrt;
    const char *pszMem;
    std::string s, s2;

    /*-------------------------------------------------------------------*/
    /* sanitize the u, v, and k output                                       */
    /*-------------------------------------------------------------------*/
    pszMem = CPLSPrintf("%s/output.raw", pszFoamPath);
    /* This is a member, hold on to it so we can read it later */
    pszVrtMem = CPLStrdup(CPLSPrintf("%s/output.vrt", pszFoamPath));

    char **papszOutputSurfacePath;
    papszOutputSurfacePath = VSIReadDir(CPLSPrintf("%s/postProcessing/surfaces/", pszFoamPath));

    for(int i = 0; i < CSLCount( papszOutputSurfacePath ); i++){
        if(std::string(papszOutputSurfacePath[i]) != "." &&
           std::string(papszOutputSurfacePath[i]) != "..") {
            fin = VSIFOpen(CPLSPrintf("%s/postProcessing/surfaces/%s/U_triSurfaceSampling.raw", 
                        pszFoamPath, 
                        papszOutputSurfacePath[i]), "r");
            fin2 = VSIFOpen(CPLSPrintf("%s/postProcessing/surfaces/%s/k_triSurfaceSampling.raw", 
                        pszFoamPath, 
                        papszOutputSurfacePath[i]), "r");
            break;
        }
        else{
            continue;
        }
    }

    fout = VSIFOpenL( pszMem, "w" );
    fvrt = VSIFOpenL( pszVrtMem, "w" );
    if( !fin )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open output file for " \
                                                "reading." );
        return NINJA_E_FILE_IO;
    }
    if( !fin2 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open output file for " \
                                                "reading." );
        return NINJA_E_FILE_IO;
    }
    if( !fout )
    {
        VSIFClose( fin );
        VSIFClose( fin2 );
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open output file for " \
                                                "writing." );
        return NINJA_E_FILE_IO;
    }
    if( !fvrt )
    {
        VSIFClose( fin );
        VSIFClose( fin2 );
        VSIFCloseL( fout );
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open vrt file for " \
                                                "writing." );
        return NINJA_E_FILE_IO;
    }
    pszVrtFile = CPLSPrintf( "CSV:%s", pszMem );

    pszVrt = CPLSPrintf( NINJA_FOAM_OGR_VRT, "output", pszVrtFile, "output" );

    VSIFWriteL( pszVrt, strlen( pszVrt ), 1, fvrt );
    VSIFCloseL( fvrt );
    buf[0] = '\0';
    buf2[0] = '\0';
    /*
    ** eat the first line
    */
    VSIFGets( buf, 512, fin );
    VSIFGets( buf2, 512, fin2 );
    /*
    ** fix the header
    */
    VSIFGets( buf, 512, fin );
    buf[ strcspn( buf, "\n" ) ] = '\0';
    VSIFGets( buf2, 512, fin2 );

    s = buf;
    s2 = buf2;
    ReplaceKeys( s, "#", "", 1 );
    ReplaceKeys( s, "  ", "", 1 );
    ReplaceKeys( s, "  ", ",", 5 );
    ReplaceKeys( s, "  ", "", 1 );
    s = s + ",k\n";
    
    VSIFWriteL( s.c_str(), s.size(), 1, fout );
    /*
    ** sanitize the data.
    */
    char **papszTokens = NULL;
    while( VSIFGets( buf, 512, fin ) != NULL )
    {
        buf[ strcspn( buf, "\n" ) ] = '\0';
        s = buf;
        VSIFGets( buf2, 512, fin2 );
        s2 = buf2;
        ReplaceKeys( s, " ", ",", 5 );
        ReplaceKeys( s2, " ", ",", 5 );
        papszTokens = CSLTokenizeString2( s2.c_str(), ",", 0);
        s = s + "," + std::string(papszTokens[CSLCount( papszTokens )-1]); 
        VSIFWriteL( s.c_str(), s.size(), 1, fout );
    }

    CSLDestroy( papszTokens );
    VSIFClose( fin );
    VSIFClose( fin2 );
    VSIFCloseL( fout );
}

static int TransformGeoToPixelSpace( double *adfInvGeoTransform, double dfX,
                                     double dfY, int *iPixel, int *iLine )
{
    *iPixel = (int) floor( adfInvGeoTransform[0] +
                           adfInvGeoTransform[1] * dfX +
                           adfInvGeoTransform[2] * dfY );
    *iLine  = (int) floor( adfInvGeoTransform[3] +
                           adfInvGeoTransform[4] * dfX +
                           adfInvGeoTransform[5] * dfY );
    return NINJA_SUCCESS;
}

int NinjaFoam::SampleCloud()
{
    int rc;
    OGRDataSourceH hDS = NULL;
    OGRLayerH hLayer = NULL;
    OGRFeatureH hFeature = NULL;
    OGRFeatureDefnH hFeatDefn = NULL;
    OGRGeometryH hGeometry = NULL;
    GDALDatasetH hGriddedDS = NULL;

    double adfGeoTransform[6], adfInvGeoTransform[6];

    hDS = OGROpen( pszVrtMem, FALSE, NULL );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid in memory datasource in NinjaFoam" );
        return NINJA_E_FILE_IO;
    }

    hLayer = OGR_DS_GetLayer( hDS, 0 );
    if( hLayer == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to extract a valid layer for NinjaFoam resampling" );
        return NINJA_E_OTHER;
    }
    double dfX, dfY, dfU, dfV;
    int nPoints, nXSize, nYSize;
    double dfXMax, dfYMax, dfXMin, dfYMin, dfCellSize;

    dfXMin = outputSampleGrid.get_xllCorner();
    dfXMax = outputSampleGrid.get_xllCorner() + outputSampleGrid.get_xDimension();
    dfYMin = outputSampleGrid.get_yllCorner();
    dfYMax = outputSampleGrid.get_yllCorner() + outputSampleGrid.get_yDimension();
    dfCellSize = outputSampleGrid.get_cellSize();

    nPoints = OGR_L_GetFeatureCount( hLayer, TRUE );
    CPLDebug( "WINDNINJA", "NinjaFoam gridding %d points", nPoints );

    /* Get DEM/output specs */
    nXSize = outputSampleGrid.get_nCols();
    nYSize = outputSampleGrid.get_nRows();

    GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
    pszGridFilename = CPLStrdup( CPLSPrintf( "%s/foam.tif", pszFoamPath ) );
    hGriddedDS = GDALCreate( hDriver, pszGridFilename, nXSize, nYSize, 2,
                             GDT_Float64, NULL );
    GDALRasterBandH hUBand, hVBand;
    hUBand = GDALGetRasterBand( hGriddedDS, 1 );
    hVBand = GDALGetRasterBand( hGriddedDS, 2 );
    GDALSetRasterNoDataValue( hUBand, -9999 );
    GDALSetRasterNoDataValue( hVBand, -9999 );

    /* Set the projection from the DEM */
    rc = GDALSetProjection( hGriddedDS, outputSampleGrid.prjString.c_str() );

    adfGeoTransform[0] = dfXMin;
    adfGeoTransform[1] = dfCellSize;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = dfYMax;
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -dfCellSize;

    GDALSetGeoTransform( hGriddedDS, adfGeoTransform );
    rc = GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform );

    int i = 0;
    int nUIndex, nVIndex;
    int nPixel, nLine;
    OGR_L_ResetReading( hLayer );
    hFeatDefn = OGR_L_GetLayerDefn( hLayer );
    nUIndex = OGR_FD_GetFieldIndex( hFeatDefn, "U" );
    nVIndex = OGR_FD_GetFieldIndex( hFeatDefn, "V" );
    while( (hFeature = OGR_L_GetNextFeature( hLayer )) != NULL )
    {
        hGeometry = OGR_F_GetGeometryRef( hFeature );
        dfX = OGR_G_GetX( hGeometry, 0 );
        dfY = OGR_G_GetY( hGeometry, 0 );
        dfU = OGR_F_GetFieldAsDouble( hFeature, nUIndex );
        dfV = OGR_F_GetFieldAsDouble( hFeature, nVIndex );
        TransformGeoToPixelSpace( adfInvGeoTransform, dfX, dfY, &nPixel, &nLine );
        rc = GDALRasterIO( hUBand, GF_Write, nPixel, nLine, 1, 1, &dfU,
                      1, 1, GDT_Float64, 0, 0 );
        rc = GDALRasterIO( hVBand, GF_Write, nPixel, nLine, 1, 1, &dfV,
                      1, 1, GDT_Float64, 0, 0 );
        i++;
    }
    OGR_G_DestroyGeometry( hGeometry );
    OGR_F_Destroy( hFeature );
    OGR_DS_Destroy( hDS );
    GDALClose( hGriddedDS );

    return 0;
}

/*
** Sample a point cloud and create a 2-band GDALDataset of U and V values.
**
** Open the data source created by SanitizeOutput() and read in the point
** features.  We'll then pass this to GDALGrid*() to write data to bands.
**
** TODO: Investigate Grid interp method and options.
**
*/

int NinjaFoam::SampleCloudGrid()
{
    int rc;
    OGRDataSourceH hDS = NULL;
    OGRLayerH hLayer = NULL;
    OGRFeatureH hFeature = NULL;
    OGRFeatureDefnH hFeatDefn = NULL;
    OGRGeometryH hGeometry = NULL;
    GDALDatasetH hGriddedDS = NULL;

    double adfGeoTransform[6];

    hDS = OGROpen( pszVrtMem, FALSE, NULL );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid in memory datasource in NinjaFoam" );
        return NINJA_E_FILE_IO;
    }

    hLayer = OGR_DS_GetLayer( hDS, 0 );
    if( hLayer == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to extract a valid layer for NinjaFoam resampling" );
        return NINJA_E_OTHER;
    }
    double *padfX, *padfY, *padfU, *padfV, *padfK;
    double *padfData;
    int nPoints, nXSize, nYSize;
    double dfXMax, dfYMax, dfXMin, dfYMin, dfCellSize;

    dfXMin = input.dem.get_xllCorner();
    dfXMax = input.dem.get_xllCorner() + input.dem.get_xDimension();
    dfYMin = input.dem.get_yllCorner();
    dfYMax = input.dem.get_yllCorner() + input.dem.get_yDimension();
    dfCellSize = input.dem.get_cellSize();

    nPoints = OGR_L_GetFeatureCount( hLayer, TRUE );
    CPLDebug( "WINDNINJA", "NinjaFoam gridding %d points", nPoints );
    padfX = (double*)CPLMalloc( sizeof( double ) * nPoints );
    padfY = (double*)CPLMalloc( sizeof( double ) * nPoints );
    padfU = (double*)CPLMalloc( sizeof( double ) * nPoints );
    padfV = (double*)CPLMalloc( sizeof( double ) * nPoints );
    padfK = (double*)CPLMalloc( sizeof( double ) * nPoints );

    int i = 0;
    int nUIndex, nVIndex, nKIndex;
    OGR_L_ResetReading( hLayer );
    hFeatDefn = OGR_L_GetLayerDefn( hLayer );
    nUIndex = OGR_FD_GetFieldIndex( hFeatDefn, "U" );
    nVIndex = OGR_FD_GetFieldIndex( hFeatDefn, "V" );
    nKIndex = OGR_FD_GetFieldIndex( hFeatDefn, "K" );
    while( (hFeature = OGR_L_GetNextFeature( hLayer )) != NULL )
    {
        hGeometry = OGR_F_GetGeometryRef( hFeature );
        padfX[i] = OGR_G_GetX( hGeometry, 0 );
        padfY[i] = OGR_G_GetY( hGeometry, 0 );
        padfU[i] = OGR_F_GetFieldAsDouble( hFeature, nUIndex );
        padfV[i] = OGR_F_GetFieldAsDouble( hFeature, nVIndex );
        padfK[i] = OGR_F_GetFieldAsDouble( hFeature, nKIndex );
        i++;
    }

    /* Get DEM/output specs */
    nXSize = input.dem.get_nCols();
    nYSize = input.dem.get_nRows();

    /*
    ** XXX
    ** Nearest neighbour gridding options.  Switch these if you switch the
    ** algorithm.
    ** XXX
    */
    GDALGridNearestNeighborOptions sOptions;
    sOptions.dfRadius1 = 0.;
    sOptions.dfRadius2 = sOptions.dfRadius1;
    sOptions.dfAngle = 0.;
    sOptions.dfNoDataValue = -9999;

    GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
    pszGridFilename = CPLStrdup( CPLSPrintf( "%s/foam.tif", pszFoamPath ) );
    hGriddedDS = GDALCreate( hDriver, pszGridFilename, nXSize, nYSize, 3,
                             GDT_Float64, NULL );
    padfData = (double*)CPLMalloc( sizeof( double ) * nXSize * nYSize );

    /* U field */
    rc = GDALGridCreate( GGA_NearestNeighbor, (void*)&sOptions, nPoints,
                         padfX, padfY, padfU, dfXMin, dfXMax, dfYMax, dfYMin,
                         nXSize, nYSize, GDT_Float64, padfData, NULL, NULL );

    GDALRasterBandH hBand;
    hBand = GDALGetRasterBand( hGriddedDS, 1 );
    GDALSetRasterNoDataValue( hBand, -9999 );
    rc = GDALRasterIO( hBand, GF_Write, 0, 0, nXSize, nYSize, padfData,
                  nXSize, nYSize, GDT_Float64, 0, 0 );

    /* V field */
    rc = GDALGridCreate( GGA_NearestNeighbor, (void*)&sOptions, nPoints,
                         padfX, padfY, padfV, dfXMin, dfXMax, dfYMax, dfYMin,
                         nXSize, nYSize, GDT_Float64, padfData, NULL, NULL );

    hBand = GDALGetRasterBand( hGriddedDS, 2 );
    GDALSetRasterNoDataValue( hBand, -9999 );
    rc = GDALRasterIO( hBand, GF_Write, 0, 0, nXSize, nYSize, padfData,
                  nXSize, nYSize, GDT_Float64, 0, 0 );

    /* Turbulence field */
    rc = GDALGridCreate( GGA_NearestNeighbor, (void*)&sOptions, nPoints,
                         padfX, padfY, padfK, dfXMin, dfXMax, dfYMax, dfYMin,
                         nXSize, nYSize, GDT_Float64, padfData, NULL, NULL );

    hBand = GDALGetRasterBand( hGriddedDS, 3 );
    GDALSetRasterNoDataValue( hBand, -9999 );
    rc = GDALRasterIO( hBand, GF_Write, 0, 0, nXSize, nYSize, padfData,
                  nXSize, nYSize, GDT_Float64, 0, 0 );

    /* Set the projection from the DEM */
    rc = GDALSetProjection( hGriddedDS, input.dem.prjString.c_str() );

    adfGeoTransform[0] = dfXMin;
    adfGeoTransform[1] = dfCellSize;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = dfYMax;
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -dfCellSize;

    GDALSetGeoTransform( hGriddedDS, adfGeoTransform );

    CPLFree( (void*)padfX );
    CPLFree( (void*)padfY );
    CPLFree( (void*)padfU );
    CPLFree( (void*)padfV );
    CPLFree( (void*)padfK );

    CPLFree( (void*)padfData );
    OGR_G_DestroyGeometry( hGeometry );
    OGR_F_Destroy( hFeature );
    OGR_DS_Destroy( hDS );
    GDALClose( hGriddedDS );

    return 0;
}

const char * NinjaFoam::GetGridFilename()
{
    return pszGridFilename;
}

bool NinjaFoam::SampleRawOutput()
{
    /*-------------------------------------------------------------------*/
    /* convert output from xyz to speed and direction                    */
    /*-------------------------------------------------------------------*/

    AsciiGrid<double> foamU, foamV;
    int rc;
    rc = SanitizeOutput();

    rc = SampleCloudGrid();
    //rc = SampleCloud();

    GDALDatasetH hDS;
    hDS = GDALOpen( GetGridFilename(), GA_ReadOnly );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Invalid output written in NinjaFoam::SampleRawOutput");
        return false;
    }

    GDAL2AsciiGrid( (GDALDataset *)hDS, 1, foamU );
    GDAL2AsciiGrid( (GDALDataset *)hDS, 2, foamV );

    if(!CheckIfOutputWindHeightIsResolved()){
        //if the output wind height is not resolved, interpolate to output height using a log profile
        windProfile profile;
        profile.profile_switch = windProfile::monin_obukov_similarity;

        for(int i=0; i<foamU.get_nRows(); i++)
        {
            for(int j=0; j<foamU.get_nCols(); j++)
            {
                profile.ObukovLength = init->L(i,j);
                profile.ABL_height = init->bl_height(i,j);
                profile.Roughness = input.surface.Roughness(i,j);
                profile.Rough_h = input.surface.Rough_h(i,j);
                profile.Rough_d = input.surface.Rough_d(i,j);
                //this is height above the vegetation
                profile.inputWindHeight = finalFirstCellHeight/2.0 + input.surface.Rough_d(i,j) - input.surface.Rough_h(i,j);
 
                //this is height above THE GROUND!! (not "z=0" for the log profile)
                profile.AGL=input.outputWindHeight + input.surface.Rough_h(i,j);

                profile.inputWindSpeed = foamU(i,j);
                foamU(i,j) = profile.getWindSpeed();
                profile.inputWindSpeed = foamV(i,j);
                foamV(i,j) = profile.getWindSpeed();
            }
        }
    }

    AngleGrid = foamU;
    VelocityGrid = foamU;

    for(int i=0; i<foamU.get_nRows(); i++)
    {
        for(int j=0; j<foamU.get_nCols(); j++)
        {
            wind_uv_to_sd(foamU(i,j), foamV(i,j), &(VelocityGrid)(i,j), &(AngleGrid)(i,j));
        }
    }

    // If we failed to fill in the data for the entire grid, we've failed.
    // Report a better message.
    if( AngleGrid.get_hasNoDataValues() || VelocityGrid.get_hasNoDataValues() ) {
        CPLError( CE_Failure, CPLE_AppDefined, "The openfoam output could not be interpolated to a proper surface.");
        return false;
    }
    if(VelocityGrid.get_maxValue() > 220.0){
        CPLError( CE_Failure, CPLE_AppDefined, "The flow solution did not converge. This may occasionally "
                "happen in very complex terrain when the mesh resolution is high. Try the simulation "
                "again with a coarser mesh.");
        return false;
    }

    /*-------------------------------------------------------------------*/
    /* convert k to average velocity fluctuations (u')                   */
    /* u' = sqrt(2/3*k)                                                  */
    /*-------------------------------------------------------------------*/
    AsciiGrid<double> foamK;

    GDAL2AsciiGrid( (GDALDataset *)hDS, 3, foamK );
    TurbulenceGrid = foamK;

    for(int i=0; i<foamK.get_nRows(); i++)
    {
        for(int j=0; j<foamK.get_nCols(); j++)
        {
            TurbulenceGrid(i,j) = std::sqrt(2.0/3.0 * foamK(i,j));
        }
    }
    GDALClose( hDS );

    return true;
}


void NinjaFoam::generateMassMesh()
{
    
    massMesh.buildStandardMesh(input);
    
    
    writeProbeSampleFile( massMesh.XORD, massMesh.YORD, massMesh.ZORD, input.dem.xllCorner, input.dem.yllCorner, input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers );
    
    runProbeSample();
    
    
    massMesh_u.allocate(&massMesh);
    massMesh_v.allocate(&massMesh);
    massMesh_w.allocate(&massMesh);
    readInProbeData( massMesh.XORD, massMesh.YORD, massMesh.ZORD, input.dem.xllCorner, input.dem.yllCorner, input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers, massMesh_u, massMesh_v, massMesh_w );
    
    fillEmptyProbeVals( massMesh.ZORD, input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers, massMesh_u, massMesh_v, massMesh_w );
    
    
    massMesh_k.allocate(&massMesh);
    readInProbeData( massMesh.XORD, massMesh.YORD, massMesh.ZORD, input.dem.xllCorner, input.dem.yllCorner, input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers, massMesh_k );
    
    fillEmptyProbeVals( massMesh.ZORD, input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers, massMesh_k );
    
}

void NinjaFoam::writeProbeSampleFile( const wn_3dArray& x, const wn_3dArray& y, const wn_3dArray& z, 
                                      const double dem_xllCorner, const double dem_yllCorner, 
                                      const int ncols, const int nrows, const int nlayers)
{
    CPLDebug("NINJAFOAM", "writing probes sample file");
    
    const char *probes_filename;
    if ( foamVersion == "2.2.0" ) {
        probes_filename = CPLFormFilename(pszFoamPath, "system/sampleDict_probes", "");
    } else {
        probes_filename = CPLFormFilename(pszFoamPath, "system/probes", "");
    }
    
    FILE *fout;
    
    fout = fopen(probes_filename, "w");
    if( fout == NULL )
        throw std::runtime_error("probes_filename cannot be opened for writing.");
    
    // Write header stuff
    if ( foamVersion == "2.2.0" ) {
        
        fprintf(fout, "/*--------------------------------*- C++ -*----------------------------------*\\\n");
        fprintf(fout, "| =========                 |                                                 |\n");
        fprintf(fout, "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n");
        fprintf(fout, "|  \\\\    /   O peration     | Version:  2.2.0                                 |\n");
        fprintf(fout, "|   \\\\  /    A nd           | Web:      www.OpenFOAM.org                      |\n");
        fprintf(fout, "|    \\\\/     M anipulation  |                                                 |\n");
        fprintf(fout, "\\*---------------------------------------------------------------------------*/\n");
        fprintf(fout, "FoamFile\n");
        fprintf(fout, "{\n");
        fprintf(fout, "    version     2.0;\n");
        fprintf(fout, "    format      ascii;\n");
        fprintf(fout, "    class       dictionary;\n");
        fprintf(fout, "    object      sampleDict;\n");
        fprintf(fout, "}\n");
        fprintf(fout, "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n");
        fprintf(fout, "// run this sample file for the latest time on a case that was run using the simpleFoam solver with the following command:\n");
        fprintf(fout, "//  sample -latestTime\n");
        
    } else {
        
        fprintf(fout, "/*--------------------------------*- C++ -*----------------------------------*\\\n");
        fprintf(fout, "  =========                 |\n");
        fprintf(fout, "  \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox\n");
        fprintf(fout, "   \\\\    /   O peration     | Website:  https://openfoam.org\n");
        fprintf(fout, "    \\\\  /    A nd           | Version:  8\n");
        fprintf(fout, "     \\\\/     M anipulation  |\n");
        fprintf(fout, "-------------------------------------------------------------------------------\n");
        fprintf(fout, "Description\n");
        fprintf(fout, "    Writes out values of fields interpolated to a specified list of points.\n");
        fprintf(fout, "    \n");
        fprintf(fout, "    Found this in /opt/openfoam8/etc/caseDicts/postProcessing/probes/\n");
        fprintf(fout, "    I was originally going to try to use probes and probes.cfg, of type \"probe\", but that wouldn't do interpolation, \n");
        fprintf(fout, "    so this file came from combining the internalProbes and internalProbes.cfg files, which are of type \"sets\", \n");
        fprintf(fout, "    which does the interpolation to the desired points\n");
        fprintf(fout, "    Looks like the base code for this \"points\" set of type \"sets\" is within /opt/openfoam8/src/sampling/sampledSet/points/\n");
        fprintf(fout, "    \n");
        fprintf(fout, "    run this sample file for the latest time on a case that was run using the simpleFoam solver with the following command:\n");
        fprintf(fout, "    simpleFoam -postProcess -func probes -latestTime\n");
        fprintf(fout, "    \n");
        fprintf(fout, "\\*---------------------------------------------------------------------------*/\n");
        
    }
    
    // Write file contents
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "// choice of variables\n");
    fprintf(fout, "fields  (%s %s);\n", "U", "k");
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "// Sampling and I/O settings\n");
    fprintf(fout, "interpolationScheme cellPoint;\n");
    fprintf(fout, "setFormat  raw;\n");
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "type            sets;\n");
    if ( foamVersion != "2.2.0" ) {
        fprintf(fout, "libs            (\"libsampling.so\");\n");
    }
    fprintf(fout, "\n");
    fprintf(fout, "writeControl    writeTime;\n");
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "// list of probe points for windninja mass solver case\n");
    fprintf(fout, "// nrows = %i, ncols = %i, nlayers = %i, xllCorner = %0.20f, yllCorner = %0.20f\n", nrows, ncols, nlayers, dem_xllCorner, dem_yllCorner);
    fprintf(fout, "points\n");
    fprintf(fout, "(\n");
    
    for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
    {
        for(int rowIdx=0; rowIdx<nrows; rowIdx++)
        {
            for(int colIdx=0; colIdx<ncols; colIdx++)
            {
                int ptIdx = layerIdx*nrows*ncols + rowIdx*ncols + colIdx;
                fprintf( fout, "    (%0.20lf %0.20lf %0.20lf)\n",  x(ptIdx)+dem_xllCorner, y(ptIdx)+dem_yllCorner, z(ptIdx)  );
            }
        }
    }
    
    fprintf(fout, ");\n");
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "sets\n");
    fprintf(fout, "(\n");
    fprintf(fout, "    points\n");
    fprintf(fout, "    {\n");
    if ( foamVersion == "2.2.0" ) {
        fprintf(fout, "        type    cloud;\n");
    } else {
        fprintf(fout, "        type    points;\n");
    }
    fprintf(fout, "        axis    xyz;\n");
    fprintf(fout, "        ordered no;\n");
    fprintf(fout, "        points  $points;\n");
    fprintf(fout, "    }\n");
    fprintf(fout, ");\n");
    fprintf(fout, "\n");
    fprintf(fout, "\n");
    fprintf(fout, "// ************************************************************************* //\n");
    
    
    fclose(fout);
    
}

void NinjaFoam::runProbeSample()
{
    CPLDebug("NINJAFOAM", "running probes sample");
    
    int nRet = -1;
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.probeSample", ""), "w");
    
    // increase write precision for this sample
    int oldPrecisionVal = 10;  // /data/ninjafoam/ files have this value set there
    int newPrecisionVal = 20;
    const char *pszInput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    const char *pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    CopyFile(pszInput, pszOutput, 
        CPLSPrintf("writePrecision  %d", oldPrecisionVal),
        CPLSPrintf("writePrecision  %d", newPrecisionVal));
    CopyFile(pszInput, pszOutput, 
        CPLSPrintf("timePrecision   %d", oldPrecisionVal),
        CPLSPrintf("timePrecision   %d", newPrecisionVal));
    
    if ( foamVersion == "2.2.0" ) {
        
        // additional step to swap back and forth between sampleDicts
        const char *pszInput2 = CPLFormFilename(pszFoamPath, "system/sampleDict", "");
        const char *pszOutput2 = CPLFormFilename(pszFoamPath, "system/sampleDict_surfaces", "");
        CopyFile(pszInput2, pszOutput2);
        pszInput2 = CPLFormFilename(pszFoamPath, "system/sampleDict_probes", "");
        pszOutput2 = CPLFormFilename(pszFoamPath, "system/sampleDict", "");
        CopyFile(pszInput2, pszOutput2);
        
        const char *const papszArgv[] = { "sample", 
                                          "-case",
                                          pszFoamPath,
                                          "-latestTime", 
                                          NULL };
        
        nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
        
        // swap back to the original sampleDict names
        pszInput2 = CPLFormFilename(pszFoamPath, "system/sampleDict_surfaces", "");
        pszOutput2 = CPLFormFilename(pszFoamPath, "system/sampleDict", "");
        CopyFile(pszInput2, pszOutput2);
        
    } else {
        
        const char *const papszArgv[] = { "simpleFoam", 
                                          "-case",
                                          pszFoamPath,
                                          "-postProcess",
                                          "-func",
                                          "probes",
                                          "-latestTime", 
                                          NULL };
        
        nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
        
    }
    
    if(nRet != 0)
        throw std::runtime_error("Error during probeSample().");
    
    // sampling is done, go back to old write precision
    CopyFile(pszInput, pszOutput, 
        CPLSPrintf("writePrecision  %d", newPrecisionVal),
        CPLSPrintf("writePrecision  %d", oldPrecisionVal));
    CopyFile(pszInput, pszOutput, 
        CPLSPrintf("timePrecision   %d", newPrecisionVal),
        CPLSPrintf("timePrecision   %d", oldPrecisionVal));
    
    VSIFCloseL(fout);
}

void NinjaFoam::readInProbeData( const wn_3dArray& x, const wn_3dArray& y, const wn_3dArray& z, 
                                 const double dem_xllCorner, const double dem_yllCorner, 
                                 const int ncols, const int nrows, const int nlayers, 
                                 wn_3dScalarField& u, wn_3dScalarField& v, wn_3dScalarField& w )
{
    CPLDebug("NINJAFOAM", "reading in probes sample data");
    
    const char *probesPostProcessDirname = "probes";
    if ( foamVersion == "2.2.0" ) {
        probesPostProcessDirname = "sets";
    }
    
    // method from surfaces sampling, to find the time directory for the surfaces file
    char **papszOutputProbeDataPath;
    papszOutputProbeDataPath = VSIReadDir(CPLSPrintf("%s/postProcessing/%s/", pszFoamPath, probesPostProcessDirname));
    
    const char *probeSampleData_filename;
    const char *timeDir;
    for(int i = 0; i < CSLCount( papszOutputProbeDataPath ); i++){
        if(std::string(papszOutputProbeDataPath[i]) != "." &&
           std::string(papszOutputProbeDataPath[i]) != "..") {
            timeDir = papszOutputProbeDataPath[i];
            probeSampleData_filename = CPLSPrintf("%s/postProcessing/%s/%s/points_U.xy", pszFoamPath, probesPostProcessDirname, timeDir);
            break;
        }
        else{
            continue;
        }
    }
    
    
    // read the full data file into a string separated by "\n" chars for each line
    VSILFILE *fin;
    fin = VSIFOpenL( probeSampleData_filename, "r" );
    
    char *data;
    
    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);
    
    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';
    
    std::string probeSampleData_allLines(data);
    
    CPLFree(data);
    VSIFCloseL( fin );
    
    
    // now go through the data string line by line from where the probe data starts, 
    // comparing the probe data points with the mesh points to detect whether any data points were silently dropped for being outside the mesh
    // filling any dropped probe data with NoData vals to be filled at later steps
    double noDataVal = -9999;
    int startLinePos = 0;  // data starts right at the first character of the file, so no need to set it by finding a specific spot/character in the file
    int endLinePos;
    for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
    {
        for(int rowIdx=0; rowIdx<nrows; rowIdx++)
        {
            for(int colIdx=0; colIdx<ncols; colIdx++)
            {
                int ptIdx = layerIdx*nrows*ncols + rowIdx*ncols + colIdx;
                
                endLinePos = probeSampleData_allLines.find("\n",startLinePos+1);
                
                //// .find() returns -1 when it can't find a value, and YES it lets it try for values past the length of the string with this as the return value. So this happens when EOF
                if ( endLinePos == -1 )
                {
                    // EOF, so it's a data point that was quietly dropped during the sample process
                    
                    double current_ux_pt = noDataVal;
                    double current_uy_pt = noDataVal;
                    double current_uz_pt = noDataVal;
                    
                    u(ptIdx) = current_ux_pt;
                    v(ptIdx) = current_uy_pt;
                    w(ptIdx) = current_uz_pt;
                    
                } else
                {
                    
                    std::string currentLine = probeSampleData_allLines.substr(startLinePos,endLinePos-startLinePos);
                    
                    // now that the current line is found, it is time to process the given line into data values
                    int valStartSpot = 0;  // no white spaces or anything before the first value, which is x
                    int valEndSpot = currentLine.find(" ",valStartSpot);  // man, I got lucky, turns out it was a space followed by a tab between each data value. If it were not always this consistently, would have needed to adjust this part and it would be a great pain
                    std::string current_x_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find(" ",valStartSpot);
                    std::string current_y_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find(" ",valStartSpot);
                    std::string current_z_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find(" ",valStartSpot);
                    std::string current_ux_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find(" ",valStartSpot);
                    std::string current_uy_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find("\n",valStartSpot);  // goes to the end of the line, no whitespace after the value, still want to drop the \n part though
                    std::string current_uz_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot-1); // -1 to drop the \n part
                    
                    double current_x_pt = atof(current_x_pt_str.c_str());
                    double current_y_pt = atof(current_y_pt_str.c_str());
                    double current_z_pt = atof(current_z_pt_str.c_str());
                    double current_ux_pt = atof(current_ux_pt_str.c_str());
                    double current_uy_pt = atof(current_uy_pt_str.c_str());
                    double current_uz_pt = atof(current_uz_pt_str.c_str());
                    
                    
                    // now to compare the found data points with the current mesh points, 
                    // if they match then can store the datapoint and increment to go to the next line of probe sample data, 
                    // otherwise OpenFOAM silently dropped the datapoint and NoDataVal needs stored instead, wait to go to the next line of probe sample data
                    // need to add back in xllCorner, yllCorner to make the points consistent between the mesh points and the probe sample points
                    double current_mesh_x_pt = x(ptIdx) + dem_xllCorner;
                    double current_mesh_y_pt = y(ptIdx) + dem_yllCorner;
                    double current_mesh_z_pt = z(ptIdx);
                    double tol = 0.0001;
                    if ( fabs(current_x_pt-current_mesh_x_pt) < tol && fabs(current_y_pt-current_mesh_y_pt) < tol && fabs(current_z_pt-current_mesh_z_pt) < tol )
                    {
                        // points match, it's a good data point in the right spot
                        u(ptIdx) = current_ux_pt;
                        v(ptIdx) = current_uy_pt;
                        w(ptIdx) = current_uz_pt;
                        
                        startLinePos = endLinePos+1;  // found that the data of this line matched, so can finally move on to the next line
                        
                    } else
                    {
                        // it's a data point that was quietly dropped during the sample process
                        double current_ux_pt = noDataVal;
                        double current_uy_pt = noDataVal;
                        double current_uz_pt = noDataVal;
                        u(ptIdx) = current_ux_pt;
                        v(ptIdx) = current_uy_pt;
                        w(ptIdx) = current_uz_pt;
                    }
                    
                }  // if ( endLinePos == -1 )  aka EOF check
        
            }  // for colIdx 0 to ncols
        }  // for rowIdx 0 to nrows
    }  // for layerIdx 0 to nlayers
    
}

void NinjaFoam::readInProbeData( const wn_3dArray& x, const wn_3dArray& y, const wn_3dArray& z, 
                                 const double dem_xllCorner, const double dem_yllCorner, 
                                 const int ncols, const int nrows, const int nlayers, 
                                 wn_3dScalarField& k )
{
    CPLDebug("NINJAFOAM", "reading in probes sample data");
    
    const char *probesPostProcessDirname = "probes";
    if ( foamVersion == "2.2.0" ) {
        probesPostProcessDirname = "sets";
    }
    
    // method from surfaces sampling, to find the time directory for the surfaces file
    char **papszOutputProbeDataPath;
    papszOutputProbeDataPath = VSIReadDir(CPLSPrintf("%s/postProcessing/%s/", pszFoamPath, probesPostProcessDirname));
    
    const char *probeSampleData_filename;
    const char *timeDir;
    for(int i = 0; i < CSLCount( papszOutputProbeDataPath ); i++){
        if(std::string(papszOutputProbeDataPath[i]) != "." &&
           std::string(papszOutputProbeDataPath[i]) != "..") {
            timeDir = papszOutputProbeDataPath[i];
            probeSampleData_filename = CPLSPrintf("%s/postProcessing/%s/%s/points_k.xy", pszFoamPath, probesPostProcessDirname, timeDir);
            break;
        }
        else{
            continue;
        }
    }
    
    
    // read the full data file into a string separated by "\n" chars for each line
    VSILFILE *fin;
    fin = VSIFOpenL( probeSampleData_filename, "r" );
    
    char *data;
    
    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);
    
    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';
    
    std::string probeSampleData_allLines(data);
    
    CPLFree(data);
    VSIFCloseL( fin );
    
    
    // now go through the data string line by line from where the probe data starts, 
    // comparing the probe data points with the mesh points to detect whether any data points were silently dropped for being outside the mesh
    // filling any dropped probe data with NoData vals to be filled at later steps
    double noDataVal = -9999;
    int startLinePos = 0;  // data starts right at the first character of the file, so no need to set it by finding a specific spot/character in the file
    int endLinePos;
    for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
    {
        for(int rowIdx=0; rowIdx<nrows; rowIdx++)
        {
            for(int colIdx=0; colIdx<ncols; colIdx++)
            {
                int ptIdx = layerIdx*nrows*ncols + rowIdx*ncols + colIdx;
                
                endLinePos = probeSampleData_allLines.find("\n",startLinePos+1);
                
                //// .find() returns -1 when it can't find a value, and YES it lets it try for values past the length of the string with this as the return value. So this happens when EOF
                if ( endLinePos == -1 )
                {
                    // EOF, so it's a data point that was quietly dropped during the sample process
                    
                    double current_k_pt = noDataVal;
                    
                    k(ptIdx) = current_k_pt;
                    
                } else
                {
                    
                    std::string currentLine = probeSampleData_allLines.substr(startLinePos,endLinePos-startLinePos);
                    
                    // now that the current line is found, it is time to process the given line into data values
                    int valStartSpot = 0;  // no white spaces or anything before the first value, which is x
                    int valEndSpot = currentLine.find(" ",valStartSpot);  // man, I got lucky, turns out it was a space followed by a tab between each data value. If it were not always this consistently, would have needed to adjust this part and it would be a great pain
                    std::string current_x_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find(" ",valStartSpot);
                    std::string current_y_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find(" ",valStartSpot);
                    std::string current_z_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot);
                    valStartSpot = valEndSpot + 1;  // increment to check the next spot
                    // seems to have an additional unknown number of spaces between data points
                    while ( currentLine.substr(valStartSpot,1) == " " || currentLine.substr(valStartSpot,1) == "\t" ) {
                        valStartSpot = valStartSpot + 1;
                    }
                    valEndSpot = currentLine.find("\n",valStartSpot);  // goes to the end of the line, no whitespace after the value, still want to drop the \n part though
                    std::string current_k_pt_str = currentLine.substr(valStartSpot,valEndSpot-valStartSpot-1); // -1 to drop the \n part
                    
                    double current_x_pt = atof(current_x_pt_str.c_str());
                    double current_y_pt = atof(current_y_pt_str.c_str());
                    double current_z_pt = atof(current_z_pt_str.c_str());
                    double current_k_pt = atof(current_k_pt_str.c_str());
                    
                    
                    // now to compare the found data points with the current mesh points, 
                    // if they match then can store the datapoint and increment to go to the next line of probe sample data, 
                    // otherwise OpenFOAM silently dropped the datapoint and NoDataVal needs stored instead, wait to go to the next line of probe sample data
                    // need to add back in xllCorner, yllCorner to make the points consistent between the mesh points and the probe sample points
                    double current_mesh_x_pt = x(ptIdx) + dem_xllCorner;
                    double current_mesh_y_pt = y(ptIdx) + dem_yllCorner;
                    double current_mesh_z_pt = z(ptIdx);
                    double tol = 0.0001;
                    if ( fabs(current_x_pt-current_mesh_x_pt) < tol && fabs(current_y_pt-current_mesh_y_pt) < tol && fabs(current_z_pt-current_mesh_z_pt) < tol )
                    {
                        // points match, it's a good data point in the right spot
                        
                        // tke to velFluct conversion
                        // from m^2/s^2 to m/s, velFluct = sqrt(2/3*k)
                        current_k_pt = std::sqrt(2.0/3.0*current_k_pt);
                        
                        k(ptIdx) = current_k_pt;
                        
                        startLinePos = endLinePos+1;  // found that the data of this line matched, so can finally move on to the next line
                        
                    } else
                    {
                        // it's a data point that was quietly dropped during the sample process
                        double current_k_pt = noDataVal;
                        k(ptIdx) = current_k_pt;
                    }
                    
                }  // if ( endLinePos == -1 )  aka EOF check
        
            }  // for colIdx 0 to ncols
        }  // for rowIdx 0 to nrows
    }  // for layerIdx 0 to nlayers
    
}


void NinjaFoam::fillEmptyProbeVals(const wn_3dArray& z, 
                                   const int ncols, const int nrows, const int nlayers, 
                                   wn_3dScalarField& u, wn_3dScalarField& v, wn_3dScalarField& w)
{
    CPLDebug("NINJAFOAM", "filling in probes sample no data vals");
    
    //double noDataVal = -9999;  // make sure this one matches the one used in the readInProbeData() function
    double noDataCheckVal = -9998;  // instead of using the if == noDataVal, use if > noDataCheckVal or if < noDataCheckVal depending on the style of the noDataVal
    
    // going to first do a log profile fill for all cells with noData vals up to the lowest known z value for each column, 
    // then do an ascii grid style nan filling to fill in any remaining noData vals
    // 
    // currently, the log profile filling seems to get all the noData vals, but keep in mind the method in case it ever becomes relevant
    // 
    // the lowest known z value is the first non noData val in a given column, or the 1stCellHeight value, whichever is higher up in the column
    // so if values below 1stCellHeight have data, they are also replaced with log profile interpolation
    
    windProfile profile;
    profile.profile_switch = windProfile::monin_obukov_similarity;
    
    for(int rowIdx=0; rowIdx<nrows; rowIdx++)
    {
        for(int colIdx=0; colIdx<ncols; colIdx++)
        {
            
            double lowestKnown_zIdx = nlayers;
            for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
            {
                if ( u(rowIdx,colIdx,layerIdx) > noDataCheckVal ) {  // the values for each component are always set together for a given idx, so only need to check one of them
                    lowestKnown_zIdx = layerIdx;
                    break;
                }
            }
            
            if ( lowestKnown_zIdx == nlayers-1 ) {
                // is a column of no data values, skip it for the log profile part of the fill
                // also warn because the method for filling no data values past the log profile part hasn't yet been implemented
                std::cout << "!!! no lowest known zIdx for column of data !!! for rowIdx = " << rowIdx << ", colIdx = " << colIdx << std::endl;
                continue;
            }
            
            
            double current_z_ground = z(rowIdx,colIdx,0);
            
            double firstCellHeight_zIdx = 0;
            for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
            {
                if ( z(rowIdx,colIdx,layerIdx)-current_z_ground >= finalFirstCellHeight )
                {
                    firstCellHeight_zIdx = layerIdx;
                    break;
                }
            }
            
            // this needs run each and every time to make the code run safely
            // if lowest known z idx is less than firstCellHeight z idx, use the firstCellHeight z idx as the lowest known value
            // tends to be true more often than not
            if ( lowestKnown_zIdx < firstCellHeight_zIdx ) {
                lowestKnown_zIdx = firstCellHeight_zIdx;
            }
            
            
            double z_ref = z(rowIdx,colIdx,lowestKnown_zIdx);
            double u_ref = u(rowIdx,colIdx,lowestKnown_zIdx);
            double v_ref = v(rowIdx,colIdx,lowestKnown_zIdx);
            double w_ref = w(rowIdx,colIdx,lowestKnown_zIdx);
            
            profile.ObukovLength = init->L(rowIdx,colIdx);
            profile.ABL_height = init->bl_height(rowIdx,colIdx);
            profile.Roughness = input.surface.Roughness(rowIdx,colIdx);
            double current_rough_h = input.surface.Rough_h(rowIdx,colIdx);
            double current_rough_d = input.surface.Rough_d(rowIdx,colIdx);
            profile.Rough_h = current_rough_h;
            profile.Rough_d = current_rough_d;
            
            // this is height above the vegetation
            profile.inputWindHeight = z_ref - current_z_ground;  // needs to be AGL
            
            for(int zIdx=1; zIdx<lowestKnown_zIdx; zIdx++)
            {
                // this is height above THE GROUND!! (not "z=0" for the log profile)
                profile.AGL = z(rowIdx,colIdx,zIdx) - current_z_ground + current_rough_h;  // needs to be AGL
                
                profile.inputWindSpeed = u_ref;
                u(rowIdx,colIdx,zIdx) = profile.getWindSpeed();
                profile.inputWindSpeed = v_ref;
                v(rowIdx,colIdx,zIdx) = profile.getWindSpeed();
                // not sure if this is valid or not, usually just do u and v not w, but without this, it is still full of all those nans, so I guess just go with it
                // seems to be working out all right
                profile.inputWindSpeed = w_ref;
                w(rowIdx,colIdx,zIdx) = profile.getWindSpeed();
            }
            
            // the ground z values are all supposed to be zero
            u(rowIdx,colIdx,0) = 0.0;
            v(rowIdx,colIdx,0) = 0.0;
            w(rowIdx,colIdx,0) = 0.0;
        }
    }
    
}

void NinjaFoam::fillEmptyProbeVals(const wn_3dArray& z, 
                                   const int ncols, const int nrows, const int nlayers, 
                                   wn_3dScalarField& k)
{
    CPLDebug("NINJAFOAM", "filling in probes sample no data vals");
    
    //double noDataVal = -9999;  // make sure this one matches the one used in the readInProbeData() function
    double noDataCheckVal = -9998;  // instead of using the if == noDataVal, use if > noDataCheckVal or if < noDataCheckVal depending on the style of the noDataVal
    
    // find the first non nan value for a given column, and fill all values below with that value
    // so no log profile correction for this dataset
    
    for(int rowIdx=0; rowIdx<nrows; rowIdx++)
    {
        for(int colIdx=0; colIdx<ncols; colIdx++)
        {
            
            double lowestKnown_zIdx = nlayers;
            for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
            {
                if ( k(rowIdx,colIdx,layerIdx) > noDataCheckVal ) {
                    lowestKnown_zIdx = layerIdx;
                    break;
                }
            }
            
            if ( lowestKnown_zIdx == nlayers-1 ) {
                // is a column of no data values, skip it for the log profile part of the fill
                // also warn because the method for filling no data values past the log profile part hasn't yet been implemented
                std::cout << "!!! no lowest known zIdx for column of data !!! for rowIdx = " << rowIdx << ", colIdx = " << colIdx << std::endl;
                continue;
            }
            
            
            double lowestKnown_zVal = k(rowIdx,colIdx,lowestKnown_zIdx);
            
            for(int zIdx=0; zIdx<lowestKnown_zIdx; zIdx++)
            {
                k(rowIdx,colIdx,zIdx) = lowestKnown_zVal;
            }
            
        }
    }
    
}


void NinjaFoam::generateColMaxGrid(const wn_3dArray& z, 
                                   const double dem_xllCorner, const double dem_yllCorner, 
                                   const int ncols, const int nrows, const int nlayers, 
                                   const double massMeshResolution, std::string prjString, 
                                   wn_3dScalarField& k) {
    
    CPLDebug("NINJAFOAM", "generating turbulence column max ascii grid from NINJAFOAM mass mesh");
    
    double colHeightAGL = input.colMax_colHeightAGL;
    lengthUnits::toBaseUnits(colHeightAGL, input.colMax_colHeightAGL_units);
    CPLDebug("NINJAFOAM", "sampling colHeightAGL = %f m",colHeightAGL);
    
    // now need to go through the data, and get the col max, and put it into an ascii grid
    colMaxGrid.set_headerData( ncols, nrows, dem_xllCorner, dem_yllCorner, massMeshResolution, -9999.0, -9999.0, prjString );
    for ( int rowIdx = 0; rowIdx < nrows; rowIdx++ )
    {
        for ( int colIdx = 0; colIdx < ncols; colIdx++ )
        {
            double current_z_ground = z(rowIdx,colIdx,0);
            
            double current_colMaxVal = -999999.0;   // start val for find max, really small val, make sure it is smaller than noDataVal just in case
            for(int layerIdx=0; layerIdx<nlayers; layerIdx++)
            {
                if ( z(rowIdx,colIdx,layerIdx) - current_z_ground > colHeightAGL ) {
                    // looked at all values up to the colHeightAGL, move on
                    break;
                }
                if ( current_colMaxVal < k(rowIdx,colIdx,layerIdx) ) {
                    current_colMaxVal = k(rowIdx,colIdx,layerIdx);
                }
            }
            
            colMaxGrid.set_cellValue( rowIdx, colIdx, current_colMaxVal );
        }
    }
    
}


void NinjaFoam::GenerateAndSampleMassMesh()
{
    
    // generate massMesh if required for other outputs
    try{
	    if ( writeMassMesh == true ) {
	        generateMassMesh();
	    }
	}catch (exception& e)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during NINJAFOAM mass mesh generation: %s", e.what());
		// disallow later outputs using the dataset, it wasn't created and later code would then reference outside the array
		input.writeTurbulence = false;
		if(input.diurnalWinds == false){
		    input.volVTKOutFlag = false;
	    }
	}catch (...)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during NINJAFOAM mass mesh generation: Cannot determine exception type.");
		// disallow later outputs using the dataset, it wasn't created and later code would then reference outside the array
		input.writeTurbulence = false;
		if(input.diurnalWinds == false){
		    input.volVTKOutFlag = false;
	    }
	}
    
    
    // prep colMaxGrid
    try{
	    if ( input.writeTurbulence == true ) {
	        generateColMaxGrid(massMesh.ZORD, input.dem.xllCorner, input.dem.yllCorner, input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers, massMesh.meshResolution, input.dem.prjString, massMesh_k);
	    }
	}catch (exception& e)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during turbulence column max from NINJAFOAM mass mesh ascii grid generation: %s", e.what());
		// disallow later outputs using the dataset, it wasn't created and later code would then reference outside the array
		input.writeTurbulence = false;
		if(input.diurnalWinds == false){
		    input.volVTKOutFlag = false;
	    }
	}catch (...)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during turbulence column max from NINJAFOAM mass mesh ascii grid generation: Cannot determine exception type.");
		// disallow later outputs using the dataset, it wasn't created and later code would then reference outside the array
		input.writeTurbulence = false;
		if(input.diurnalWinds == false){
		    input.volVTKOutFlag = false;
	    }
	}
    
}


void NinjaFoam::SetOutputResolution()
{
    //Set output file resolutions now
    if( input.kmzResolution <= 0.0 )  //if negative, use DEM resolution
        input.kmzResolution = input.dem.get_cellSize();
    if( input.shpResolution <= 0.0 )  //if negative, use DEM resolution
        input.shpResolution = input.dem.get_cellSize();
    if( input.velResolution <= 0.0 )  //if negative, use DEMresolution
        input.velResolution = input.dem.get_cellSize();
    if( input.angResolution <= 0.0 )  //if negative, use DEM resolution
        input.angResolution = input.dem.get_cellSize();
    if( input.pdfResolution <= 0.0 )
        input.pdfResolution = input.dem.get_cellSize();
}

void NinjaFoam::SetOutputFilenames()
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

    input.volVTKFile = rootFile + fileAppend + ".vtk";

    input.legFile = rootFile + kmz_fileAppend + ".bmp";
    if( input.ninjaTime.is_not_a_date_time() )	//date and time not set?
        input.dateTimeLegFile = "";
    else
        input.dateTimeLegFile = rootFile + kmz_fileAppend + ".date_time" + ".bmp";
}

void NinjaFoam::WriteOutputFiles()
{
    /*-------------------------------------------------------------------*/
    /* prepare output                                                    */
    /*-------------------------------------------------------------------*/

    //Clip off bounding doughnut if desired
    VelocityGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
    AngleGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
    if(input.writeTurbulence)
    {
        TurbulenceGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
        colMaxGrid.clipGridInPlaceSnapToCells(input.outputBufferClipping);
    }

    //change windspeed units back to what is specified by speed units switch
    velocityUnits::fromBaseUnits(VelocityGrid, input.outputSpeedUnits);
    if(input.writeTurbulence)
    {
        velocityUnits::fromBaseUnits(TurbulenceGrid, input.outputSpeedUnits);
        velocityUnits::fromBaseUnits(colMaxGrid, input.outputSpeedUnits);
    }

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

                        // if output clipping was set by the user, don't buffer to overlap the DEM
                        // but only if writing atm file for farsite grids
                        if(!input.outputBufferClipping > 0.0 && input.writeAtmFile == true)
                        {
                            //ensure grids cover original DEM extents for FARSITE
                            AsciiGrid<double> demGrid;
                            GDALDatasetH hDS;
                            hDS = GDALOpen( input.dem.fileName.c_str(), GA_ReadOnly );
                            if( hDS == NULL )
                            {
                                input.Com->ninjaCom(ninjaComClass::ninjaNone,
                                        "Problem reading DEM during output writing." );
                            }
                            GDAL2AsciiGrid( (GDALDataset *)hDS, 1, demGrid );
                            tempCloud.BufferToOverlapGrid(demGrid);
                            angTempGrid->BufferToOverlapGrid(demGrid);
                            velTempGrid->BufferToOverlapGrid(demGrid);
                        }

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
			AsciiGrid<double> *velTempGrid, *angTempGrid, *turbTempGrid, *colMaxTempGrid;
			velTempGrid=NULL;
			angTempGrid=NULL;
			turbTempGrid=NULL;
			colMaxTempGrid=NULL;

			KmlVector ninjaKmlFiles;

			angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.kmzResolution, 
                                    AsciiGrid<double>::order0));
			velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.kmzResolution, 
                                    AsciiGrid<double>::order0));
                        if(input.writeTurbulence)
                        {
                            //turbTempGrid = new AsciiGrid<double> (TurbulenceGrid.resample_Grid(input.kmzResolution, 
                            //            AsciiGrid<double>::order0));
                            //
                            //ninjaKmlFiles.setTurbulenceFlag("true");
                            //ninjaKmlFiles.setTurbulenceGrid(*turbTempGrid, input.outputSpeedUnits);
                            
                            
                            colMaxTempGrid = new AsciiGrid<double> (colMaxGrid.resample_Grid(input.kmzResolution, 
                                        AsciiGrid<double>::order0));
                            
                            ninjaKmlFiles.setColMaxFlag("true");
                            ninjaKmlFiles.setColMaxGrid(*colMaxTempGrid, input.outputSpeedUnits,  input.colMax_colHeightAGL, input.colMax_colHeightAGL_units);
                            
                            //// for debugging
                            //colMaxGrid.ascii2png("colMaxGrid.png", "Speed Fluctuation", "(mph)", "colMax_legend", true, false);
                            //colMaxTempGrid->ascii2png("colMaxTempGrid.png", "Speed Fluctuation", "(mph)", "colMax_legend", true, false);
                        }

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
			if(turbTempGrid)
			{
				delete turbTempGrid;
				turbTempGrid=NULL;
			}
			if(colMaxTempGrid)
			{
				delete colMaxTempGrid;
				colMaxTempGrid=NULL;
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
	
	
	try{
	    if ( input.volVTKOutFlag == true ) {
	        writeMassMeshVtkOutput();
	    }
	}catch (exception& e)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during NINJAFOAM mass mesh vtk file writing: %s", e.what());
	}catch (...)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during NINJAFOAM mass mesh vtk file writing: Cannot determine exception type.");
	}
	
}


void NinjaFoam::writeMassMeshVtkOutput()
{
    CPLDebug("NINJAFOAM", "writing mass mesh vtk output for foam simulation.");
    
    try {
        bool vtk_out_as_utm = false;
	    if(CSLTestBoolean(CPLGetConfigOption("VTK_OUT_AS_UTM", "FALSE")))
        {
            vtk_out_as_utm = CPLGetConfigOption("VTK_OUT_AS_UTM", "FALSE");
        }
        // can pick between "ascii" and "binary" format for the vtk write format
        std::string vtkWriteFormat = "binary";//"binary";//"ascii";
        std::string found_vtkWriteFormat = CPLGetConfigOption("VTK_OUT_FORMAT", "binary");
        if(found_vtkWriteFormat != "")
        {
            vtkWriteFormat = found_vtkWriteFormat;
        }
		volVTK VTK(massMesh_u, massMesh_v, massMesh_w, massMesh.XORD, massMesh.YORD, massMesh.ZORD, input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_nCols(), input.dem.get_nRows(), massMesh.nlayers, input.volVTKFile, vtkWriteFormat, vtk_out_as_utm);
	} catch (exception& e) {
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during volume VTK file writing: %s", e.what());
	} catch (...) {
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during volume VTK file writing: Cannot determine exception type.");
	}
	
}



void NinjaFoam::UpdateExistingCase()
{
    if(!CheckForValidDem())
        throw std::runtime_error(std::string("The DEM ") + input.dem.fileName.c_str() + 
                " does not correspond to the supplied case directory " + pszFoamPath);

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Using existing case directory...");
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Updating case files...");

    /*
    ** Copy and save OpenFOAM files for this timestep if NINJAFOAM_KEEP_ALL_TIMESTEPS is TRUE.
    */
    if(CSLTestBoolean(CPLGetConfigOption("NINJAFOAM_KEEP_ALL_TIMESTEPS", "FALSE")))
    {
        CPLDebug("NINJAFOAM", "Keeping all timesteps for simulation.");

        char *pszPath;
        char *pszFoamTimePath = CPLStrdup(CPLSPrintf("%s_run%d", pszFoamPath, input.inputsRunNumber-1));
        VSIMkdir(pszFoamTimePath, 0777);
        
        CPLDebug("NINJAFOAM", "Writing files for the previous timestep to %s", pszFoamTimePath);

        const char *pszInput;
        const char *pszOutput;
        char **papszFileList;
        const char *pszFilename;
        std::string osFullPath;
        papszFileList = NinjaVSIReadDirRecursive(pszFoamPath);
        for(int i = 0; i < CSLCount( papszFileList ); i++){
            pszFilename = CPLGetFilename(papszFileList[i]);
            osFullPath = papszFileList[i];
            if(std::string(pszFilename) == ""){
                VSIMkdir(CPLFormFilename(pszFoamTimePath, osFullPath.c_str(), ""), 0777);
            }
            else{
                pszInput = CPLFormFilename(pszFoamPath, osFullPath.c_str(), "");
                pszOutput = CPLFormFilename(pszFoamTimePath, osFullPath.c_str(), "");
                CopyFile(pszInput, pszOutput);
            }
        }

        CSLDestroy(papszFileList);
    }

    //find and delete the old sampled output and residual directories
    std::string outputSurfacePath = CPLSPrintf("%s/postProcessing/surfaces/", pszFoamPath);
    std::string outputProbesPath = CPLSPrintf("%s/postProcessing/probes/", pszFoamPath);
    if ( foamVersion == "2.2.0" ) {
        outputProbesPath = CPLSPrintf("%s/postProcessing/sets/", pszFoamPath);
    }
    std::string outputResidualsPath = CPLSPrintf("%s/postProcessing/residuals/", pszFoamPath);

    NinjaUnlinkTree( outputSurfacePath.c_str() );
    NinjaUnlinkTree( outputProbesPath.c_str() );
    NinjaUnlinkTree( outputResidualsPath.c_str() );

    //set meshResolution and other values from log.ninja
    ReadNinjaLog();

    //read in firstCellHeight from the 0 dir on disk before it is lost during WriteFoamFiles()
    initialFirstCellHeight = GetFirstCellHeightFromDisk(0);

    //many files are overwritten by writeFoamFiles(), most with the same values replaced back in
    //make a tmp copy of those files overwritten by writeFoamFiles(), but where the values are not replaced back in
    std::string tmpPath = CPLGetPath(input.dem.fileName.c_str());
    CopyFile(CPLFormFilename(pszFoamPath,     "constant/polyMesh/blockMeshDict", ""),
             CPLFormFilename(tmpPath.c_str(), "blockMeshDict", ""));
    CopyFile(CPLFormFilename(pszFoamPath,     "system/topoSetDict", ""),
             CPLFormFilename(tmpPath.c_str(), "topoSetDict", ""));

    //write the new dict files
    WriteFoamFiles();

    //put the tmp copy files back, delete the leftover tmp files from the tmp location
    CopyFile(CPLFormFilename(tmpPath.c_str(), "blockMeshDict", ""),
             CPLFormFilename(pszFoamPath,     "constant/polyMesh/blockMeshDict", ""));
    CopyFile(CPLFormFilename(tmpPath.c_str(), "topoSetDict", ""),
             CPLFormFilename(pszFoamPath,     "system/topoSetDict", ""));
    VSIUnlink(CPLFormFilename(tmpPath.c_str(), "blockMeshDict", ""));
    VSIUnlink(CPLFormFilename(tmpPath.c_str(), "topoSetDict", ""));

    //rm latestTime in case (old flow solution)
    latestTime = GetLatestTimeOnDisk();
    if( latestTime > 50+nRoundsRefinement )
    {
        NinjaUnlinkTree(CPLSPrintf("%s/%d", pszFoamPath, latestTime));
    }

    //now latestTime on disk is where we want to start (the mesh is here)
    latestTime = GetLatestTimeOnDisk();

    //Get rid of -9999.9 in 0/ files
    CopyFile(CPLFormFilename(pszFoamPath, "0/U", ""),
             CPLFormFilename(pszFoamPath, "0/U", ""),
             "-9999.9",
             CPLSPrintf("%.2f", initialFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/k", ""),
             CPLFormFilename(pszFoamPath, "0/k", ""),
             "-9999.9",
             CPLSPrintf("%.2f", initialFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/epsilon", ""),
             CPLFormFilename(pszFoamPath, "0/epsilon", ""),
             "-9999.9",
             CPLSPrintf("%.2f", initialFirstCellHeight));

    //copy 0/ to each later time/ up to latestTime/
    int finalTime = latestTime;
    for( int time = 50; time <= finalTime; time++ )
    {
        // update latestTime to the current time for the current operation
        latestTime = time;
        //read in firstCellHeight from latestTime dir on disk before it is overwritten with the new files
        finalFirstCellHeight = GetFirstCellHeightFromDisk(latestTime);
        UpdateTimeDirFiles();
    }

    //update controlDict
    CopyFile(CPLSPrintf("%s/system/controlDict_simpleFoam", pszFoamPath),
             CPLSPrintf("%s/system/controlDict", pszFoamPath));
    // this is for updating controlDict to a different, better form,
    // rather than the standard form set in the line above
    //CopyFile(CPLSPrintf("%s/system/controlDict", pszFoamPath),
    //         CPLSPrintf("%s/system/controlDict", pszFoamPath),
    //         "startFrom       latestTime", "startFrom       startTime");
    //CopyFile(CPLSPrintf("%s/system/controlDict", pszFoamPath),
    //         CPLSPrintf("%s/system/controlDict", pszFoamPath),
    //         "latestTime", CPLSPrintf("%d", latestTime));

    //update simpleFoam controlDict endTime
    UpdateSimpleFoamControlDict();

    //rm any processor* directories
    std::vector<std::string> dirList = GetProcessorDirsOnDisk();
    for(int n=0; n<dirList.size(); n++){
        NinjaUnlinkTree( CPLSPrintf( "%s/processor%d", pszFoamPath, n) );
    }

    //rm any additional log, output, and system files that will later be replaced during the run, if they exist
    //log.blockMesh, log.moveDynamicMesh, log.refineMesh, log.topoSet should be kept, those processes aren't rerun
    char **papszFileList = NULL;
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.decomposePar", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.reconstructPar", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.renumberMesh", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.applyInit", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.simpleFoam", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.sample", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "foam.tif", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "output.raw", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "output.vrt", "") );
    papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "log.probeSample", "") );
    if ( foamVersion == "2.2.0" ) {
        papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "system/sampleDict_probes", "") );
    } else {
        papszFileList = CSLAddString( papszFileList, CPLFormFilename(pszFoamPath, "system/probes", "") );
    }
    for(int i = 0; i < CSLCount( papszFileList ); i++)
    {
        VSIUnlink(papszFileList[i]);
    }
    CSLDestroy( papszFileList );

    //renumbering mesh is NOT required, the mesh and files are already in renumbered form
    //but calling renumberMesh is handy for updating the "value" entries in k and epsilon
    CPLDebug("NINJAFOAM", "calling renumberMesh, to update latestTime \"value\" entries...");
    RenumberMesh();
}

void NinjaFoam::GenerateNewCase()
{
    #ifdef _OPENMP
    startFoamFileWriting = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing OpenFOAM files...");

    //writes *most* of the foam files, but not all can be written at this point
    WriteFoamFiles();

    //write controlDict for flow solution--this will get modified during moveDynamicMesh
    const char *pszInput = CPLFormFilename(pszFoamPath, "system/controlDict_simpleFoam", "");
    const char *pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    CopyFile(pszInput, pszOutput);

    checkCancel();

    /*-------------------------------------------------------------------*/
    /*  write blockMesh mesh file(s)                                     */
    /*  the meshResolution is also set here if needed                    */
    /*  and the DEM is resampled to the meshResolution                   */
    /*-------------------------------------------------------------------*/

    writeBlockMesh();

    /*-------------------------------------------------------------------*/
    /*  convert DEM to STL format and write to constant/triSurface       */
    /*-------------------------------------------------------------------*/

    #ifdef _OPENMP
    startStlConversion = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Converting DEM to STL format...");

    std::string demName = NinjaSanitizeString(CPLGetBasename(input.dem.fileName.c_str()));
    const char *pszStlFileName = CPLStrdup(CPLFormFilename(
                (CPLSPrintf("%s/constant/triSurface/", pszFoamPath)),
                CPLSPrintf("%s.stl", demName.c_str()), ""));

    //buffer input.dem on all edges to ensure it is larger than the blockMesh
    Elevation bufferedDemGrid = input.dem;
    bufferedDemGrid.BufferAroundGridInPlace(1, 1);

    CPLErr eErr;
    eErr = NinjaElevationToStl(bufferedDemGrid,
                        pszStlFileName,
                        NinjaStlBinary,
                        0.0);

    CPLFree((void*)pszStlFileName);

    if(eErr != 0){
        throw std::runtime_error("Error while converting DEM to STL format.");
    }

    checkCancel();

    #ifdef _OPENMP
    endStlConversion = omp_get_wtime();
    #endif

//    keeping this here until sampleCloud() has been updated to no longer use outputSampleGrid
//    /*-------------------------------------------------------------------*/
//    /*  write output stl and run surfaceCheck on original stl            */
//    /*-------------------------------------------------------------------*/
//
//    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Transforming surface points to output wind height...");
//
//    // create the output surface stl with NinjaElevationToStl unless
//    demName = NinjaSanitizeString(CPLGetBasename(input.dem.fileName.c_str()));
//    pszStlFileName = CPLStrdup((CPLSPrintf("%s/constant/triSurface/%s_out.stl", pszFoamPath, demName.c_str())));

    //create the grid to sample on (input.outputWindHeight above the DEM)
    //note that input.dem has already been resampled to the mesh resolution
    outputSampleGrid = input.dem; 
    //make sure the grid is completely inside the mesh
    outputSampleGrid.BufferAroundGridInPlace(-1, -1);

//    eErr = NinjaElevationToStl(outputSampleGrid,
//                        pszStlFileName,
//                        NinjaStlBinary,
//                        input.outputWindHeight);
//
//    CPLFree((void*)pszStlFileName);

    /*-------------------------------------------------------------------*/
    /*  write remaining mesh file(s)                                     */
    /*-------------------------------------------------------------------*/
    writeMoveDynamicMesh();

    #ifdef _OPENMP
    endFoamFileWriting = omp_get_wtime();
    #endif

    checkCancel();

    /*-------------------------------------------------------------------*/
    /* create the mesh                                                   */
    /*-------------------------------------------------------------------*/

    #ifdef _OPENMP
    startMesh = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating mesh...");

    MoveDynamicMesh();
    
    checkCancel();

    /*refine mesh near the ground */
    RefineSurfaceLayer();

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Renumbering mesh...");
    RenumberMesh();

    #ifdef _OPENMP
    endMesh = omp_get_wtime();
    #endif

    //write log.ninja
    WriteNinjaLog();

    //write output stl
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Transforming surface points to output wind height...");
    createOutputSurfSampleStl();

    checkCancel();
}

void NinjaFoam::WriteNinjaLog()
{
    //write log.ninja to store info needed for reusing cases
    int nRet = -1;
    const char *pszInput = CPLSPrintf("%s/log.ninja", pszFoamPath);
    VSILFILE *fout;
    fout = VSIFOpenL(pszInput, "w");
    if( !fout ){
        throw std::runtime_error("Error writing log.ninja to case directory.");
    }
    nRet = VSIFPrintfL(fout, "meshResolution = %.2f\n", meshResolution);
    nRet = VSIFPrintfL(fout, "nRoundsRefinement = %d\n", nRoundsRefinement);
    VSIFCloseL(fout);
}

void NinjaFoam::ReadNinjaLog()
{
    // set meshResolution and other values from log.ninja
    const char *pszInput = CPLSPrintf("%s/log.ninja", pszFoamPath);
    VSILFILE *fin;
    fin = VSIFOpenL(pszInput, "r");
    if(fin == NULL)
    {
        throw std::runtime_error("Can't open log.ninja to set the mesh resolution!");
    }

    std::string currentLine, currentSubstr;
    size_t pos1, pos2;

    currentLine = CPLReadLineL(fin);
    pos1 = currentLine.find("meshResolution");
    if(pos1 == currentLine.npos)
    {
        throw std::runtime_error("log.ninja does not contain meshResolution!");
    } else
    {
        pos1 = pos1 + 17;
        pos2 = currentLine.length() - 1;
        currentSubstr = currentLine.substr(pos1, pos2-pos1+1);
        set_meshResolution( atof(currentSubstr.c_str()), lengthUnits::meters );
        CPLDebug("NINJAFOAM", "meshResolution = %f %s", meshResolution, lengthUnits::getString(meshResolutionUnits));
    }

    currentLine = CPLReadLineL(fin);
    pos1 = currentLine.find("nRoundsRefinement");
    if(pos1 == currentLine.npos)
    {
        throw std::runtime_error("log.ninja does not contain nRoundsRefinement!");
    } else
    {
        pos1 = pos1 + 20;
        pos2 = currentLine.length() - 1;
        currentSubstr = currentLine.substr(pos1, pos2-pos1+1);
        nRoundsRefinement = atof(currentSubstr.c_str());
        CPLDebug("NINJAFOAM", "nRoundsRefinement = %d", nRoundsRefinement);
    }

    VSIFCloseL(fin);
}

int NinjaFoam::GetLatestTimeOnDisk()
{
    std::vector<std::string> dirList = GetTimeDirsOnDisk();
    int latest = 0;

    for(int i=0; i<dirList.size(); i++){
        if(atoi(dirList[i].c_str()) > latest){
            latest = atoi(dirList[i].c_str());    
        } 
    }

    return latest;
}

std::vector<std::string> NinjaFoam::GetProcessorDirsOnDisk()
{
    char **papszFileList;
    const char *pszFilename;
    papszFileList = VSIReadDir( pszFoamPath );
    std::vector<std::string> dirList;

    for(int i=0; i<CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename( papszFileList[i] );
        std::string str(pszFilename);
        if(str.find("processor") != str.npos){
            dirList.push_back(str);  
        }
    }

    CSLDestroy( papszFileList );

    return dirList;
}

std::vector<std::string> NinjaFoam::GetTimeDirsOnDisk()
{
    char **papszFileList;
    const char *pszFilename;
    papszFileList = VSIReadDir( pszFoamPath );
    std::vector<std::string> dirList;

    for(int i=0; i<CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename( papszFileList[i] );
        std::string str(pszFilename);
        if(StringIsNumeric(str)){
            dirList.push_back(str);  
        }
    }

    CSLDestroy( papszFileList );

    return dirList;
}

bool NinjaFoam::StringIsNumeric(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

double NinjaFoam::GetFirstCellHeightFromDisk(int time)
{
    //read time/U and search for firstCellHeight
    const char *pszInput = CPLSPrintf("%s/%d/U", pszFoamPath, time);
    VSILFILE *fin;

    fin = VSIFOpenL( pszInput, "r" );

    char *data;

    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';

    std::string s(data);
    
    CPLFree(data);
    VSIFCloseL(fin);
 
    //search string for firstCellHeight
    std::string h;
    int pos;
    if(s.find("firstCellHeight") != s.npos){
        pos = s.find("firstCellHeight ");
        h = s.substr(pos+16, (s.find(";", pos+16) - (pos+16)));
    }

    double height = atof(h.c_str());

    return height;
}

bool NinjaFoam::CheckForValidCaseDir(const char* dir)
{
    //check at least for the controlDict file
    if( !CPLCheckForFile((char*)CPLSPrintf("%s/system/controlDict", dir), NULL) ){
        return false;
    }

    return true;
}

bool NinjaFoam::CheckForValidDem()
{
    char **papszFileList;
    const char *pszFilename;
    papszFileList = VSIReadDir( CPLSPrintf("%s/constant/triSurface", pszFoamPath ) );

    for(int i=0; i<CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename( papszFileList[i] );
        std::string s(pszFilename);
        std::string ss = NinjaSanitizeString(CPLGetBasename( input.dem.fileName.c_str() ));
        if( s.find(".stl") != s.npos & s.find("_out.stl") == s.npos){
            s = (CPLGetBasename(pszFilename));
            if( s.compare(ss) != s.npos ){
                CSLDestroy( papszFileList );
                return true;
            }
        }
    }

    CSLDestroy( papszFileList );

    return false;
}

void NinjaFoam::SetMeshResolutionAndResampleDem()
{
    //if we are re-using a case, just set the meshResolution
    if(CheckForValidCaseDir(pszFoamPath)){
        //set meshResolution and other values from log.ninja
        ReadNinjaLog();
    }
    //otherwise, if the mesh resolution hasn't been set, calculate it
    else if(meshResolution < 0.0){
        // get some info from the DEM
        double zmin, zmax;
        double meshVolume;
        double cellVolume;
        double side;

        double dz = input.dem.get_maxValue() - input.dem.get_minValue();
        double dx = input.dem.get_xDimension();
        double dy = input.dem.get_yDimension();

        double blockMeshDz = max((0.1 * max(dx, dy)), (dz + 0.1 * dz));

        int nCellsX, nCellsY;

        zmin = input.dem.get_maxValue() + 0.05 * blockMeshDz; //zmin (above highest point in DEM for MDM)
        zmax = input.dem.get_maxValue() + blockMeshDz; //zmax

        //total volume for block mesh
        meshVolume = dx * dy * (zmax-zmin); 
        cellCount = 0.5 * input.meshCount; //half the cells in the blockMesh and half reserved for refineMesh
        cellVolume = meshVolume/cellCount; // volume of 1 cell
        side = std::pow(cellVolume, (1.0/3.0)); // length of side of regular hex cell
        nCellsX=int( dx / side);
        nCellsY=int( dy / side);
        //determine number of rounds of refinement
        int nCellsToAdd = 0;
        int refinedCellCount = 0;
        int nCellsInLowestLayer = nCellsX * nCellsY; 
        while(refinedCellCount < (0.5 * input.meshCount)){
            nCellsToAdd = nCellsInLowestLayer * 8; //each cell is divided into 8 cells
            refinedCellCount += nCellsToAdd - nCellsInLowestLayer; //subtract the parent cells
            nCellsInLowestLayer = nCellsToAdd/2; //only half of the added cells are in the lowest layer
            nRoundsRefinement += 1;
        }
        CPLDebug("NINJAFOAM", "refinedCellCount = %d", refinedCellCount);
        CPLDebug("NINJAFOAM", "nCellsInLowestLayer = %d", nCellsInLowestLayer);
        CPLDebug("NINJAFOAM", "nCellsToAdd = %d", nCellsToAdd);
        CPLDebug("NINJAFOAM", "nRoundsRefinement = %d", nRoundsRefinement);

        set_meshResolution(side/(nRoundsRefinement*2.0), lengthUnits::meters);
    } 
    else{ //if the mesh resolution has been set
        //default to two rounds of refinement
        nRoundsRefinement = 2; 
    }
    
    
    if ( writeMassMesh == true ) {
        // need to setup mesh sizing BEFORE the dem gets resampled, but AFTER the mesh resolution gets set
        massMesh.set_numVertLayers(20);  // done in cli.cpp calling ninja_army calling ninja calling this function, with windsim.setNumVertLayers( i_, 20); where i_ is ninjaIdx
        massMesh.set_meshResolution(meshResolution, meshResolutionUnits);
        massMesh.compute_domain_height(input);
    }
    

    //Resample DEM to desired computational resolution
    //NOTE: DEM IS THE ELEVATION ABOVE SEA LEVEL
    if(meshResolution < input.dem.get_cellSize())
    {
        input.dem.resample_Grid_in_place(meshResolution,
                Elevation::order1); //make the grid finer
        input.surface.resample_in_place(meshResolution,
                AsciiGrid<double>::order1); //make the grid finer
    }else if(meshResolution > input.dem.get_cellSize())
    {
        input.dem.resample_Grid_in_place(meshResolution,
                Elevation::order0); //coarsen the grid
        input.surface.resample_in_place(meshResolution,
                AsciiGrid<double>::order0); //coarsen the grids
    }

    // if troubles, try smoothing the dem before the whole process, AFTER resampling to mesh resolution
    if( CSLTestBoolean(CPLGetConfigOption("SMOOTH_DEM", "FALSE")) )
    {
        int smoothDist = 1;
        std::string found_smoothDist_str = CPLGetConfigOption("DEM_SMOOTH_DIST", "");
        if( found_smoothDist_str != "" )
        {
            smoothDist = atof(found_smoothDist_str.c_str());
        }

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Smoothing elevation file, smoothDist = %d ...",smoothDist);
        input.dem.smooth_elevation(smoothDist);
    }
}

bool NinjaFoam::CheckIfOutputWindHeightIsResolved()
{
    if(finalFirstCellHeight < input.outputWindHeight){
        return true;
    }
    else{
        return false;
    }
}
