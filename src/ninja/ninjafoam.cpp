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

NinjaFoam::NinjaFoam()
{
    pszTerrainFile = NULL;
    pszTempPath = NULL;
    pszOgrBase = NULL;
    hGriddedDS = NULL;
    nFaces = 0;
}

NinjaFoam::~NinjaFoam()
{
    free( (void*)pszTerrainFile );
    free( (void*)pszTempPath );
    free( (void*)pszOgrBase );
    GDALClose( hGriddedDS );
}

int NinjaFoam::SetTerrainFile( const char * pszInputFile )
{
    pszTerrainFile = strdup( pszInputFile );
    return NINJA_SUCCESS;
}

int NinjaFoam::SetNumCpus( int nCpus )
{
    this->nCpus = nCpus;
    return NINJA_SUCCESS;
}

int NinjaFoam::SetSpeed( double dfSpeed )
{
    this->dfSpeed = dfSpeed;
    return NINJA_SUCCESS;
}

int NinjaFoam::SetDirection( double dfDirection )
{
    assert( dfDirection >= 0.0 && dfDirection <= 360.0 );
    this->dfDirection = dfDirection;
    if( dfDirection == 360.0 || dfDirection == 0.0 )
        nFaces = nFaces | NINJA_FOAM_NORTH;
    else if( dfDirection > 0.0 && dfDirection < 90.0 )
        nFaces = nFaces | NINJA_FOAM_NORTH | NINJA_FOAM_EAST;
    else if( dfDirection == 90.0 )
        nFaces = nFaces | NINJA_FOAM_EAST;
    else if( dfDirection > 90.0 && dfDirection < 180.0 )
        nFaces = nFaces | NINJA_FOAM_EAST | NINJA_FOAM_SOUTH;
    else if( dfDirection == 180.0 )
        nFaces = nFaces | NINJA_FOAM_SOUTH;
    else if( dfDirection > 180.0 && dfDirection < 270.0 )
        nFaces = nFaces | NINJA_FOAM_SOUTH | NINJA_FOAM_WEST;
    else if( dfDirection ==270.0 )
        nFaces = nFaces | NINJA_FOAM_WEST;
    else if( dfDirection > 270.0 )
        nFaces = nFaces | NINJA_FOAM_WEST | NINJA_FOAM_NORTH;
    return NINJA_SUCCESS;
}

int NinjaFoam::SetRoughD( double dfRoughD )
{
    this->dfRoughD = dfRoughD;
    return NINJA_SUCCESS;
}

int NinjaFoam::SetRoughness( double dfRoughness )
{
    this->dfRoughness = dfRoughness;
    return NINJA_SUCCESS;
}

int NinjaFoam::SetInputHeight( double dfInputHeight )
{
    this->dfInputHeight = dfInputHeight;
    return NINJA_SUCCESS;
}

int NinjaFoam::WriteJson()
{
    return NINJA_SUCCESS;
}

int NinjaFoam::GenerateTempDirectory()
{
    pszOgrBase = CPLStrdup( CPLGenerateTempFilename( NULL ) );
    pszTempPath = CPLStrdup( CPLGetDirname( pszOgrBase ) );

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
