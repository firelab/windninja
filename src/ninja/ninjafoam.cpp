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
    pszTerrainFile = NULL;
    pszTempPath = NULL;
    pszOgrBase = NULL;
    hGriddedDS = NULL;
    
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
    free( (void*)pszTerrainFile );
    free( (void*)pszTempPath );
    free( (void*)pszOgrBase );
    GDALClose( hGriddedDS );
}

bool NinjaFoam::simulate_wind()
{

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reading elevation file...");

    readInputFile();
    set_position();
    set_uniVegetation();

    checkInputs();
    
    ComputeDirection(); //convert wind direction to unit vector notation
    SetInlets();
    SetBcs();

    #ifdef _OPENMP
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);
    #endif

    /*------------------------------------------*/
    /*  write OpenFOAM files                    */
    /*------------------------------------------*/

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing OpenFOAM files...");

    int status;

    status = GenerateTempDirectory();
    if(status != 0){
        //do something
    }

    status = WriteFoamFiles();
    if(status != 0){
        //do something
    }

    /*-------------------------------------------------------------------*/
    /*  convert DEM to STL format and write to constant/triSurface       */
    /*-------------------------------------------------------------------*/

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Converting DEM to STL format...");

    const char *pszShortName = CPLGetBasename(input.dem.fileName.c_str());
    const char *pszStlPath = CPLSPrintf("%s/constant/triSurface/", pszTempPath);
    const char *pszStlFileName = CPLFormFilename(pszStlPath, pszShortName, ".stl");

    int nBand = 1;
    const char * inFile = input.dem.fileName.c_str();
    const char * outFile = pszStlFileName;

    CPLErr eErr;

    eErr = NinjaElevationToStl(inFile,
                        outFile,
                        nBand,
                        NinjaStlBinary,
                        NULL);

    if(eErr != 0){
        //do something
    }
    
    //system call: surfaceTransformPoints, surfaceCheck

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
    data = (char*)CPLMalloc(offset * sizeof(char));
    VSIFReadL(data, offset, 1, fin); //read in the template file
    
    //cout<<"data in new block = \n"<<data<<endl;
    
    std::string s(data);
    int pos; 
    int len; 
    pos = s.find("$boundary_name$");
    len = std::string("$boundary_name$").length();
    if(pos != s.npos){
        s.replace(pos, len, boundary_name);
    }
    
    pos = s.find("$type$");
    len = std::string("$type$").length();
    if(pos != s.npos){
        s.replace(pos, len, type);
    }
    
    pos = s.find("$value$");
    len = std::string("$value$").length();
    if(pos != s.npos){
        s.replace(pos, len, value);
    }
    
    pos = s.find("$gammavalue$");
    len = std::string("$gammavalue$").length();
    if(pos != s.npos){
        s.replace(pos, len, gammavalue);
    }
    
    pos = s.find("$pvalue$");
    len = std::string("$pvalue$").length();
    if(pos != s.npos){
        s.replace(pos, len, pvalue);
    }
    
    pos = s.find("$U_freestream$");
    len = std::string("$U_freestream$").length();
    if(pos != s.npos){
        std::string t = boost::lexical_cast<std::string>(input.inputSpeed);
        s.replace(pos, len, t);
    }
    
    pos = s.find("$direction$");
    len = std::string("$direction$").length();
    if(pos != s.npos){
        std::string t, t1, t2, t3;
        t1 = boost::lexical_cast<std::string>(direction[0]);
        t2 = boost::lexical_cast<std::string>(direction[1]);
        t3 = boost::lexical_cast<std::string>(direction[2]);
        t = "(" + t1 + " " + t2 + " " + t3 + ")";
        s.replace(pos, len, t);
    }
    
    pos = s.find("$InputWindHeight$");
    len = std::string("$InputWindHeight$").length();
    if(pos != s.npos){
        std::string t = boost::lexical_cast<std::string>(input.inputWindHeight);
        s.replace(pos, len, t);
    }
    
    pos = s.find("$z0$");
    len = std::string("$z0$").length();
    if(pos != s.npos){
        std::string t = boost::lexical_cast<std::string>(input.surface.Roughness(0,0));
        s.replace(pos, len, t);
    }
    
    pos = s.find("$Rd$");
    len = std::string("$Rd$").length();
    if(pos != s.npos){
        std::string t = boost::lexical_cast<std::string>(input.surface.Rough_d(0,0));
        s.replace(pos, len, t);
    }
    
    pos = s.find("$inletoutletvalue$");
    len = std::string("$inletoutletvalue$").length();
    if(pos != s.npos){
        s.replace(pos, len, inletoutletvalue);
    }
    
    pos = s.find("$inletoutletvalue$");
    len = std::string("$inletoutletvalue$").length();
    if(pos != s.npos){
        s.replace(pos, len, inletoutletvalue);
    }
    
    dataString.append(s);
    
    CPLFree(data);
    VSIFCloseL(fin);
    
    return NINJA_SUCCESS;
    
}

int NinjaFoam::WriteZeroFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename)
{
    
    /* 
     * write the p file
     */
         
    if(std::string(pszFilename) == "p"){
        WritePFile(fin, fout);
    }
    
    
    /* 
     * write the U file
     */
     
    if(std::string(pszFilename) == "U"){
        WriteUFile(fin, fout);
    }

     
     /* 
      * write the k file
      */
      
    if(std::string(pszFilename) == "k"){
        WriteKFile(fin, fout);
    }
      
      
    /* 
     * write the epsilon file
     */
     
    if(std::string(pszFilename) == "epsilon"){
        WriteEpsilonFile(fin, fout);
    }
      
    /* 
     * write the nut file
     */
     
    if(std::string(pszFilename) == "nut"){
        WriteNutFile(fin, fout);
    }
    
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
        if(std::string(pszFilename) != "" && std::string(CPLGetExtension(pszFilename)) != "tmp"){
            pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
            pszArchive = CPLSPrintf("%s/ninjafoam.zip/ninjafoam", pszPath);
            pszInput = CPLFormFilename(pszArchive, osFullPath.c_str(), "");
            pszOutput = CPLFormFilename(pszTempPath, osFullPath.c_str(), "");

            fin = VSIFOpenL( pszInput, "r" );
            fout = VSIFOpenL( pszOutput, "w" );
            
            if( osFullPath.find("0") == 0){ //zero files
                WriteZeroFiles(fin, fout, pszFilename);
            }
            else if( osFullPath.find("system") == 0 ){ //system files
                //WriteSystemFiles(fin, fout);
            }
            else if( osFullPath.find("constant") == 0 ){ //constant files
                //WriteConstantFiles(fin, fout);
            }
        }
    }

    CSLDestroy( papszFileList );

    return NINJA_SUCCESS;
}

int NinjaFoam::GenerateTempDirectory()
{
    pszTempPath = CPLStrdup( CPLGenerateTempFilename( NULL ) );
    VSIMkdir( pszTempPath, 0777 );
    pszOgrBase = NULL;

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

int NinjaFoam::WriteOgrVrt( const char *pszSrsWkt )
{
    VSILFILE *fout;
    CPLAssert( pszTempPath );
    CPLAssert( pszOgrBase );
    CPLAssert( pszSrsWkt );
    vsi_l_offset nOffset;
    fout = VSIFOpenL( CPLFormFilename( pszTempPath, pszOgrBase, ".vrt" ),
                      "wb" );
    if( !fout )
    {
        return NINJA_E_FILE_IO;
    }
    const char *pszVrt;
    pszVrt = CPLSPrintf( NINJA_FOAM_OGR_VRT, pszOgrBase, pszOgrBase,
                         pszOgrBase, pszSrsWkt );

    nOffset = VSIFWriteL( pszVrt, CPLStrnlen( pszVrt, 8192 ), 1, fout );
    CPLAssert( nOffset );
    VSIFCloseL( fout );
    return NINJA_SUCCESS;
}

int NinjaFoam::RunGridSampling()
{
    return NINJA_SUCCESS;
}

GDALDatasetH NinjaFoam::GetRasterOutputHandle()
{
    return hGriddedDS;
}

int NinjaFoam::WriteNutFile(VSILFILE *fin, VSILFILE *fout)
{
        int pos;
        char *data;
        
        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);

        VSIRewindL(fin);
        data = (char*)CPLMalloc(offset * sizeof(char));
        VSIFReadL(data, offset, 1, fin); //read in full p template file
        
        // write to first dictionary value
        std::string dataString;
        std::string s(data);
        pos = s.find("$boundaryField$");
        if(pos != s.npos){
            s.erase(pos);
            dataString.append(s);
        }
        
        s = data;
        pos = s.find("$boundaryField$");
        int len = std::string("$boundaryField$").length();
        if(pos != s.npos){
            s.erase(0, pos+len);
        }
        
        pos = s.find("$terrainName$");
        len = std::string("$terrainName$").length();
        if(pos != s.npos){
            s.replace(pos, len, terrainName);
        }
        
        dataString.append(s);
        
        //cout<<"dataString = \n"<<dataString<<endl;
        
        const char * d = dataString.c_str();
        int nSize = strlen(d);
        
        VSIFWriteL( d, nSize, 1, fout );

        CPLFree(data);
        
        VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
        VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()
        
        return NINJA_SUCCESS;
}


int NinjaFoam::WriteEpsilonFile(VSILFILE *fin, VSILFILE *fout)
{
        int pos;
        char *data;
        
        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);

        VSIRewindL(fin);
        data = (char*)CPLMalloc(offset * sizeof(char));
        VSIFReadL(data, offset, 1, fin); //read in full p template file
        
        // write to first dictionary value
        std::string dataString;
        std::string s(data);
        pos = s.find("$boundaryField$");
        if(pos != s.npos){
            s.erase(pos);
            dataString.append(s);
        }
        
        //append BC blocks from template files
        for(int i = 0; i < bcs.size(); i++){
            boundary_name = bcs[i];
            //check if boundary_name is an inlet
            if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
                template_ = "inlet.tmp";
                type = "logProfileDissipationRateInlet";
            }
            else{
                template_ = "";
                type = "zeroGradient";
            }
            int status;
            //append BC block for current face
            status = AddBcBlock(dataString);
            if(status != 0){
                //do something
            }
        }
        
        s = data;
        pos = s.find("$boundaryField$");
        int len = std::string("$boundaryField$").length();
        if(pos != s.npos){
            s.erase(0, pos+len);
        }
        
        pos = s.find("$terrainName$");
        len = std::string("$terrainName$").length();
        if(pos != s.npos){
            s.replace(pos, len, terrainName);
        }
        
        dataString.append(s);
        
        //cout<<"dataString = \n"<<dataString<<endl;
        
        const char * d = dataString.c_str();
        int nSize = strlen(d);
        
        VSIFWriteL( d, nSize, 1, fout );

        CPLFree(data);
        
        VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
        VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()
        
        return NINJA_SUCCESS;
}

int NinjaFoam::WriteKFile(VSILFILE *fin, VSILFILE *fout)
{
        int pos;
        char *data;
        
        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);

        VSIRewindL(fin);
        data = (char*)CPLMalloc(offset * sizeof(char));
        VSIFReadL(data, offset, 1, fin); //read in full p template file
        
        // write to first dictionary value
        std::string dataString;
        std::string s(data);
        pos = s.find("$boundaryField$");
        if(pos != s.npos){
            s.erase(pos);
            dataString.append(s);
        }
        
        //append BC blocks from template files
        for(int i = 0; i < bcs.size(); i++){
            boundary_name = bcs[i];
            //check if boundary_name is an inlet
            if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
                template_ = "inlet.tmp";
                type = "logProfileTurbulentKineticEnergyInlet";
            }
            else{
                template_ = "";
                type = "zeroGradient";
            }
            int status;
            //append BC block for current face
            status = AddBcBlock(dataString);
            if(status != 0){
                //do something
            }
        }
        
        s = data;
        pos = s.find("$boundaryField$");
        int len = std::string("$boundaryField$").length();
        if(pos != s.npos){
            s.erase(0, pos+len);
        }
        
        pos = s.find("$terrainName$");
        len = std::string("$terrainName$").length();
        if(pos != s.npos){
            s.replace(pos, len, terrainName);
        }
        
        dataString.append(s);
        
        //cout<<"dataString = \n"<<dataString<<endl;
        
        const char * d = dataString.c_str();
        int nSize = strlen(d);
        
        VSIFWriteL( d, nSize, 1, fout );

        CPLFree(data);
        
        VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
        VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()
        
        return NINJA_SUCCESS;
}

int NinjaFoam::WritePFile(VSILFILE *fin, VSILFILE *fout)
{
        int pos;
        char *data;
        
        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);

        VSIRewindL(fin);
        data = (char*)CPLMalloc(offset * sizeof(char));
        VSIFReadL(data, offset, 1, fin); //read in full p template file
        
        // write to first dictionary value
        std::string dataString;
        std::string s(data);
        pos = s.find("$boundaryField$");
        if(pos != s.npos){
            s.erase(pos);
            dataString.append(s);
        }
        
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
            }
            else{
                template_ = "";
                type = "totalPressure";
                value = "0";
                gammavalue = "1";
                pvalue = "0";
            }
            int status;
            //append BC block for current face
            status = AddBcBlock(dataString);
            if(status != 0){
                //do something
            }
        }
        
        s = data;
        pos = s.find("$boundaryField$");
        int len = std::string("$boundaryField$").length();
        if(pos != s.npos){
            s.erase(0, pos+len);
        }
        
        pos = s.find("$terrainName$");
        len = std::string("$terrainName$").length();
        if(pos != s.npos){
            s.replace(pos, len, terrainName);
        }
        
        dataString.append(s);
        
        //cout<<"dataString = \n"<<dataString<<endl;
        
        const char * d = dataString.c_str();
        int nSize = strlen(d);
        
        VSIFWriteL( d, nSize, 1, fout );

        CPLFree(data);
        
        VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
        VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()
        
        return NINJA_SUCCESS;
}

int NinjaFoam::WriteUFile(VSILFILE *fin, VSILFILE *fout)
{
        int pos;
        char *data;
        
        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);

        VSIRewindL(fin);
        data = (char*)CPLMalloc(offset * sizeof(char));
        VSIFReadL(data, offset, 1, fin); //read in full U template file
        
        // write to first dictionary value
        std::string dataString;
        std::string s(data);
        pos = s.find("$boundaryField$");
        if(pos != s.npos){
            s.erase(pos);
            dataString.append(s);
        }
        
        //append BC blocks from template files
        for(int i = 0; i < bcs.size(); i++){
            boundary_name = bcs[i];
            //check if boundary_name is an inlet
            if(std::find(inlets.begin(), inlets.end(), boundary_name) != inlets.end()){
                template_ = "inlet.tmp";
                type = "logProfileVelocityInlet";
            }
            else{
                template_ = "";
                type = "pressureInletOutletVelocity";
                inletoutletvalue = "(0 0 0)";
            }
            int status;
            status = AddBcBlock(dataString);
            if(status != 0){
                //do something
            }
        }
        
        s = data;
        pos = s.find("$boundaryField$");
        int len = std::string("$boundaryField$").length();
        if(pos != s.npos){
            s.erase(0, pos+len);
        }
        
        pos = s.find("$terrainName$");
        len = std::string("$terrainName$").length();
        if(pos != s.npos){
            s.replace(pos, len, terrainName);
        }
        
        dataString.append(s);
        
        //cout<<"dataString = \n"<<dataString<<endl;
        
        const char * d = dataString.c_str();
        int nSize = strlen(d);
        
        VSIFWriteL( d, nSize, 1, fout );
        
        CPLFree(data);
        
        VSIFCloseL( fin ); // reopened for each file in writeFoamFiles()
        VSIFCloseL( fout ); // reopened for each file in writeFoamFiles()
        
        return NINJA_SUCCESS;
}
