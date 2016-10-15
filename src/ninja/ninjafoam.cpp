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
}

double NinjaFoam::get_meshResolution()
{
    return meshResolution;
}

bool NinjaFoam::simulate_wind()
{
    #ifdef _OPENMP
    startTotal = omp_get_wtime();
    #endif

    checkCancel();

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reading elevation file...");

    readInputFile();
    set_position();
    set_uniVegetation();

    checkInputs();

    ComputeDirection(); //convert wind direction to unit vector notation
    SetInlets();
    SetBcs();

    input.meshCount = atoi(CPLGetConfigOption("NINJAFOAM_MESH_COUNT", CPLSPrintf("%d",input.meshCount)));
    input.nIterations = atoi(CPLGetConfigOption("NINJAFOAM_ITERATIONS", CPLSPrintf("%d",input.nIterations)));
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);

    /*------------------------------------------*/
    /*  write OpenFOAM files                    */
    /*------------------------------------------*/

    CPLDebug("NINJAFOAM", "meshCount = %d", input.meshCount);
    CPLDebug("NINJAFOAM", "Rd = %lf", input.surface.Rough_d(0,0));
    CPLDebug("NINJAFOAM", "z0 = %lf", input.surface.Roughness(0,0));
    CPLDebug("NINJAFOAM", "input wind height = %lf", input.inputWindHeight);
    CPLDebug("NINJAFOAM", "input speed = %lf", input.inputSpeed);
    CPLDebug("NINJAFOAM", "input direction = %lf", input.inputDirection);
    CPLDebug("NINJAFOAM", "foam direction = (%lf, %lf, %lf)", direction[0], direction[1], direction[2]);
    CPLDebug("NINJAFOAM", "number of inlets = %ld", inlets.size());
    CPLDebug("NINJAFOAM", "input.nonEqBc = %d", input.nonEqBc);
    CPLDebug("NINJAFOAM", "Roughness = %f", input.surface.Roughness.get_meanValue());
    CPLDebug("NINJAFOAM", "Rough_d = %f", input.surface.Rough_d.get_meanValue());
    CPLDebug("NINJAFOAM", "Rough_h = %f", input.surface.Rough_h.get_meanValue());
    CPLDebug("NINJAFOAM", "input.nIterations = %d", input.nIterations);

    int status = 0;

    //if pszFoamPath is not valid, create a new case 
    if(CheckForValidCaseDir(pszFoamPath) != NINJA_SUCCESS){
        status = GenerateNewCase();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error setting up new OpenFOAM case");
            return NINJA_E_OTHER;
        }
    }
    else{ //otherwise, we're just updating an existing case
        status = UpdateExistingCase();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error setting up existing case.");
            return NINJA_E_OTHER;
        }
    }

    /*-------------------------------------------------------------------*/
    /* Apply initial conditions                                          */
    /*-------------------------------------------------------------------*/
    
    #ifdef _OPENMP
    endMesh = omp_get_wtime();
    startInit = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Applying initial conditions...");
    status = ApplyInit();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during applyInit().");
        return NINJA_E_OTHER;
    }

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
        status = DecomposePar();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during decomposePar().");
            return NINJA_E_OTHER;
        }
    }

    checkCancel();

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solving for the flow field...");
    status = SimpleFoam();
    if(status != 0){
        if(input.existingCaseDirectory == "!set"){
            //no coarsening if this is an existing case
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam(). Can't coarsen "
                    "mesh for existing case directory. Try again without using an existing case.");
            return NINJA_E_OTHER;
        }
        //try solving with previous mesh iterations (less refinement)
        while(latestTime > 50){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam(). Coarsening mesh...");
            CPLDebug("NINJAFOAM", "unlinking %s", CPLSPrintf( "%s/%d", pszFoamPath, latestTime ));
            NinjaUnlinkTree( CPLSPrintf( "%s/%d", pszFoamPath, latestTime  ) );
            if(input.numberCPUs > 1){
                for(int n=0; n<input.numberCPUs; n++){
                    NinjaUnlinkTree( CPLSPrintf( "%s/processor%d", pszFoamPath, n) );
                }
            }
            latestTime -= 1;
            meshResolution *= 2.0;
            CPLDebug("NINJAFOAM", "stepping back to time = %d", latestTime);

            /* update simpleFoam controlDict writeInterval */
            UpdateSimpleFoamControlDict();

            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Applying initial conditions...");
            status = ApplyInit();
            if(status != 0){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during applyInit().");
                return NINJA_E_OTHER;
            }
            if(input.numberCPUs > 1){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel flow calculations...");
                status = DecomposePar();
                if(status != 0){
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during decomposePar()");
                }
            }
            status = SimpleFoam();
            if(status == 0){
                break;
            }
        }
        //if the solver fails with latestTime = 50 (moveDynamicMesh mesh), we're done
        if( status != 0 & latestTime == 50 ){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during simpleFoam(). The flow solution failed.");
            return NINJA_E_OTHER;
        }
    }
    CPLDebug("NINJAFOAM", "meshResolution= %f", meshResolution);

    if(input.numberCPUs > 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");
        status = ReconstructPar();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during ReconstructPar(). Check that number of iterations is a multiple of 100.");
            return NINJA_E_OTHER;
        }
    }

    checkCancel();

    /*-------------------------------------------------------------------*/
    /* Sample at requested output height                                 */
    /*-------------------------------------------------------------------*/

    #ifdef _OPENMP
    endSolve = omp_get_wtime();
    startOutputSampling = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Sampling at requested output height...");
    status = Sample();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error while sampling the output.");
        return NINJA_E_OTHER;
    }

    status = SampleRawOutput();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error while sampling the raw output.");
        return NINJA_E_OTHER;
    }

    #ifdef _OPENMP
    endOutputSampling = omp_get_wtime();
    #endif

    /*----------------------------------------*/
    /*  write output files                    */
    /*----------------------------------------*/

    #ifdef _OPENMP
    startWriteOut = omp_get_wtime();
    #endif

    if(input.diurnalWinds == false){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing output files...");

        status = WriteOutputFiles();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during output file writing.");
            return NINJA_E_OTHER;
        }
    }
           
    #ifdef _OPENMP
    endWriteOut = omp_get_wtime();
    endTotal = omp_get_wtime();
    #endif

    /*----------------------------------------*/
    /*  wrap up                               */
    /*----------------------------------------*/

    #ifdef _OPENMP
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "File writing time was %lf seconds.", endFoamFileWriting-startFoamFileWriting);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "STL conversion time was %lf seconds.", endStlConversion-startStlConversion);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Meshing time was %lf seconds.",endMesh-startMesh);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Initialization time was %lf seconds.",endInit-startInit);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solver time was %lf seconds.",endSolve-startSolve);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Output sampling time was %lf seconds.", endOutputSampling-startOutputSampling);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Output writing time was %lf seconds.",endWriteOut-startWriteOut);
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Total simulation time was %lf seconds.",endTotal-startTotal);
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d done!", input.inputsRunNumber);

    if(!input.keepOutGridsInMemory && input.diurnalWinds == false)
    {
       CloudGrid.deallocate();
       AngleGrid.deallocate();
       VelocityGrid.deallocate();
    }

    if(input.diurnalWinds == true){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Adding diurnal winds...");
    }   

    return true;
}

int NinjaFoam::AddBcBlock(std::string &dataString)
{
    const char *pszPath =  CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    const char *pszTemplateFile;
    const char *pszPathToFile;

    if(template_ == ""){
        if(gammavalue != ""){
            pszPathToFile = CPLSPrintf("ninjafoam/0/%s", "genericTypeVal.tmp");
        }
        else if(inletoutletvalue != ""){
            pszPathToFile = CPLSPrintf("ninjafoam/0/%s", "genericType.tmp");
        }
        else{
            pszPathToFile = CPLSPrintf("ninjafoam/0/%s", "genericType-kep.tmp");
        }
    }
    else{
        pszPathToFile = CPLSPrintf("ninjafoam/0/%s", template_.c_str());
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

    /*
     * set roughness to 0.01 regardless of veg type until we fix how roughness
     * is handled in the OpenFOAM BCs and turbulence model
     */
    ReplaceKeys(s, "$z0$", boost::lexical_cast<std::string>( foamRoughness ));
    //ReplaceKeys(s, "$z0$", boost::lexical_cast<std::string>( input.surface.Roughness.get_meanValue() ));

    ReplaceKeys(s, "$Rd$", boost::lexical_cast<std::string>( input.surface.Rough_d.get_meanValue() ));
    ReplaceKeys(s, "$inletoutletvalue$", inletoutletvalue);

    dataString.append(s);

    CPLFree(data);
    VSIFCloseL(fin);

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteZeroFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
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

    if(input.nonEqBc == 0){
        if(std::string(pszFilename) == "epsilon"){
            ReplaceKeys(s, "$wallFunction$", "epsilonWallFunction");
        }
        else if(std::string(pszFilename) == "nut"){
            ReplaceKeys(s, "$wallFunction$", "nutkWallFunction");
        }
    }
    else{
        if(std::string(pszFilename) == "epsilon"){
            ReplaceKeys(s, "$wallFunction$", "epsilonNonEquiWallFunction");
        }
        else if(std::string(pszFilename) == "nut"){
            ReplaceKeys(s, "$wallFunction$", "nutNonEquiWallFunction");
        }
    }

    dataString.append(s);

    const char * d = dataString.c_str();
    int nSize = strlen(d);

    VSIFWriteL( d, nSize, 1, fout );

    CPLFree(data);

    VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
    VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteSystemFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
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
    else if(std::string(pszFilename) == "sampleDict"){
        std::string t = NinjaRemoveSpaces(std::string(CPLGetBasename(input.dem.fileName.c_str())));
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

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteConstantFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
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

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteFoamFiles()
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
    pszArchive = CPLSPrintf("%s/ninjafoam", pszPath);
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
            pszArchive = CPLSPrintf("%s/ninjafoam", pszPath);
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

    return NINJA_SUCCESS;
}

void NinjaFoam::SetFoamPath(const char* pszPath)
{
    pszFoamPath = pszPath;

}

int NinjaFoam::GenerateFoamDirectory(std::string demName)
{
    std::string t = NinjaRemoveSpaces(std::string(CPLGetBasename(demName.c_str())));
    pszFoamPath = CPLStrdup(CPLGenerateTempFilename( CPLSPrintf("NINJAFOAM_%s_", t.c_str())));
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

int NinjaFoam::WriteEpsilonBoundaryField(std::string &dataString)
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
        int status;
        //append BC block for current face
        status = AddBcBlock(dataString);
        if(status != 0){
            //do something
        }
    }

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteKBoundaryField(std::string &dataString)
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
        int status;
        //append BC block for current face
        status = AddBcBlock(dataString);
        if(status != 0){
            //do something
        }
    }

    return NINJA_SUCCESS;
}

int NinjaFoam::WritePBoundaryField(std::string &dataString)
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
        int status;
        //append BC block for current face
        status = AddBcBlock(dataString);
        if(status != 0){
            //do something
        }
    }

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteUBoundaryField(std::string &dataString)
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
        int status;
        status = AddBcBlock(dataString);
        if(status != 0){
            //do something
        }
    }

    return NINJA_SUCCESS;
}

int NinjaFoam::readDem(double &expansionRatio)
{
    // get some info from the DEM
    double dz = input.dem.get_maxValue() - input.dem.get_minValue();
    double dx = input.dem.get_xDimension();
    double dy = input.dem.get_yDimension();
    double xBuffer, yBuffer;
    
    xBuffer = input.dem.get_cellSize(); // buffers for MDM
    yBuffer = input.dem.get_cellSize();
    
    double blockMeshDz = max((0.1 * max(dx, dy)), (dz + 0.1 * dz));

    bbox.push_back( input.dem.get_xllCorner() + xBuffer ); //xmin 
    bbox.push_back( input.dem.get_yllCorner() + yBuffer ); //ymin
    bbox.push_back( input.dem.get_maxValue() + 0.05 * blockMeshDz ); //zmin (should be above highest point in DEM for MDM)
    bbox.push_back( input.dem.get_xllCorner() + input.dem.get_xDimension() - xBuffer ); //xmax
    bbox.push_back( input.dem.get_yllCorner() + input.dem.get_yDimension() - yBuffer ); //ymax
    bbox.push_back( input.dem.get_maxValue() + blockMeshDz ); //zmax

    double meshVolume;
    double cellVolume;
    double side;

    meshVolume = (bbox[3] - bbox[0]) * (bbox[4] - bbox[1]) * (bbox[5] - bbox[2]); // total volume for block mesh
    cellCount = 0.5 * input.meshCount; //half the cells in the blockMesh and half reserved for refineMesh
    cellVolume = meshVolume/cellCount; // volume of 1 cell
    side = std::pow(cellVolume, (1.0/3.0)); // length of side of regular hex cell
    meshResolution = side;

    nCells.push_back(int( (bbox[3] - bbox[0]) / side)); // Nx1
    nCells.push_back(int( (bbox[4] - bbox[1]) / side)); // Ny1
    nCells.push_back(int( (bbox[5] - bbox[2]) / side)); // Nz1

    initialFirstCellHeight = ((bbox[5] - bbox[2]) / nCells[2]); //height of first cell
    expansionRatio = 1.0;
    
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
    
    CPLDebug("NINJAFOAM", "meshVolume = %f", meshVolume);
    CPLDebug("NINJAFOAM", "firstCellHeight = %f", initialFirstCellHeight);
    CPLDebug("NINJAFOAM", "side = %f", side);
    CPLDebug("NINJAFOAM", "expansionRatio = %f", expansionRatio);
    
    CPLDebug("NINJAFOAM", "Nx1 = %d", nCells[0]);
    CPLDebug("NINJAFOAM", "Ny1 = %d", nCells[1]);
    CPLDebug("NINJAFOAM", "Nz1 = %d", nCells[2]);
    
    CPLDebug("NINJAFOAM", "xmin = %f", bbox[0]);
    CPLDebug("NINJAFOAM", "ymin = %f", bbox[1]);
    CPLDebug("NINJAFOAM", "zmin = %f", bbox[2]);
    CPLDebug("NINJAFOAM", "xmax = %f", bbox[3]);
    CPLDebug("NINJAFOAM", "ymax = %f", bbox[4]);
    CPLDebug("NINJAFOAM", "zmax = %f", bbox[5]);

    return NINJA_SUCCESS;
}

int NinjaFoam::writeBlockMesh()
{
    const char *pszInput;
    const char *pszOutput;
    const char *pszPath;
    const char *pszArchive;
    double ratio_;
    int status;

    status = readDem(ratio_);
    if(status != 0){
        //do something
    }

    pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    pszArchive = CPLSPrintf("%s/ninjafoam", pszPath);

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

    return NINJA_SUCCESS;
}

int NinjaFoam::writeMoveDynamicMesh()
{
    VSILFILE *fin;
    VSILFILE *fout;

    const char *pszPath;
    const char *pszArchive;
    const char *pszInput;
    const char *pszOutput;

    pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    pszArchive = CPLSPrintf("%s/ninjafoam", pszPath);

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

    std::string t = NinjaRemoveSpaces(std::string(CPLGetBasename(input.dem.fileName.c_str())));
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
    
    return NINJA_SUCCESS;
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

int NinjaFoam::CopyFile(const char *pszInput, const char *pszOutput, std::string key, std::string value)
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
    
    return NINJA_SUCCESS;
}

int NinjaFoam::SurfaceTransformPoints()
{
    int nRet = -1;

    std::string stl = NinjaRemoveSpaces(std::string(CPLSPrintf("%s/constant/triSurface/%s.stl",
                    pszFoamPath,
                    CPLGetBasename(input.dem.fileName.c_str()))));
    std::string stlOut = NinjaRemoveSpaces(std::string(CPLSPrintf("%s/constant/triSurface/%s_out.stl",
                    pszFoamPath,
                    CPLGetBasename(input.dem.fileName.c_str()))));

    const char *const papszArgv[] = { "surfaceTransformPoints",
                                      "-case",
                                      pszFoamPath,
                                      "-translate",
                                      CPLSPrintf("(0 0 %.0f)", input.outputWindHeight),
                                      (const char*)stl.c_str(),
                                      (const char*)stlOut.c_str(),
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "surfaceTransformPoints.log", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE); //create output surface stl in pszTemppath/constant/triSurface

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::SurfaceCheck()
{
    int nRet = -1;

    std::string stlName = NinjaRemoveSpaces(std::string(CPLGetBasename(input.dem.fileName.c_str())));

    const char *const papszArgv[] = { "surfaceCheck",
                                      "-case",
                                      pszFoamPath,
                                      CPLSPrintf("%s/constant/triSurface/%s.stl", pszFoamPath, stlName.c_str()),
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.json", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE); //writes log.json used in mesh file writing

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::MoveDynamicMesh()
{
    int nRet = -1;

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Running blockMesh...");
    nRet = BlockMesh();
    if(nRet != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during blockMesh().");
    }

    VSILFILE *fout;

    const char *pszInput;
    const char *pszOutput;
    
    std::string s, ss;

    if(input.numberCPUs > 1){

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel mesh calculations...");
        nRet = DecomposePar();
        if(nRet != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during decomposePar().");
            return NINJA_E_OTHER;
        }

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
            CPLDebug("NINJAFOAM", "moveDynamicMesh: %s", data);
            s.append(data);

            /* eventually set up to stop at resid < 1e-6, a little complicated...for now just stop at 100 */
            /*if(s.find("Initial residual") != s.npos){
                pos = s.rfind("Initial residual");
                    if( s.find(',', pos) != s.npos ){ // if not at the end of the string
                    int nchar = s.find(',', pos) - (pos+19);
                    std::string resid = s.substr( pos+19, s.find(',', pos)-(pos+19) );
                    if(atof(resid.c_str()) < 1e-6){
                        /*
                         * change endTime = writeNow in system/controlDict
                         * also need to know what last written time step is since 0/* needs to be copied here after reoncstructPar/
                         * right now only set to write every 10th time step, so it's not simply latestTime
                         */
                        /*input.Com->ninjaCom(ninjaComClass::ninjaNone, "(moveDynamicMesh) 100%% complete...");
                    }
                }
            }*/

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
            //do something
        }

        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");
        nRet = ReconstructPar();
        if(nRet != 0){
            //do something
        }
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
            //do something
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
    UpdateDictFiles();

    return nRet;
}

int NinjaFoam::RefineSurfaceLayer(){    
    const char *pszInput;
    const char *pszOutput;
    int nRet = 0;
    
    //write topoSetDict
    pszInput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");
    pszOutput = CPLFormFilename(pszFoamPath, "system/topoSetDict", "");

    std::string stlName = NinjaRemoveSpaces(std::string(CPLGetBasename(input.dem.fileName.c_str())));

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

    while(cellCount < input.meshCount){ 
        nRet = TopoSet();
        if(nRet != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during TopoSet().");
            return nRet;
        }
        nRet = RefineMesh();
        if(nRet != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during RefineMesh().");
            return nRet;
        }
        CheckMesh(); //update cellCount
        
        //update time, near-wall cell height, BC files, topoSetDict file
        latestTime += 1;
        oldFirstCellHeight = finalFirstCellHeight;
        finalFirstCellHeight /= 2.0; //keep track of first cell height
        meshResolution /= 2.0;
        
        UpdateDictFiles();
        
        CopyFile(pszInput, pszOutput, 
                CPLSPrintf("nearDistance    %.2f", oldFirstCellHeight),
                CPLSPrintf("nearDistance    %.2f", finalFirstCellHeight));
        
        CPLDebug("NINJAFOAM", "finalFirstCellHeght = %f", finalFirstCellHeight);

        percentDone = 100.0 - double(input.meshCount - cellCount) / double(input.meshCount) * 100.0;

        if(percentDone < 100.0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "(refineMesh) %.0f%% complete...", percentDone);
        }
    }

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(refineMesh) 99%% complete...");
    
    CPLDebug("NINJAFOAM", "firstCellHeight = %f", initialFirstCellHeight);
    CPLDebug("NINJAFOAM", "finalFirstCellHeight = %f", finalFirstCellHeight);
        
    return nRet;
}

void NinjaFoam::UpdateDictFiles()
{
    /* update simpleFoam controlDict writeInterval */
    UpdateSimpleFoamControlDict();

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
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/p", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%s/p", boost::lexical_cast<std::string>(latestTime).c_str()),  ""));
}

void NinjaFoam::UpdateSimpleFoamControlDict()
{
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

int NinjaFoam::TopoSet()
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

    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::RefineMesh()
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

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::BlockMesh()
{
    int nRet = -1;
    char* currentDir = CPLGetCurrentDir();
    const char *const papszArgv[] = { "blockMesh", 
                                    "-case",
                                    pszFoamPath,
                                    NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.blockMesh", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::DecomposePar()
{
    int nRet = -1;

    const char *const papszArgv[] = { "decomposePar", 
                                      "-case",
                                      pszFoamPath,
                                      "-force", 
                                      NULL };
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.decomposePar", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::ReconstructParMesh()
{
    int nRet = -1;

    const char *const papszArgv[] = { "reconstructParMesh", 
                                      "-case",
                                      pszFoamPath,
                                      "-latestTime", 
                                      NULL };
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.reconstructParMesh", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::ReconstructPar()
{
    int nRet = -1;

    const char *const papszArgv[] = { "reconstructPar", 
                                      "-case",
                                      pszFoamPath,
                                      "-latestTime",
                                      NULL };
    
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.reconstructPar", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::RenumberMesh()
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

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::CheckMesh()
{
    int nRet = -1;

    const char *const papszArgv[] = { "checkMesh",
                                      "-latestTime",
                                      "-case",
                                      pszFoamPath,
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.checkmesh", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);
    
    //update cellCount from log.checkmesh
    VSILFILE *fin;

    const char *pszInput;

    pszInput = CPLFormFilename(pszFoamPath, "log.checkmesh", "");

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
    int pos, endPos;
    int found;
    pos = s.find("cells:");
    if(pos != s.npos){
        cellCount = atof(s.substr(pos+7, (s.find("\n", pos+7) - (pos+7))).c_str());
        CPLDebug("NINJAFOAM", "cellCount = %d", cellCount);
    }

    CPLFree(data);
    VSIFCloseL(fin);

    return nRet;
}

int NinjaFoam::ApplyInit()
{
    int nRet = -1;

    const char *const papszArgv[] = { "applyInit", 
                                      "-case",
                                      pszFoamPath,
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.applyInit", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::SimpleFoam()
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
            CPLDebug("NINJAFOAM", "simpleFoam: %s", data);
            if(s.rfind("smoothSolver") != s.npos){
                startPos = s.rfind("smoothSolver");
                pos = s.rfind("Time = ", startPos);
                if(pos != s.npos && s.npos > (pos + 12) && s.rfind("\n", pos) == (pos-1)){
                    t = s.substr(pos+7, (s.find("\n", pos+7) - (pos+7)));
                    //number of iterations is set equal to the write interval
                    p = atof(t.c_str()) / simpleFoamEndTime * 100;
                    input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d", (int)p);
                }
            }
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
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
    }
    
    // write simpleFoam stdout to a log file 
    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.simpleFoam", ""), "w");
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
    VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::Sample()
{
    int nRet = -1;

    const char *const papszArgv[] = { "sample", 
                                      "-case",
                                      pszFoamPath,
                                      "-latestTime", 
                                      NULL };

    VSILFILE *fout = VSIFOpenL(CPLFormFilename(pszFoamPath, "log.sample", ""), "w");

    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);

    VSIFCloseL(fout);

    return nRet;
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
    FILE *fin;
    VSILFILE *fout, *fvrt;
    char buf[512];
    int rc;
    const char *pszVrtFile;
    const char *pszVrt;
    const char *pszMem;
    std::string s;

    pszMem = CPLSPrintf( "%s/output.raw", pszFoamPath );
    /* This is a member, hold on to it so we can read it later */
    pszVrtMem = CPLStrdup( CPLSPrintf( "%s/output.vrt", pszFoamPath ) );

    char **papszOutputSurfacePath;
    papszOutputSurfacePath = VSIReadDir( CPLSPrintf("%s/postProcessing/surfaces/", pszFoamPath) );

    for(int i = 0; i < CSLCount( papszOutputSurfacePath ); i++){
        if(std::string(papszOutputSurfacePath[i]) != "." &&
           std::string(papszOutputSurfacePath[i]) != "..") {
            fin = VSIFOpen(CPLSPrintf( "%s/postProcessing/surfaces/%s/U_triSurfaceSampling.raw", 
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
    if( !fout )
    {
        VSIFClose( fin );
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open output file for " \
                                                "writing." );
        return NINJA_E_FILE_IO;
    }
    if( !fvrt )
    {
        VSIFClose( fin );
        VSIFClose( fout );
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open vrt file for " \
                                                "writing." );
        return NINJA_E_FILE_IO;
    }
    pszVrtFile = CPLSPrintf( "CSV:%s", pszMem );

    pszVrt = CPLSPrintf( NINJA_FOAM_OGR_VRT, "output", pszVrtFile, "output" );

    VSIFWriteL( pszVrt, strlen( pszVrt ), 1, fvrt );
    VSIFCloseL( fvrt );
    buf[0] = '\0';
    /*
    ** eat the first line
    */
    VSIFGets( buf, 512, fin );
    /*
    ** fix the header
    */
    VSIFGets( buf, 512, fin );

    s = buf;
    ReplaceKeys( s, "#", "", 1 );
    ReplaceKeys( s, "  ", "", 1 );
    ReplaceKeys( s, "  ", ",", 5 );
    ReplaceKeys( s, "  ", "", 1 );
    VSIFWriteL( s.c_str(), s.size(), 1, fout );
    /*
    ** sanitize the data.
    */
    while( VSIFGets( buf, 512, fin ) != NULL )
    {
        s = buf;
        ReplaceKeys( s, " ", ",", 5 );
        VSIFWriteL( s.c_str(), s.size(), 1, fout );
    }
    VSIFClose( fin );
    VSIFCloseL( fout );

    return 0;
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

    dfXMin = input.dem.get_xllCorner();
    dfXMax = input.dem.get_xllCorner() + input.dem.get_xDimension();
    dfYMin = input.dem.get_yllCorner();
    dfYMax = input.dem.get_yllCorner() + input.dem.get_yDimension();
    dfCellSize = input.dem.get_cellSize();

    nPoints = OGR_L_GetFeatureCount( hLayer, TRUE );
    CPLDebug( "WINDNINJA", "NinjaFoam gridding %d points", nPoints );

    /* Get DEM/output specs */
    nXSize = input.dem.get_nCols();
    nYSize = input.dem.get_nRows();

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
    rc = GDALSetProjection( hGriddedDS, input.dem.prjString.c_str() );

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
        GDALRasterIO( hUBand, GF_Write, nPixel, nLine, 1, 1, &dfU,
                      1, 1, GDT_Float64, 0, 0 );
        GDALRasterIO( hVBand, GF_Write, nPixel, nLine, 1, 1, &dfV,
                      1, 1, GDT_Float64, 0, 0 );
        i++;
    }
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
    double *padfX, *padfY, *padfU, *padfV;
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

    int i = 0;
    int nUIndex, nVIndex;
    OGR_L_ResetReading( hLayer );
    hFeatDefn = OGR_L_GetLayerDefn( hLayer );
    nUIndex = OGR_FD_GetFieldIndex( hFeatDefn, "U" );
    nVIndex = OGR_FD_GetFieldIndex( hFeatDefn, "V" );
    while( (hFeature = OGR_L_GetNextFeature( hLayer )) != NULL )
    {
        hGeometry = OGR_F_GetGeometryRef( hFeature );
        padfX[i] = OGR_G_GetX( hGeometry, 0 );
        padfY[i] = OGR_G_GetY( hGeometry, 0 );
        padfU[i] = OGR_F_GetFieldAsDouble( hFeature, nUIndex );
        padfV[i] = OGR_F_GetFieldAsDouble( hFeature, nVIndex );
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
    hGriddedDS = GDALCreate( hDriver, pszGridFilename, nXSize, nYSize, 2,
                             GDT_Float64, NULL );
    padfData = (double*)CPLMalloc( sizeof( double ) * nXSize * nYSize );

    /* U field */
    rc = GDALGridCreate( GGA_NearestNeighbor, (void*)&sOptions, nPoints,
                         padfX, padfY, padfU, dfXMin, dfXMax, dfYMax, dfYMin,
                         nXSize, nYSize, GDT_Float64, padfData, NULL, NULL );

    GDALRasterBandH hBand;
    hBand = GDALGetRasterBand( hGriddedDS, 1 );
    GDALSetRasterNoDataValue( hBand, -9999 );
    GDALRasterIO( hBand, GF_Write, 0, 0, nXSize, nYSize, padfData,
                  nXSize, nYSize, GDT_Float64, 0, 0 );

    /* V field */
    rc = GDALGridCreate( GGA_NearestNeighbor, (void*)&sOptions, nPoints,
                         padfX, padfY, padfV, dfXMin, dfXMax, dfYMax, dfYMin,
                         nXSize, nYSize, GDT_Float64, padfData, NULL, NULL );

    hBand = GDALGetRasterBand( hGriddedDS, 2 );
    GDALSetRasterNoDataValue( hBand, -9999 );
    GDALRasterIO( hBand, GF_Write, 0, 0, nXSize, nYSize, padfData,
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

    CPLFree( (void*)padfData );
    OGR_DS_Destroy( hDS );
    GDALClose( hGriddedDS );

    return 0;
}

const char * NinjaFoam::GetGridFilename()
{
    return pszGridFilename;
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
    std::string rootFile, rootName, fileAppend, kmz_fileAppend, \
        shp_fileAppend, ascii_fileAppend, mesh_units, kmz_mesh_units, \
        shp_mesh_units, ascii_mesh_units, pdf_fileAppend, pdf_mesh_units;

    std::string pathName;
    std::string baseName(CPLGetBasename(input.dem.fileName.c_str()));

    if(input.customOutputPath == "!set"){
        pathName = CPLGetPath(input.dem.fileName.c_str());
    }
    else{
        pathName = input.customOutputPath;
    }
    
    rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);

    /* set the output path member variable */
    input.outputPath = pathName;
    set_outputPath(pathName);

    mesh_units = "m";
    kmz_mesh_units = lengthUnits::getString( input.kmzUnits );
    shp_mesh_units = lengthUnits::getString( input.shpUnits );
    ascii_mesh_units = lengthUnits::getString( input.velOutputFileDistanceUnits );
    pdf_mesh_units   = lengthUnits::getString( input.pdfUnits );

    ostringstream os, os_kmz, os_shp, os_ascii, os_pdf;

    double tempSpeed = input.inputSpeed;
    velocityUnits::fromBaseUnits(tempSpeed, input.inputSpeedUnits);
    os << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
    os_kmz << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
    os_shp << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
    os_ascii << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);
    os_pdf << "_" << (long) (input.inputDirection+0.5) << "_" << (long) (tempSpeed+0.5);

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

    os << "_" << (long) (meshResolutionTemp+0.5)  << mesh_units;
    os_kmz << "_" << (long) (kmzResolutionTemp+0.5)  << kmz_mesh_units;
    os_shp << "_" << (long) (shpResolutionTemp+0.5)  << shp_mesh_units;
    os_ascii << "_" << (long) (velResolutionTemp+0.5)  << ascii_mesh_units;
    os_pdf << "_" << (long) (pdfResolutionTemp+0.5)    << pdf_mesh_units;

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


int NinjaFoam::SampleRawOutput()
{
    /*-------------------------------------------------------------------*/
    /* convert output from xyz to speed and direction                    */
    /*-------------------------------------------------------------------*/

    AsciiGrid<double> foamU, foamV;
    int rc;
    rc = SanitizeOutput();

    if( CSLTestBoolean( CPLGetConfigOption( "NINJAFOAM_USE_GDALGRID", "NO" ) ) )
        rc = SampleCloudGrid();
    else
        rc = SampleCloud();
    GDALDatasetH hDS;
    hDS = GDALOpen( GetGridFilename(), GA_ReadOnly );
    if( hDS == NULL )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Invalid output written" );
        return false;
    }

    GDAL2AsciiGrid( (GDALDataset *)hDS, 1, foamU );
    GDAL2AsciiGrid( (GDALDataset *)hDS, 2, foamV );

    AsciiGrid<double> foamSpd( foamU );
    AsciiGrid<double> foamDir( foamU );

    for(int i=0; i<foamU.get_nRows(); i++)
    {
        for(int j=0; j<foamU.get_nCols(); j++)
        {
            wind_uv_to_sd(foamU(i,j), foamV(i,j), &(foamSpd)(i,j), &(foamDir)(i,j));
        }
    }

    AngleGrid = foamDir;
    VelocityGrid = foamSpd;
    if(VelocityGrid.get_maxValue() > 220.0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "The flow solution did not converge. This may occasionally "
                "happen in very complex terrain when the mesh resolution is high. Try the simulation "
                "again with a coarser mesh.");
        return(NINJA_E_OTHER);
    }

    GDALClose( hDS );

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteOutputFiles()
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

int NinjaFoam::UpdateExistingCase()
{
    int status = 0;

    status = CheckForValidDem();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, CPLSPrintf("The DEM, '%s' does not correspond "
                "to the supplied case directory, '%s'", input.dem.fileName.c_str(), pszFoamPath));
        return NINJA_E_OTHER;
    }

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Using existing case directory...");
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Updating case files...");

    //set meshResolution from log.ninja
    const char *pszInput = CPLSPrintf("%s/log.ninja", pszFoamPath);
    VSILFILE *fin;
    fin = VSIFOpenL(pszInput, "r");

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

    std::string h;
    int pos;
    if(s.find("meshResolution") != s.npos){
        pos = s.find("firstCellHeight ");
        h = s.substr(pos+18, pos+23);
    }

    meshResolution = atof(h.c_str());

    //write the new dict files
    status = WriteFoamFiles();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during WriteFoamFiles().");
        return NINJA_E_OTHER;
    }
    
    //rm latestTime in case (old flow solution)
    latestTime = GetLatestTimeOnDisk();
    NinjaUnlinkTree(CPLSPrintf("%s/%d", pszFoamPath, latestTime));

    //now latestTime on disk is where we want to start (the mesh is here)
    latestTime = GetLatestTimeOnDisk();

    //read in firstCellHeight from latestTime dir on disk
    finalFirstCellHeight = GetFirstCellHeightFromDisk(); 

    //Get rid of -9999.9 in 0/ files
    CopyFile(CPLFormFilename(pszFoamPath, "0/U", ""), 
            CPLFormFilename(pszFoamPath, "0/U", ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", finalFirstCellHeight)); //just use final cell height here for now
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/k", ""), 
            CPLFormFilename(pszFoamPath, "0/k", ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", finalFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/epsilon", ""), 
            CPLFormFilename(pszFoamPath, "0/epsilon", ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", finalFirstCellHeight));

    //copy 0/ to latestTime/
    CopyFile(CPLFormFilename(pszFoamPath, "0/U", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%d/U", latestTime), ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", finalFirstCellHeight));

    CopyFile(CPLFormFilename(pszFoamPath, "0/k", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%d/k", latestTime), ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", finalFirstCellHeight));
            
    CopyFile(CPLFormFilename(pszFoamPath, "0/epsilon", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%d/epsilon", latestTime), ""), 
            "-9999.9", 
            CPLSPrintf("%.2f", finalFirstCellHeight));

    CopyFile(CPLFormFilename(pszFoamPath, "0/p", ""), 
            CPLFormFilename(pszFoamPath, CPLSPrintf("%d/p", latestTime), "")); 

    //update controlDict
    CopyFile(CPLSPrintf("%s/system/controlDict_simpleFoam", pszFoamPath),
            CPLSPrintf("%s/system/controlDict", pszFoamPath));
    CopyFile(CPLSPrintf("%s/system/controlDict", pszFoamPath),
            CPLSPrintf("%s/system/controlDict", pszFoamPath),
            "startFrom       latestTime", "startFrom       startTime");
    CopyFile(CPLSPrintf("%s/system/controlDict", pszFoamPath),
            CPLSPrintf("%s/system/controlDict", pszFoamPath),
            "latestTime", CPLSPrintf("%d", latestTime));

    //update endTime
    UpdateSimpleFoamControlDict();

    //rm any processor* directories
    std::vector<std::string> dirList = GetProcessorDirsOnDisk();
    for(int n=0; n<dirList.size(); n++){
        NinjaUnlinkTree( CPLSPrintf( "%s/processor%d", pszFoamPath, n) );
    }

    return NINJA_SUCCESS;
}

int NinjaFoam::GenerateNewCase()
{
    #ifdef _OPENMP
    startFoamFileWriting = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing OpenFOAM files...");

    int status = 0;

    //writes *most* of the foam files, but not all can be written at this point
    status = WriteFoamFiles();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during WriteFoamFiles().");
        return NINJA_E_OTHER;
    }

    //write controlDict for flow solution--this will get modified during moveDynamicMesh
    const char *pszInput = CPLFormFilename(pszFoamPath, "system/controlDict_simpleFoam", "");
    const char *pszOutput = CPLFormFilename(pszFoamPath, "system/controlDict", "");
    CopyFile(pszInput, pszOutput);

    checkCancel();

    /*-------------------------------------------------------------------*/
    /*  convert DEM to STL format and write to constant/triSurface       */
    /*-------------------------------------------------------------------*/

    #ifdef _OPENMP
    startStlConversion = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Converting DEM to STL format...");

    const char *pszStlFileName = CPLStrdup(CPLFormFilename(
                (CPLSPrintf("%s/constant/triSurface/", pszFoamPath)),
                CPLGetBasename(input.dem.fileName.c_str()), ".stl"));

    std::string stlName = NinjaRemoveSpaces(std::string(pszStlFileName));

    int nBand = 1;
    const char * inFile = input.dem.fileName.c_str();
    CPLErr eErr;

    eErr = NinjaElevationToStl(inFile,
                        (const char*)stlName.c_str(),
                        nBand,
                        input.dem.get_cellSize(),
                        NinjaStlBinary,
                        //NinjaStlAscii,
                        NULL);

    CPLFree((void*)pszStlFileName);

    if(eErr != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error while converting DEM to STL format.");
        return NINJA_E_OTHER;
    }

    checkCancel();

    #ifdef _OPENMP
    endStlConversion = omp_get_wtime();
    #endif

    /*-------------------------------------------------------------------*/
    /*  write output stl and run surfaceCheck on original stl            */
    /*-------------------------------------------------------------------*/

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Transforming surface points to output wind height...");
    status = SurfaceTransformPoints();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during surfaceTransformPoints().");
        return NINJA_E_OTHER;
    }


    checkCancel();
	
    if( atoi( CPLGetConfigOption("WRITE_FOAM_FILES", "-1") ) == 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "WRITE_FOAM_FILES set to 0. STL surfaces written.");
        return true;
    }

    /*-------------------------------------------------------------------*/
    /*  write necessary mesh file(s)                                     */
    /*-------------------------------------------------------------------*/

    //reads from log.json created from surfaceCheck if DEM not available
    status = writeBlockMesh();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during writeBlockMesh().");
        return NINJA_E_OTHER;
    }
    status = writeMoveDynamicMesh();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during writeMoveDynamicMesh().");
        return NINJA_E_OTHER;
    }

    #ifdef _OPENMP
    endFoamFileWriting = omp_get_wtime();
    #endif
	
    if( atoi( CPLGetConfigOption("WRITE_FOAM_FILES", "-1") ) == 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "WRITE_FOAM_FILES set to 1. Mesh dict files written.");
        return true;
    }

    checkCancel();

    /*-------------------------------------------------------------------*/
    /* create the mesh                                                   */
    /*-------------------------------------------------------------------*/

    #ifdef _OPENMP
    startMesh = omp_get_wtime();
    #endif

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating mesh...");

    status = MoveDynamicMesh();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during moveDynamicMesh().");
        return NINJA_E_OTHER;
    }
    
    checkCancel();

    /*refine mesh near the ground */
    status = RefineSurfaceLayer();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during RefineSurfaceLayer().");
        return NINJA_E_OTHER;
    }

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Renumbering mesh...");
    status = RenumberMesh();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during RenumberMesh().");
        return NINJA_E_OTHER;
    }

    if( atoi( CPLGetConfigOption("WRITE_FOAM_FILES", "-1") ) == 2){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "WRITE_FOAM_FILES set to 2. Mesh written.");
        return true;
    }

    //write log.ninja
    status = WriteNinjaLog();
    if(status != 0){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error writing log.ninja.");
    }

    checkCancel();

    return NINJA_SUCCESS;
}

int NinjaFoam::WriteNinjaLog()
{
    //write log.ninja to store info needed for reusing cases
    const char *pszInput = CPLSPrintf("%s/log.ninja", pszFoamPath);
    VSILFILE *fout;
    fout = VSIFOpenL(pszInput, "w");
    if( !fout ){
        return NINJA_E_OTHER;
    }
    const char *d = CPLSPrintf("meshResolution = %.2f", meshResolution);
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
    VSIFCloseL(fout);

    return NINJA_SUCCESS;
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

double NinjaFoam::GetFirstCellHeightFromDisk()
{
    int time = GetLatestTimeOnDisk();

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

    double height = atoi(h.c_str());

    return height;
}

int NinjaFoam::CheckForValidCaseDir(const char* dir)
{
    //check at least for the controlDict file
    if( !CPLCheckForFile((char*)CPLSPrintf("%s/system/controlDict", dir), NULL) ){
        return NINJA_E_OTHER;
    }

    return NINJA_SUCCESS;
}

int NinjaFoam::CheckForValidDem()
{
    char **papszFileList;
    const char *pszFilename;
    papszFileList = VSIReadDir( CPLSPrintf("%s/constant/triSurface", pszFoamPath ) );

    for(int i=0; i<CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename( papszFileList[i] );
        std::string s(pszFilename);
        std::string ss = NinjaRemoveSpaces(CPLGetBasename( input.dem.fileName.c_str() ));
        if( s.find(".stl") != s.npos & s.find("_out.stl") == s.npos){
            s = (CPLGetBasename(pszFilename));
            if( s.compare(ss) != s.npos ){
                CSLDestroy( papszFileList );
                return NINJA_SUCCESS;
            }
        }
    }

    CSLDestroy( papszFileList );

    return NINJA_E_OTHER;
}
