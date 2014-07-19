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
    nFaces = 0;
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

    #ifdef _OPENMP
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Run number %d started with %d threads.", input.inputsRunNumber, input.numberCPUs);
    #endif

    /*------------------------------------------*/
    /*  convert DEM to STL format               */
    /*------------------------------------------*/

    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Converting DEM to STL format...");

    std::string stlFileName = input.dem.fileName;
    stlFileName = stlFileName.replace(stlFileName.end()-3, stlFileName.end(), "stl");

    int pos;
    pos = stlFileName.find_last_of("/");
    stlFileName = stlFileName.substr(pos+1);

    int nBand = 1;
    const char * inFile = input.dem.fileName.c_str();
    const char * outFile = stlFileName.c_str();

    CPLErr eErr;

    eErr = NinjaElevationToStl(inFile,
                        outFile,
                        nBand,
                        NinjaStlBinary,
                        NULL);

    if(eErr != 0){
        //do something
    }

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

    return true;
}

int NinjaFoam::WriteFoamFiles()
{
    const char *pszPath = CPLSPrintf( "/vsizip/%s", CPLGetConfigOption( "WINDNINJA_DATA", NULL ) );
    const char *pszArchive = CPLSPrintf("%s/ninjafoam.zip/ninjafoam", pszPath);

    char **papszFileList = VSIReadDirRecursive( pszArchive );
    const char *osFullPath;
    const char *pszFilename;
    const char *pszOutput;
    const char *pszInput;
    char *data;

    const char *pszTempFoamPath;

    //write temporary OpenFOAM directories
    for(int i = 0; i < CSLCount( papszFileList ); i++){
        pszFilename = CPLGetFilename(papszFileList[i]);
        osFullPath = papszFileList[i];
        if(std::string(pszFilename) == ""){
            pszTempFoamPath = CPLFormFilename(pszTempPath, osFullPath, "");
            VSIMkdir(pszTempFoamPath, 0777);
        }
    }

    //write temporary OpenFOAM files
    VSILFILE *fin;
    VSILFILE *fout;
    for(int i = 0; i < CSLCount( papszFileList ); i++){
        osFullPath = papszFileList[i];
        pszFilename = CPLGetFilename(papszFileList[i]);
        if(std::string(pszFilename) != ""){
            pszInput = CPLFormFilename(pszArchive, osFullPath, "");
            pszOutput = CPLFormFilename(pszTempPath, osFullPath, "");

            fin = VSIFOpenL( pszInput, "r" );
            fout = VSIFOpenL( pszOutput, "w" );

            vsi_l_offset offset;
            VSIFSeekL(fin, 0, SEEK_END);
            offset = VSIFTellL(fin);

            VSIRewindL(fin);
            data = (char*)CPLMalloc(offset * sizeof(char));
            VSIFReadL(data, offset, 1, fin);

            //check user input and update OpenFOAM files

            VSIFWriteL(data, offset, 1, fout);
        }
    }

    CSLDestroy( papszFileList );

    VSIFCloseL( fin );
    VSIFCloseL( fout );
    CPLFree(data);

    return NINJA_SUCCESS;
}

int NinjaFoam::GenerateTempDirectory()
{
    pszTempPath = CPLStrdup( CPLGenerateTempFilename( NULL ) );
    VSIMkdir( pszTempPath, 0777 );
    pszOgrBase = NULL;

    return NINJA_SUCCESS;
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
