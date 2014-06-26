/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Nomads weather model initialization
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#include "nomads_wx_init.h"

static int NomadsCompareFileName( const char *pszFile, const char *pszFormat )
{
    const char *a, *b;
    a = pszFile;
    b = pszFormat;
    while( *a++ != '\0' && *b++ != '\0' )
    {
        if( *b == '%' )
        {
            while( isdigit( *a++ ) ); a--;
            while( *b++ != 'd' );
        }
        if( *a != *b )
            return FALSE;
    }
    return TRUE;
}

NomadsWxModel::NomadsWxModel()
{
    pszKey = NULL;
    ppszModelData = NULL;
    pfnProgress = NULL;
}

NomadsWxModel::NomadsWxModel( const char *pszModelKey )
{
    ppszModelData = NULL;
    pfnProgress = NULL;

    int i = 0;
    while( apszNomadsKeys[i][0] != NULL )
    {
        if( EQUAL( pszModelKey, apszNomadsKeys[i][0] ) )
        {
            ppszModelData = apszNomadsKeys[i];
            CPLDebug( "NOMADS", "Found model key: %s",
                      ppszModelData[NOMADS_NAME] );
            break;
        }
        i++;
    }
    if( ppszModelData == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not find model key in nomads data" );
        pszKey = NULL;
    }
    else
    {
        pszKey = CPLStrdup( pszModelKey );
    }
}

NomadsWxModel::~NomadsWxModel()
{
    CPLFree( (void*)pszKey );
}

bool NomadsWxModel::identify( std::string fileName )
{
    const char *pszVsiDir = fileName.c_str();
    char **papszFileList = NULL;
    int nCount;
    papszFileList = VSIReadDir( pszVsiDir );
    if( !papszFileList )
    {
        return FALSE;
    }
    nCount = CSLCount( papszFileList );
    /* Must match one file name format */
    int i = 0;
    int j = 0;
    int bFound = FALSE;
    while( apszNomadsKeys[i][0] != NULL )
    {
        for( j = 0; j < nCount; j++ )
        {
            if( EQUAL( papszFileList[j], ".." ) ||
                EQUAL( papszFileList[j], "." ) )
            {
                continue;
            }
            if( NomadsCompareFileName( papszFileList[j],
                                       apszNomadsKeys[i][NOMADS_FILE_NAME_FRMT] ) )
            {
                bFound = TRUE;
                break;
            }
        }
        i++;
    }
    return bFound;
}

int NomadsWxModel::getEndHour()
{
    if( !ppszModelData )
    {
        return 0;
    }
    char **papszTokens = NULL;
    int nHour = 0;
    int nCount = 0;
    papszTokens = CSLTokenizeString2( ppszModelData[NOMADS_FCST_RUN_HOURS],
                                      ":,", 0 );
    nCount = CSLCount( papszTokens );
    nHour = atoi( papszTokens[nCount - 2] );
    CSLDestroy( papszTokens );
    return nHour;
}

int NomadsWxModel::getStartHour()
{
    if( !ppszModelData )
    {
        return 0;
    }
    char **papszTokens = NULL;
    int nHour = 0;
    int nCount = 0;
    papszTokens = CSLTokenizeString2( ppszModelData[NOMADS_FCST_RUN_HOURS],
                                      ":,", 0 );
    nCount = CSLCount( papszTokens );
    nHour = atoi( papszTokens[0] );
    CSLDestroy( papszTokens );
    return nHour;
}

std::string NomadsWxModel::fetchForecast( std::string demFile, int nHours )
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Model not found" );
    }
    GDALDatasetH hDS = GDALOpen( demFile.c_str(), GA_ReadOnly );

    double adfNESW[4], adfWENS[4];
    ComputeWxModelBuffer( (GDALDataset*)hDS, adfNESW );
    /* Different order for nomads */
    adfWENS[0] = adfNESW[3];
    adfWENS[1] = adfNESW[1];
    adfWENS[2] = adfNESW[0];
    adfWENS[3] = adfNESW[2];

    GDALClose( hDS );

    std::string path( CPLGetDirname( demFile.c_str() ) );
    std::string fileName( CPLGetFilename( demFile.c_str() ) );
    std::string newPath( path + "/" + getForecastIdentifier()
             + "-" + fileName + "/" + 
             std::string(CPLGetBasename(generateForecastName().c_str())) + "/" );

    VSIMkdir( newPath.c_str(), 0777 );

    int rc;
    rc = NomadsFetch( pszKey, nHours, adfWENS, newPath.c_str(), NULL,
                      pfnProgress );
    if( rc )
    {
        throw badForecastFile( "Could not download from nomads server!" );
    }
    return newPath;
}

std::vector<std::string> NomadsWxModel::getVariableList()
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Invalid model" );
    }
    char **papszTokens = NULL;
    papszTokens = CSLTokenizeString2( ppszModelData[NOMADS_VARIABLES], ",", 0 );
    int nCount = CSLCount( papszTokens );
    std::vector<std::string>v;
    int i;
    for( i = 0; i < nCount; i++ )
    {
        v.push_back( std::string( papszTokens[i] ) );
    }
    CSLDestroy( papszTokens );
    return v;
}

std::string NomadsWxModel::getForecastIdentifier()
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Invalid Model" );
    }
    return std::string( ppszModelData[NOMADS_NAME] );
}

std::string NomadsWxModel::getForecastReadable()
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Invalid Model" );
    }
    std::string s = "NOMADS ";
    s += ppszModelData[NOMADS_HUMAN_READABLE];
    s += " ";
    s += ppszModelData[NOMADS_GRID_RES];
    return s;
}

std::vector<blt::local_date_time>
NomadsWxModel::getTimeList( std::string timeZoneString )
{
    return std::vector<blt::local_date_time>();
}
std::vector<blt::local_date_time>
NomadsWxModel::getTimeList( const char *pszVariable,
                            std::string timeZoneString )
{
    return std::vector<blt::local_date_time>();
}
std::vector<blt::local_date_time>
NomadsWxModel::getTimeList( blt::time_zone_ptr timeZonePtr )
{
    return std::vector<blt::local_date_time>();
}
std::vector<blt::local_date_time>
NomadsWxModel::getTimeList( const char *pszVariable, 
                            blt::time_zone_ptr timeZonePtr )
{
    return std::vector<blt::local_date_time>();
}
void NomadsWxModel::setSurfaceGrids( WindNinjaInputs &input,
                                     AsciiGrid<double> &airGrid,
                                     AsciiGrid<double> &cloudGrid,
                                     AsciiGrid<double> &uGrid,
                                     AsciiGrid<double> &vGrid,
                                     AsciiGrid<double> &wGrid )
{
    return;
}

void NomadsWxModel::checkForValidData()
{
    return;
}
