/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Global initialization
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
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

#include "ninja_init.h"

void NinjaCheckThreddsData( void *rc )
{
    int *r;
    if( rc )
        r = (int*)rc;
    std::string p = FindDataPath( "thredds.csv" );
    if( p == "" )
    {
        *r = 1;
        return;
    }
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", "5" );
    CPLHTTPResult *poResult;
    poResult = CPLHTTPFetch( "https://collab.firelab.org/svn/windninja/trunk/data/thredds.csv", NULL );
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", NULL );
    if( !poResult || poResult->nStatus != 0 || poResult->nDataLen == 0 )
    {
        *r = 1;
        return;
    }
    VSILFILE *fout;
    fout = VSIFOpenL( p.c_str(), "wb" );
    if( !fout )
    {
        *r = 1;
        CPLHTTPDestroyResult( poResult );
        return;
    }
    if( !VSIFWriteL( poResult->pabyData, poResult->nDataLen, 1, fout ) )
        *r = 1;
    else
        *r = 0;
    VSIFCloseL( fout );
    CPLHTTPDestroyResult( poResult );
    return;
}

/*
** Initialize global singletons and environments.
*/
int NinjaInitialize()
{
    GDALAllRegister();
    OGRRegisterAll();
#ifdef WIN32
    CPLDebug( "WINDNINJA", "Setting GDAL_DATA..." );
    std::string osGdalData;
    const char *pszGdalData = CPLGetConfigOption( "GDAL_DATA", NULL );
    if( pszGdalData == NULL )
    {
        osGdalData = FindDataPath( "gdal-data/data/gdalicon.png" );
        pszGdalData = CPLGetPath( osGdalData.c_str() );
        CPLDebug( "WINDNINJA", "Setting GDAL_DATA:%s", pszGdalData );
        CPLSetConfigOption( "GDAL_DATA", pszGdalData );
    }
    else
    {
        CPLDebug( "WINDNINJA", "Setting GDAL_DATA from user environment..." );
    }
    CPLDebug( "WINDNINJA", "Setting GDAL_DATA to %s", pszGdalData );
#endif
    /* Try to update our thredds file */
    CPLDebug( "NINJA", "Attempting to download the thredds.csv file" );
    int rc;
    NinjaCheckThreddsData( (void*) &rc );
    //CPLCreateThread( NinjaCheckThreddsData, (void*) &rc );

    return 0;
}

