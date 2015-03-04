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

NinjaFoam::NinjaFoam() : ninja()
{
    pszTempPath = NULL;
    pszVrtMem = NULL;
    pszGridFilename = NULL;

    boundary_name = "";
    terrainName = "NAME";
    type = "";
    value = "";
    gammavalue = "";
    pvalue = "";
    inletoutletvalue = "";
    template_ = "";

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
    CPLFree( (void*)pszTempPath );
    CPLFree( (void*)pszVrtMem );
    CPLFree( (void*)pszGridFilename );
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
    
    checkCancel();

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);

    /*------------------------------------------*/
    /*  write OpenFOAM files                    */
    /*------------------------------------------*/
    
    CPLDebug("NINJAFOAM", "meshCount = %d", input.meshCount);
    CPLDebug("NINJAFOAM", "Rd = %lf", input.surface.Rough_d(0,0));
    CPLDebug("NINJAFOAM", "z0 = %lf", input.surface.Roughness(0,0));
    CPLDebug("NINJAFOAM", "input speed = %lf", input.inputSpeed);
    CPLDebug("NINJAFOAM", "input direction = %lf", input.inputDirection);
    CPLDebug("NINJAFOAM", "foam direction = (%lf, %lf, %lf)", direction[0], direction[1], direction[2]);
    CPLDebug("NINJAFOAM", "number of inlets = %ld", inlets.size());
    
    #ifdef _OPENMP
    startFoamFileWriting = omp_get_wtime();
	#endif
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing OpenFOAM files...");

    int status;
    
    status = GenerateTempDirectory();
    if(status != 0){
        //do something
    }
    
    //writes *most* of the foam files, but not all can be written at this point
    status = WriteFoamFiles();
    if(status != 0){
        //do something
    }
    
    //write controlDict for flow solution--this will get modified during moveDynamicMesh
    const char *pszInput = CPLFormFilename(pszTempPath, "system/controlDict_simpleFoam", "");
    const char *pszOutput = CPLFormFilename(pszTempPath, "system/controlDict", "");
    CopyFile(pszInput, pszOutput);
    
    checkCancel();

    /*-------------------------------------------------------------------*/
    /*  convert DEM to STL format and write to constant/triSurface       */
    /*-------------------------------------------------------------------*/

    #ifdef _OPENMP
    startStlConversion = omp_get_wtime();
	#endif
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Converting DEM to STL format...");

    const char *pszShortName = CPLGetBasename(input.dem.fileName.c_str());
    const char *pszStlPath = CPLStrdup( CPLSPrintf("%s/constant/triSurface/", pszTempPath) );
    const char *pszStlFileName = CPLFormFilename(pszStlPath, pszShortName, ".stl");

    int nBand = 1;
    const char * inFile = input.dem.fileName.c_str();
    const char * outFile = pszStlFileName;

    CPLErr eErr;

    eErr = NinjaElevationToStl(inFile,
                        outFile,
                        nBand,
                        NinjaStlBinary,
                        //NinjaStlAscii,
                        NULL);

    if(eErr != 0){
        //do something
    }
    
    checkCancel();
    
    if(input.stlFile != "!set"){
        status = ReadStl();
        
        if(status != 0){
        //do something
        }
    }

    /*-------------------------------------------------------------------*/
    /*  write output stl and run surfaceCheck on original stl            */
    /*-------------------------------------------------------------------*/
       
    //set working directory to pszTempPath
    status = chdir(pszTempPath);
    if(status != 0){
        //do something
    }
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Transforming surface points to output wind height...");
    status = SurfaceTransformPoints();
    if(status != 0){
        //do something
    }
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Checking surface points in original terrain file...");
    status = SurfaceCheck();
    if(status != 0){
        //do something
    }
    
    //move back to ninja working directory
    status = chdir("../");
    if(status != 0){
        //do something
    } 

    checkCancel();
    
    #ifdef _OPENMP
    endStlConversion = omp_get_wtime();
	#endif
	
	/*-------------------------------------------------------------------*/
    /*  write necessary mesh file(s)                                     */
    /*-------------------------------------------------------------------*/
	
    CPLDebug("NINJAFOAM", "input.meshType = %d", input.meshType);
    
    if(input.meshType == WindNinjaInputs::MDM){ //use moveDynamicMesh
        //reads from log.json created from surfaceCheck
        status = writeBlockMesh();
        if(status != 0){
            //do something
        }
        status = writeMoveDynamicMesh();
        if(status != 0){
            //do something
        }
    }
    else{ //use snappyHexMesh 
        const char *pszInput;
        const char *pszOutput;
    
        pszInput = CPLFormFilename(pszTempPath, "system/controlDict_simpleFoam", "");
        pszOutput = CPLFormFilename(pszTempPath, "system/controlDict", "");
        CopyFile(pszInput, pszOutput); //write controlDict for flow solution
        
        //reads from log.json created from surfaceCheck
        status = writeBlockMesh();
        if(status != 0){
            //do something
        }
    
        status = writeSnappyMesh();
        if(status != 0){
            //do something
        }
    }
    
    #ifdef _OPENMP
    endFoamFileWriting = omp_get_wtime();
	#endif
    
    /*-------------------------------------------------------------------*/
    /* create the mesh                                                   */
    /*-------------------------------------------------------------------*/

    //set working directory to pszTempPath
    status = chdir(pszTempPath);
    if(status != 0){
        //do something
    }
    
    #ifdef _OPENMP
    startMesh = omp_get_wtime();
	#endif
	
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating mesh...");
    
    if(input.meshType == WindNinjaInputs::MDM){ //use moveDynamicMesh
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Running moveDynamicMesh...");
        status = MoveDynamicMesh();
        if(status != 0){
        //do something
        }
    }
    else{ // use snappyHexMesh
        status = BlockMesh();
        if(status != 0){
            //do something
        }
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Running snappyHexMesh...");
        status = SnappyHexMesh();
        if(status != 0){
            //do something
        }
    }
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Renumbering mesh...");
    status = RenumberMesh();
    if(status != 0){
        //do something
    }
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Checking mesh...");
    status = CheckMesh();
    if(status != 0){
        //do something
    }
    
    /*-------------------------------------------------------------------*/
    /* Apply initial and boundary conditions                             */
    /*-------------------------------------------------------------------*/
    
    #ifdef _OPENMP
    endMesh = omp_get_wtime();
    startInit = omp_get_wtime();
	#endif
	
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Applying initial conditions...");
    status = ApplyInit();
    if(status != 0){
        //do something
    }
    
    
    /*-------------------------------------------------------------------*/
    /* Solve for the flow field                                          */
    /*-------------------------------------------------------------------*/
    
    VSILFILE *fout; 
    
    #ifdef _OPENMP
    endInit = omp_get_wtime();
    startSolve = omp_get_wtime();
	#endif

	
    if(input.numberCPUs > 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel flow calculations...");
        fout = VSIFOpenL("log", "w" );
        status = DecomposePar(fout);
        if(status != 0){
            //do something
        }
    }
    
    checkCancel();
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Solving for the flow field...");
    status = SimpleFoam();
    if(status != 0){
        //do something
    }
    
    if(input.numberCPUs > 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");
        fout = VSIFOpenL("log", "w");
        status = ReconstructPar("-latestTime", fout);
        if(status != 0){
            //do something
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during ReconstructPar(). Check that number of iterations is a multiple of 100.");
            return false;
        }
    }
    
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
        //do something
    }
    
    #ifdef _OPENMP
    endOutputSampling = omp_get_wtime();
	#endif
    
    //move back to ninja working directory
    status = chdir("../");
    if(status != 0){
        //do something
    }
    
    /*----------------------------------------*/
    /*  write output files                    */
    /*----------------------------------------*/
    
    #ifdef _OPENMP
    startWriteOut = omp_get_wtime();
    #endif
	
	input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing output files...");
    
	status = WriteOutputFiles();
    if(status != 1){
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error during output file writing.");
        return false;
    }
    
    #ifdef _OPENMP
    endWriteOut = omp_get_wtime();
    endTotal = omp_get_wtime();
	#endif
	
    /*----------------------------------------*/
    /*  wrap up                              */
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
    
    if(!input.keepOutGridsInMemory)
	{
        AngleGrid.deallocate();
	    VelocityGrid.deallocate();
    }

    return true;
}

int NinjaFoam::AddBcBlock(std::string &dataString)
{
    const char *pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
    const char *pszTemplateFile;
    const char *pszPathToFile;
    const char *pszTemplate;
    
    if(template_ == ""){
        if(gammavalue != ""){
            pszTemplate = CPLStrdup("genericTypeVal.tmp");
        }
        else if(inletoutletvalue != ""){
            pszTemplate = CPLStrdup("genericType.tmp");
        }
        else{
            pszTemplate = CPLStrdup("genericType-kep.tmp");
        }
    }
    else{
        pszTemplate = CPLStrdup(template_.c_str());
    }

    pszPathToFile = CPLSPrintf("ninjafoam.zip/ninjafoam/0/%s", pszTemplate); 
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
    ReplaceKeys(s, "$InputWindHeight$", boost::lexical_cast<std::string>(input.inputWindHeight));
    ReplaceKeys(s, "$z0$", boost::lexical_cast<std::string>( input.surface.Roughness.get_meanValue() ));
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
    
    ReplaceKeys(s, "$terrainName$", "patch0"); //binary stl file
    //ReplaceKeys(s, "$terrainName$", terrainName); //ascii stl file
    
    CPLDebug("NINJAFOAM", "input.nonEqBc = %d", input.nonEqBc);
    
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
    int pos;
    int len; 
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
        std::string t = std::string(CPLGetBasename(input.dem.fileName.c_str()));
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
        ReplaceKeys(s, "$finaltime$",boost::lexical_cast<std::string>(input.nIterations));
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
    int pos;
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
    
    VSIFCloseL(fin); // reopened for each file in writeFoamFiles()
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
    pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
    pszArchive = CPLSPrintf("%s/ninjafoam.zip/ninjafoam", pszPath);
    papszFileList = VSIReadDirRecursive( pszArchive );
    for(int i = 0; i < CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename(papszFileList[i]);
        osFullPath = papszFileList[i];
        if(std::string(pszFilename) == ""){
            pszTempFoamPath = CPLFormFilename(pszTempPath, osFullPath.c_str(), "");
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
           std::string(pszFilename) != "snappyHexMeshDict_cast" &&
           std::string(pszFilename) != "snappyHexMeshDict_layer" &&
           std::string(pszFilename) != "pointDisplacement"){
            pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
            pszArchive = CPLSPrintf("%s/ninjafoam.zip/ninjafoam", pszPath);
            pszInput = CPLFormFilename(pszArchive, osFullPath.c_str(), "");
            pszOutput = CPLFormFilename(pszTempPath, osFullPath.c_str(), "");

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

int NinjaFoam::GenerateTempDirectory()
{
    pszTempPath = CPLStrdup( CPLGenerateTempFilename( "NINJAFOAM_" ) );
    VSIMkdir( pszTempPath, 0777 );
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

int NinjaFoam::readLogFile(int &ratio_)
{
    const char *pszInput;
    
    pszInput = CPLFormFilename(pszTempPath, "log", "json");
    
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
    std:string ss;
    int pos, pos2, pos3, pos4, pos5; 
    int found; 
    pos = s.find("Bounding Box");
    if(pos != s.npos){
        pos2 = s.find("(", pos);
        pos3 = s.find(")", pos2);
        ss = s.substr(pos2+1, pos3-pos2-1); // xmin ymin zmin
        pos4 = s.find("(", pos3);
        pos5 = s.find(")", pos4);
        ss.append(" ");
        ss.append(s.substr(pos4+1, pos5-pos4-1));// xmin ymin zmin xmax ymax zmax
        found = ss.find(" ");
        if(found != ss.npos){
            if(input.meshType == WindNinjaInputs::MDM){
                bbox.push_back(atof(ss.substr(0, found).c_str()) + 100); // xmin
                bbox.push_back(atof(ss.substr(found).c_str()) + 100); // ymin
            }
            else{ // SHM
                bbox.push_back(atof(ss.substr(0, found).c_str())); // xmin
                bbox.push_back(atof(ss.substr(found).c_str())); // ymin
            }
        }
        found = ss.find(" ", found+1);
        if(found != ss.npos){
            if(input.meshType == WindNinjaInputs::MDM){
                bbox.push_back(atof(ss.substr(found).c_str()) + 50); // zmin (should be above highest point in DEM)
            }
            else{ // SHM
                bbox.push_back(atof(ss.substr(found).c_str())); // zmin
            }
        }
        found = ss.find(" ", found+1);
        if(found != ss.npos){
            if(input.meshType == WindNinjaInputs::MDM){
                bbox.push_back(atof(ss.substr(found).c_str()) - 100); // xmax
            }
            else{ // SHM
                bbox.push_back(atof(ss.substr(found).c_str())); // xmax
            }
        }
        found = ss.find(" ", found+1);
        if(found != ss.npos){
            if(input.meshType == WindNinjaInputs::MDM){
                bbox.push_back(atof(ss.substr(found).c_str()) - 100); // ymax
            }
            else{ // SHM
                bbox.push_back(atof(ss.substr(found).c_str())); // ymax
            }
        }
        found = ss.find(" ", found+1);
        if(found != ss.npos){
            if(input.meshType == WindNinjaInputs::MDM){
                bbox.push_back(atof(ss.substr(found).c_str()) + 5000); // zmax
                bbox.push_back(atof(ss.substr(found).c_str()) + 2500); // zmid
            }
            else{ // SHM
                bbox.push_back(atof(ss.substr(found).c_str()) + 3000); // zmax
                bbox.push_back(atof(ss.substr(found).c_str()) + 1000); // zmid
            }
        }
    }
    else{
        cout<<"Bounding Box not found in log.json!"<<endl;
        return NINJA_E_FILE_IO;
    }
    
    double volume1, volume2;
    double cellCount1, cellCount2;
    double cellVolume1, cellVolume2;
    double side2;
    double firstCellHeight2;
    double expansionRatio;
    
    volume1 = (bbox[3] - bbox[0]) * (bbox[4] - bbox[1]) * (bbox[6] - bbox[2]); // volume near terrain
    volume2 = (bbox[3] - bbox[0]) * (bbox[4] - bbox[1]) * (bbox[5] - bbox[6]); // volume away from terrain
    
    cellCount1 = input.meshCount * 0.5; // cell count in volume 1
    cellCount2 = input.meshCount - cellCount1; // cell count in volume 2
    
    cellVolume1 = volume1/cellCount1; // volume of 1 cell in zone1
    cellVolume2 = volume2/cellCount2; // volume of 1 cell in zone2
    
    side1 = std::pow(cellVolume1, (1.0/3.0)); // length of side of regular hex cell in zone1
    side2 = std::pow(cellVolume2, (1.0/3.0)); // length of side of regular hex cell in zone2
    
    nCells.push_back(int( (bbox[3] - bbox[0]) / side1)); // Nx1
    nCells.push_back(int( (bbox[4] - bbox[1]) / side1)); // Ny1
    nCells.push_back(int( (bbox[6] - bbox[2]) / side1)); // Nz1
    
    nCells.push_back(nCells[0]); // Nx2 = Nx1;
    nCells.push_back(nCells[1]); // Ny2 = Ny1;
    
    expansionRatio = 1.13; // expansion ratio in zone2
    
    firstCellHeight2 = ((bbox[6] - bbox[2]) / nCells[2]) * expansionRatio;
    nCells.push_back(int (log(((bbox[5] - bbox[6]) * (expansionRatio - 1) / firstCellHeight2) + 1) / log(expansionRatio) + 1) ); // Nz2
    ratio_ = int(std::pow(expansionRatio, (nCells[5] - 1))); // final2oneRatio
    expansionRatio = std::pow(ratio_, (1.0 / (nCells[5] - 1)));
    
    CPLFree(data);
    VSIFCloseL(fin);
    
    return NINJA_SUCCESS;
}

int NinjaFoam::readDem(int &ratio_)
{
    if(input.meshType == WindNinjaInputs::MDM){
        bbox.push_back( input.dem.get_xllCorner() + 100); //xmin (+/- 100 is a buffer for MDM)
        bbox.push_back( input.dem.get_yllCorner() + 100); //ymin
        bbox.push_back( input.dem.get_maxValue() + 50); //zmin (should be above highest point in DEM for MDM)
        bbox.push_back( input.dem.get_xllCorner() + input.dem.get_xDimension() - 100); //xmax
        bbox.push_back( input.dem.get_yllCorner() + input.dem.get_yDimension() - 100); //ymax
        bbox.push_back( input.dem.get_maxValue() + 5000); //zmax
        bbox.push_back( input.dem.get_maxValue() + 2500); //zmid
    }
    else{ // SHM requires zmin == lowest point in the STL
        bbox.push_back( input.dem.get_xllCorner() ); //xmin 
        bbox.push_back( input.dem.get_yllCorner() ); //ymin
        bbox.push_back( input.dem.get_minValue() ); //zmin 
        bbox.push_back( input.dem.get_xllCorner() + input.dem.get_xDimension() ); //xmax
        bbox.push_back( input.dem.get_yllCorner() + input.dem.get_yDimension() ); //ymax
        bbox.push_back( input.dem.get_maxValue() + 3000); //zmax
        bbox.push_back( input.dem.get_maxValue() + 500); //zmid
    }
    
    double volume1, volume2;
    double cellCount1, cellCount2;
    double cellVolume1, cellVolume2;
    double side2;
    double firstCellHeight2;
    double expansionRatio;
    
    volume1 = (bbox[3] - bbox[0]) * (bbox[4] - bbox[1]) * (bbox[6] - bbox[2]); // volume near terrain
    volume2 = (bbox[3] - bbox[0]) * (bbox[4] - bbox[1]) * (bbox[5] - bbox[6]); // volume away from terrain
    
    cellCount1 = input.meshCount * 0.5; // cell count in volume 1
    cellCount2 = input.meshCount - cellCount1; // cell count in volume 2
    
    cellVolume1 = volume1/cellCount1; // volume of 1 cell in zone1
    cellVolume2 = volume2/cellCount2; // volume of 1 cell in zone2
    
    side1 = std::pow(cellVolume1, (1.0/3.0)); // length of side of regular hex cell in zone1
    side2 = std::pow(cellVolume2, (1.0/3.0)); // length of side of regular hex cell in zone2
    
    nCells.push_back(int( (bbox[3] - bbox[0]) / side1)); // Nx1
    nCells.push_back(int( (bbox[4] - bbox[1]) / side1)); // Ny1
    nCells.push_back(int( (bbox[6] - bbox[2]) / side1)); // Nz1
    
    nCells.push_back(nCells[0]); // Nx2 = Nx1;
    nCells.push_back(nCells[1]); // Ny2 = Ny1;
    
    expansionRatio = 1.13; // expansion ratio in zone2
    
    firstCellHeight2 = ((bbox[6] - bbox[2]) / nCells[2]) * expansionRatio;
    nCells.push_back(int (log(((bbox[5] - bbox[6]) * (expansionRatio - 1) / firstCellHeight2) + 1) / log(expansionRatio) + 1) ); // Nz2
    ratio_ = int(std::pow(expansionRatio, (nCells[5] - 1))); // final2oneRatio
    expansionRatio = std::pow(ratio_, (1.0 / (nCells[5] - 1)));
    
    return NINJA_SUCCESS;
}

int NinjaFoam::writeBlockMesh()
{
    const char *pszInput;
    const char *pszOutput;
    const char *pszPath;
    const char *pszArchive;
    int ratio_;
    int status;
    
    if(input.stlFile != "!set"){ //if an STL file was supplied and we don't have a DEM
        status = readLogFile(ratio_);
        if(status != 0){
            //do something
        }
    }
    else{
        status = readDem(ratio_);
        if(status != 0){
            //do something
        }
    }
    
    pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
    pszArchive = CPLSPrintf("%s/ninjafoam.zip", pszPath);
    
    pszInput = CPLFormFilename(pszArchive, "ninjafoam/constant/polyMesh/blockMeshDict", "");
    pszOutput = CPLFormFilename(pszTempPath, "constant/polyMesh/blockMeshDict", "");
    
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
    bboxField.push_back("$zmid$");
    
    cellField.push_back("$Nx1$");
    cellField.push_back("$Ny1$");
    cellField.push_back("$Nz1$");
    cellField.push_back("$Nx2$");
    cellField.push_back("$Ny2$");
    cellField.push_back("$Nz2$");
    
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
    
    pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
    pszArchive = CPLSPrintf("%s/ninjafoam.zip", pszPath);
    
    pszInput = CPLFormFilename(pszArchive, "ninjafoam/0/pointDisplacement", "");
    pszOutput = CPLFormFilename(pszTempPath, "0/pointDisplacement", "");
    
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
    
    std::string t = std::string(CPLGetBasename(input.dem.fileName.c_str()));
    ReplaceKeys(s, "$stlName$", t);
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
    
    CPLFree(data);
    VSIFCloseL(fin); 
    VSIFCloseL(fout);

}

int NinjaFoam::writeSnappyMesh()
{
    int lx, ly, lz;
    double expansionRatio;
    double final_;
    double first;
    int nLayers;
    
    lx = int((bbox[0] + bbox[3]) * 0.5);
    ly = int((bbox[1] + bbox[4]) * 0.5);
    lz = int((bbox[6]));
    expansionRatio = 1.4;
    final_ = side1/expansionRatio;
    first = 4.0;
    nLayers = int((log(final_/first) / log(expansionRatio)) + 1 + 1);
    
    const char *pszInput;
    const char *pszOutput;
    const char *pszPath;
    const char *pszArchive;
    
    pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
    pszArchive = CPLSPrintf("%s/ninjafoam.zip", pszPath);
    
    //-----------------------------
    //  write snappyHexMeshDict
    //-----------------------------
    
    pszInput = CPLFormFilename(pszArchive, "ninjafoam/system/snappyHexMeshDict_cast", "");
    pszOutput = CPLFormFilename(pszTempPath, "system/snappyHexMeshDict", "");
    
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
    
    ReplaceKeys(s, "$stlName$", std::string(CPLGetBasename(input.dem.fileName.c_str())));
    ReplaceKeys(s, "$stlRegionName$", "patch0"); //binary stl files
    //ReplaceKeys(s, "$stlRegionName$", "NAME"); //ascii stl files
    ReplaceKeys(s, "$lx$", boost::lexical_cast<std::string>(lx));
    ReplaceKeys(s, "$ly$", boost::lexical_cast<std::string>(ly));
    ReplaceKeys(s, "$lz$", boost::lexical_cast<std::string>(lz));
    ReplaceKeys(s, "$Nolayers$", boost::lexical_cast<std::string>(nLayers));
    ReplaceKeys(s, "$expansion_ratio$", CPLSPrintf("%.1lf", expansionRatio));
    ReplaceKeys(s, "$final$", boost::lexical_cast<std::string>(final_));
    
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
        
    CPLFree(data);
    VSIFCloseL(fin); 
    VSIFCloseL(fout); 
    
    //-----------------------------
    //  write snappyHexMeshDict1
    //-----------------------------
    
    pszInput = CPLFormFilename(pszArchive, "ninjafoam/system/snappyHexMeshDict_layer", "");
    pszOutput = CPLFormFilename(pszTempPath, "system/snappyHexMeshDict1", "");
    
    fin = VSIFOpenL( pszInput, "r" );
    fout = VSIFOpenL( pszOutput, "w" );
    
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    data = (char*)CPLMalloc(offset * sizeof(char) + 1);
    VSIFReadL(data, offset, 1, fin);
    data[offset] = '\0';
    
    s = data;
    
    ReplaceKeys(s, "$stlName$", std::string(CPLGetBasename(input.dem.fileName.c_str())));
    ReplaceKeys(s, "$stlRegionName$", "patch0"); //binary stl files
    //ReplaceKeys(s, "$stlRegionName$", "NAME"); //ascii stl files
    ReplaceKeys(s, "$lx$", boost::lexical_cast<std::string>(lx));
    ReplaceKeys(s, "$ly$", boost::lexical_cast<std::string>(ly));
    ReplaceKeys(s, "$lz$", boost::lexical_cast<std::string>(lz));
    ReplaceKeys(s, "$Nolayers$", boost::lexical_cast<std::string>(nLayers));
    ReplaceKeys(s, "$expansion_ratio$", boost::lexical_cast<std::string>(expansionRatio));
    ReplaceKeys(s, "$final$", boost::lexical_cast<std::string>(final_));
    
    d = s.c_str();
    nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
        
    CPLFree(data);
    VSIFCloseL(fin); 
    VSIFCloseL(fout); 
    
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

int NinjaFoam::CopyFile(const char *pszInput, const char *pszOutput)
{
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
    
    const char * d = s.c_str();
    int nSize = strlen(d);
    VSIFWriteL(d, nSize, 1, fout);
        
    CPLFree(data);
    VSIFCloseL(fin); 
    VSIFCloseL(fout);
    
    return NINJA_SUCCESS;
}

int NinjaFoam::SurfaceTransformPoints()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "surfaceTransformPoints", 
                                      "-translate", 
                                      CPLSPrintf("(0 0 %.0f)", input.outputWindHeight), 
                                      CPLSPrintf("constant/triSurface/%s.stl", CPLGetBasename(input.dem.fileName.c_str())), 
                                      CPLSPrintf("constant/triSurface/%s_out.stl", CPLGetBasename(input.dem.fileName.c_str())),
                                      NULL };
    
    VSILFILE *fout = VSIFOpenL("surfaceTransformPoints.log", "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE); //create output surface stl in pszTemppath/constant/triSurface

    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::SurfaceCheck()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "surfaceCheck", 
                                      CPLSPrintf("constant/triSurface/%s.stl", CPLGetBasename(input.dem.fileName.c_str())), 
                                      NULL };
                                      
    VSILFILE *fout = VSIFOpenL("log.json", "w");
    
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
        //do something
    }
    
    VSILFILE *fout;
    
    const char *pszInput;
    const char *pszOutput;

    if(input.numberCPUs > 1){
    
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel mesh calculations...");
        fout = VSIFOpenL("logMesh", "w");
        nRet = DecomposePar(fout);
        if(nRet != 0){
            //do something
        }
        
        //re-write controlDict for moveDynamicMesh
        pszInput = CPLFormFilename("system", "controlDict_moveDynamicMesh", "");
        pszOutput = CPLFormFilename("system", "controlDict", "");    
        CopyFile(pszInput, pszOutput); 

#ifdef WIN32
        const char *const papszArgv[] = { "mpiexec", 
                                      "-env",
                                      "MPI_BUFFER_SIZE",
                                      "20000000",
                                      "-n",
                                      CPLSPrintf("%d", input.numberCPUs),
                                      "moveDynamicMesh",
                                      "-parallel",
                                       NULL };
#else
        CPLSetConfigOption("MPI_BUFFER_SIZE", "20000000");
        const char *const papszArgv[] = { "mpiexec", 
                                      "-np",
                                      CPLSPrintf("%d", input.numberCPUs), 
                                      "moveDynamicMesh",
                                      "-parallel",
                                       NULL };
#endif
                                       
        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
        
        char data[PIPE_BUFFER_SIZE + 1];
        int pos;
        std::string s;
        
        /* Track progress */
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){	
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
            
            std::string ss;
            int nchar;
            
            if(s.rfind("Time = ") != s.npos){
                pos = s.rfind("Time = ");
                    nchar = s.find('\n', pos) - (pos+7);
                    ss = s.substr( (pos+7), nchar );
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(moveDynamicMesh) %.0f%% complete...", atof(ss.c_str())*2);
            }
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
        if(nRet != 0){
            //do something
        }
 
        //re-write controlDict for flow
        pszInput = CPLFormFilename("system", "controlDict_simpleFoam", "");
        pszOutput = CPLFormFilename("system", "controlDict", "");    
        CopyFile(pszInput, pszOutput); 
    
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");
        fout = VSIFOpenL("logMesh", "w");
        nRet = ReconstructPar("-latestTime", fout);
        
        //VSIFCloseL(fout);
        
        if(nRet != 0){
            //do something
        }
    } 
    
    else{ // single processor
        //re-write controlDict for moveDynamicMesh
        pszInput = CPLFormFilename("system", "controlDict_moveDynamicMesh", "");
        pszOutput = CPLFormFilename("system", "controlDict", "");    
        CopyFile(pszInput, pszOutput); 
    
        const char *const papszArgv[] = { "moveDynamicMesh", NULL };
    
        VSILFILE *fout = VSIFOpenL("log.moveDynamicMesh", "w");
    
        nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
        //VSIFCloseL(fout);
        
        //re-write controlDict for flow solution
        pszInput = CPLFormFilename("system", "controlDict_simpleFoam", "");
        pszOutput = CPLFormFilename("system", "controlDict", "");
        CopyFile(pszInput, pszOutput);
    }
    
    //copy 0/ to 100/ (or latest time)
    pszInput = CPLFormFilename("0", "U", "");
    pszOutput = CPLFormFilename("50", "U", "");
    CopyFile(pszInput, pszOutput);
    
    pszInput = CPLFormFilename("0", "p", "");
    pszOutput = CPLFormFilename("50", "p", "");
    CopyFile(pszInput, pszOutput);
    
    pszInput = CPLFormFilename("0", "k", "");
    pszOutput = CPLFormFilename("50", "k", "");
    CopyFile(pszInput, pszOutput);
    
    pszInput = CPLFormFilename("0", "epsilon", "");
    pszOutput = CPLFormFilename("50", "epsilon", "");
    CopyFile(pszInput, pszOutput);
    
    //VSIFCloseL(fout);

    return nRet;
}

int NinjaFoam::BlockMesh()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "blockMesh", NULL };
    
    VSILFILE *fout = VSIFOpenL("logMesh", "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::DecomposePar(VSILFILE *fout)
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "decomposePar", "-force", NULL };
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::SnappyHexMesh()
{    
    int nRet = -1;
    
    if(input.numberCPUs > 1){
    
        VSILFILE *fout; 
       
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Decomposing domain for parallel mesh calculations...");
        fout = VSIFOpenL("logMesh", "w");
        nRet = DecomposePar(fout);
        if(nRet != 0){
            //do something
        }
        VSIFCloseL(fout);
    
        #ifdef WIN32
        const char *const papszArgv[] = { "mpiexec", 
                                      "-env",
                                      "MPI_BUFFER_SIZE",
                                      "20000000",
                                      "-n",
                                      CPLSPrintf("%d", input.numberCPUs),
                                      "snappyHexMesh",
                                      "-overwrite",
                                      "-parallel",
                                       NULL };
        #else
        CPLSetConfigOption("MPI_BUFFER_SIZE", "20000000");
        const char *const papszArgv[] = { "mpiexec", 
                                      "-np",
                                      CPLSPrintf("%d", input.numberCPUs), 
                                      "snappyHexMesh",
                                      "-overwrite",
                                      "-parallel",
                                       NULL };
        #endif
        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
        
        char data[PIPE_BUFFER_SIZE + 1];
        int pos;
        std::string s;
        
        /* find a better way to determine % complete. Are the number of morph iterations fixed? */
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            data[sizeof(data)-1] = '\0';
            s.append(data);
            if(s.find("Morph iteration 8") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 60%% complete...");
                break;
            }
            else if(s.find("Morph iteration 6") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 50%% complete...");
            }
            else if(s.find("Morph iteration 4") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 40%% complete...");
            }
            else if(s.find("Morph iteration 2") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 30%% complete...");
            }
            else if(s.find("Morph iteration 0") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 20%% complete...");
            }
            else if(s.find("Checking initial mesh") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 10%% complete...");
            }
            else if(s.find("Determining initial surface intersections") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 5%% complete...");
            }       
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
    }
    else{
        const char *const papszArgv[] = { "snappyHexMesh",
                                      "-overwrite",
                                       NULL };
                                       
        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
        
        char data[PIPE_BUFFER_SIZE + 1];
        int pos;
        std::string s;
        
        /* find a better way to determine % complete. Are the number of morph iterations fixed? */
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            data[sizeof(data)-1] = '\0';
            s.append(data);
            if(s.find("Morph iteration 8") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 60%% complete...");
                break;
            }
            else if(s.find("Morph iteration 6") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 50%% complete...");
            }
            else if(s.find("Morph iteration 4") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 40%% complete...");
            }
            else if(s.find("Morph iteration 2") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 30%% complete...");
            }
            else if(s.find("Morph iteration 0") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 20%% complete...");
            }
            else if(s.find("Checking initial mesh") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 10%% complete...");
            }
            else if(s.find("Determining initial surface intersections") != s.npos){
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 5%% complete...");
            }         
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
    }
    
    if(nRet != 0){
        return nRet;
    }
    else{
        VSIUnlink(CPLFormFilename("system", "snappyHexMeshDict", ""));
      
        VSILFILE *fin = VSIFOpenL(CPLFormFilename("system", "snappyHexMeshDict1", ""), "r" );
        VSILFILE *fout = VSIFOpenL(CPLFormFilename("system", "snappyHexMeshDict", ""), "w" );
    
        char *data;
    
        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);

        VSIRewindL(fin);
        data = (char*)CPLMalloc(offset * sizeof(char));
        VSIFReadL(data, offset, 1, fin);
        VSIFWriteL(data, offset, 1, fout);
        
        CPLFree(data);
        VSIFCloseL(fin);
        VSIFCloseL(fout);
        VSIUnlink(CPLFormFilename("system", "snappyHexMeshDict1", ""));
        
        if(input.numberCPUs > 1){
            #ifdef WIN32
            const char *const papszArgv2[] = { "mpiexec", 
                                     "-env",
                                      "MPI_BUFFER_SIZE",
                                      "20000000",
                                      "-n",
                                      CPLSPrintf("%d", input.numberCPUs), 
                                      "snappyHexMesh",
                                      "-overwrite",
                                      "-parallel",
                                       NULL };
            #else
            CPLSetConfigOption("MPI_BUFFER_SIZE", "20000000");
            const char *const papszArgv2[] = { "mpiexec", 
                                      "-np",
                                      CPLSPrintf("%d", input.numberCPUs), 
                                      "snappyHexMesh",
                                      "-overwrite",
                                      "-parallel",
                                       NULL };
            #endif
        
            CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv2, FALSE, TRUE, TRUE, NULL);
            CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
            
            char data[PIPE_BUFFER_SIZE + 1];
            int pos;
            std::string s;
        
            while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
                data[sizeof(data)-1] = '\0';
                s.append(data);
                if(s.find("Checking initial mesh") != s.npos){
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 80%% complete...");
                }
                else if(s.find("Determining initial surface intersections") != s.npos){
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 70%% complete...");
                }
            }
            nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
            
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reconstructing domain...");
            fout = VSIFOpenL("logMesh", "w");
            nRet = ReconstructParMesh("-constant", fout);
            if(nRet != 0){
                //do something
            }
            VSIFCloseL(fout);
        }
        else{
            const char *const papszArgv2[] = { "snappyHexMesh",
                                           "-overwrite",
                                            NULL };
        
            CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv2, FALSE, TRUE, TRUE, NULL);
            CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
            
            char data[PIPE_BUFFER_SIZE];
            int pos;
            std::string s;
            
            while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
                data[sizeof(data)-1] = '\0';
                s.append(data);
                if(s.find("Checking initial mesh") != s.npos){
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 80%% complete...");
                }
                else if(s.find("Determining initial surface intersections") != s.npos){
                    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 70%% complete...");
                }
            }
            nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
        }
    }
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "(snappyHexMesh) 100%% complete...");
    
    return nRet;
}

int NinjaFoam::ReconstructParMesh(const char *const arg, VSILFILE *fout)
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "reconstructParMesh", arg, NULL };
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::ReconstructPar(const char *const arg, VSILFILE *fout)
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "reconstructPar", arg, NULL };
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::RenumberMesh()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "renumberMesh", "-overwrite", NULL };
    
    VSILFILE *fout = VSIFOpenL("log", "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::CheckMesh()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "checkMesh", NULL };
    
    VSILFILE *fout = VSIFOpenL("log.checkmesh", "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::ApplyInit()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "applyInit", NULL };
    
    VSILFILE *fout = VSIFOpenL("log", "w");
    
    nRet = CPLSpawn(papszArgv, NULL, fout, TRUE);
    
    VSIFCloseL(fout);
    
    return nRet;
}

int NinjaFoam::SimpleFoam()
{
    int nRet = -1;
    
    char data[PIPE_BUFFER_SIZE + 1];
    int pos;
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
                                      "-parallel",
                                       NULL };
        #else
        CPLSetConfigOption("MPI_BUFFER_SIZE", "20000000");
        const char *const papszArgv[] = { "mpiexec", 
                                      "-np",
                                      CPLSPrintf("%d", input.numberCPUs), 
                                      "simpleFoam",
                                      "-parallel",
                                       NULL };
        #endif
        
        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
        
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            data[sizeof(data)-1] = '\0';
            s.append(data);
            CPLDebug("NINJAFOAM", "simpleFoam: %s", data);
            pos = s.rfind("Time = ");
            if(pos != s.npos && s.npos > (pos + 12) && s.rfind("\n", pos) == (pos-1)){ 
                t = s.substr(pos+7, (s.find("\n", pos+7) - (pos+7)));
                p = atof(t.c_str()) / input.nIterations * 100;
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(solver) %.0f%% complete...", p);
            }
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
    }
    else{
        const char *const papszArgv[] = { "simpleFoam",
                                       NULL };
                                       
        CPLSpawnedProcess *sp = CPLSpawnAsync(NULL, papszArgv, FALSE, TRUE, TRUE, NULL);
        CPL_FILE_HANDLE out_child = CPLSpawnAsyncGetInputFileHandle(sp);
        
        while(CPLPipeRead(out_child, &data, sizeof(data)-1)){
            data[sizeof(data)-1] = '\0';
            s.append(data);
            pos = s.rfind("Time = ");
            if(pos != s.npos && s.npos > (pos + 12) && s.rfind("\n", pos) == (pos-1)){ 
                t = s.substr(pos+7, (s.find("\n", pos+7) - (pos+7)));
                p = atof(t.c_str()) / input.nIterations * 100;
                input.Com->ninjaCom(ninjaComClass::ninjaNone, "(solver) %.0f%% complete...", p);
            }
        }
        nRet = CPLSpawnAsyncFinish(sp, TRUE, FALSE);
    }

    return nRet;
}

int NinjaFoam::Sample()
{
    int nRet = -1;
    
    const char *const papszArgv[] = { "sample", "-latestTime", NULL };
    
    VSILFILE *fout = VSIFOpenL("log", "w");
    
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
    const char *pszRaw;
    const char *pszMem;
    std::string s;

    pszMem = CPLSPrintf( "%s/output.raw", pszTempPath );
    /* This is a member, hold on to it so we can read it later */
    pszVrtMem = CPLStrdup( CPLSPrintf( "%s/output.vrt", pszTempPath ) );
    
    char **papszOutputSurfacePath;
    papszOutputSurfacePath = VSIReadDir( CPLStrdup(CPLSPrintf("%s/postProcessing/surfaces/", pszTempPath)) );
    
    for(int i = 0; i < CSLCount( papszOutputSurfacePath ); i++){
        if(std::string(papszOutputSurfacePath[i]) != "." && 
           std::string(papszOutputSurfacePath[i]) != "..") {
            pszRaw = CPLStrdup( CPLSPrintf( "%s/postProcessing/surfaces/%s/U_triSurfaceSampling.raw", pszTempPath, papszOutputSurfacePath[i]) );
            break;
        }
        else{
            continue;
        }
    }
    
    fin = VSIFOpen( pszRaw, "r" );
    fout = VSIFOpenL( pszMem, "w" );
    fvrt = VSIFOpenL( pszVrtMem, "w" );
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
    pszGridFilename = CPLStrdup( CPLSPrintf( "%s/foam.tif", pszTempPath ) );
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
    pszGridFilename = CPLStrdup( CPLSPrintf( "%s/foam.tif", pszTempPath ) );
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

void NinjaFoam::SetOutputFilenames()
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

    //Do file naming string stuff for all output files
    std::string rootFile, rootName, fileAppend, kmz_fileAppend, \
        shp_fileAppend, ascii_fileAppend, mesh_units, kmz_mesh_units, \
        shp_mesh_units, ascii_mesh_units, pdf_fileAppend, pdf_mesh_units;

    std::string pathName;
    std::string baseName(CPLGetBasename(input.dem.fileName.c_str()));
    
    pathName = CPLGetPath(input.dem.fileName.c_str());
    rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);

    /* set the output path member variable */
    input.outputPath = pathName;

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
    lengthUnits::fromBaseUnits(kmzResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(shpResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(velResolutionTemp, meshResolutionUnits);
    lengthUnits::fromBaseUnits(pdfResolutionTemp, meshResolutionUnits);

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

    input.velFile = rootFile + ascii_fileAppend + "_vel.asc";
    input.angFile = rootFile + ascii_fileAppend + "_ang.asc";
    input.atmFile = rootFile + ascii_fileAppend + ".atm";

    input.legFile = rootFile + kmz_fileAppend + ".bmp";
    if( input.ninjaTime.is_not_a_date_time() )	//date and time not set?
        input.dateTimeLegFile = "";
    else
        input.dateTimeLegFile = rootFile + kmz_fileAppend + ".date_time" + ".bmp";
}


int NinjaFoam::WriteOutputFiles()
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
        //do something
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
    
    /*-------------------------------------------------------------------*/
    /* set up filenames                                                  */
    /*-------------------------------------------------------------------*/
    
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

			angTempGrid = new AsciiGrid<double> (AngleGrid.resample_Grid(input.angResolution, AsciiGrid<double>::order0));
			velTempGrid = new AsciiGrid<double> (VelocityGrid.resample_Grid(input.velResolution, AsciiGrid<double>::order0));

			angTempGrid->write_Grid(input.angFile.c_str(), 0);
			velTempGrid->write_Grid(input.velFile.c_str(), 2);

            //angTempGrid->write_Grid("angle.asc", 0);
			//velTempGrid->write_Grid("vel.asc", 2);

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
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during shape file writing: %s", e.what());
	}catch (...)
	{
		input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during shape file writing: Cannot determine exception type.");
	}
	/* keep pszTempPath and OpenFOAM files if vtk output is requested */
	if(input.volVTKOutFlag==false)
    {
        NinjaUnlinkTree( pszTempPath );
    }
	
	return true;
}

int NinjaFoam::ReadStl()
{
    //set working directory to pszTempPath
    int status = chdir(pszTempPath);
    if(status != 0){
        //do something
    }
    
    VSILFILE *ffin;
    VSILFILE *ffout;
    
    ffin = VSIFOpenL( input.stlFile.c_str(), "r" );
    ffout = VSIFOpenL( ("constant/triSurface/%s", CPLGetFilename(input.stlFile.c_str()) ), "w" );

    char *data;
    
    vsi_l_offset offset;
    VSIFSeekL(ffin, 0, SEEK_END);
    offset = VSIFTellL(ffin);

    VSIRewindL(ffin);
    data = (char*)CPLMalloc(offset * sizeof(char));
    VSIFReadL(data, offset, 1, ffin);
        
    //cout<<"data = "<<data<<endl;
    
    VSIFWriteL(data, offset, 1, ffout);
        
    CPLFree(data);
    VSIFCloseL(ffin); 
    VSIFCloseL(ffout); 
    
    status = chdir("../");
    if(status != 0){
        //do something
    }
    
    //move back to ninja working directory
    status =  chdir("../");
    if(status != 0){
        //do something
    }
    
    return true;
}
