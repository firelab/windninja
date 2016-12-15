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

boost::local_time::tz_database globalTimeZoneDB;

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
    poResult = CPLHTTPFetch( "http://windninja.org/cgi-bin/ninjavisit?thredds=1", NULL );
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", NULL );
    if( !poResult || poResult->nStatus != 0 || poResult->nDataLen == 0 )
    {
        CPLDebug( "NINJA", "Failed to download thredds file." );
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
	int rc = 0;
#ifdef WIN32
    CPLDebug( "WINDNINJA", "Setting GDAL_DATA..." );
    std::string osGdalData;
    osGdalData = FindDataPath( "gdal-data/data/gdalicon.png" );
    const char *pszGdalData = CPLGetPath( osGdalData.c_str() );
    CPLDebug( "WINDNINJA", "Setting GDAL_DATA:%s", pszGdalData );
    CPLSetConfigOption( "GDAL_DATA", pszGdalData );

#if defined(FIRELAB_PACKAGE)
    /*
    ** Setting the CURL_CA_BUNDLE variable through GDAL doesn't seem to work,
    ** but could be investigated in the future.  CURL_CA_BUNDLE can only be set in GDAL
    ** >2.1.2. Test with CPL_CURL_VERBOSE.  For #231.
    **
    ** For now, just skip the SSL verification with GDAL_HTTP_UNSAFESSL.
    */
    CPLSetConfigOption( "GDAL_HTTP_UNSAFESSL", "YES");

#endif /* defined(FIRELAB_PACKAGE) */

#if defined(NINJAFOAM) && defined(FIRELAB_PACKAGE)
    char *pszExecPath;
    const char *pszFoamLibPath = "platforms/linux64mingw-w64DPOpt/lib";
    const char *pszFoamBinPath = "platforms/linux64mingw-w64DPOpt/bin";
    const char *pszTmp;

    pszExecPath = (char*)CPLMalloc( 8192 );
    rc = CPLGetExecPath( pszExecPath, 8192 );
    /*
    ** Set the WM_PROJECT_DIR.  This should point to the installation, with etc
    ** and platforms folders
    */
    rc = _putenv( CPLSPrintf("WM_PROJECT_DIR=%s", CPLGetPath( pszExecPath ) ) );

    /*
    ** Set the PATH variable to point to the lib and bin folders in the open
    ** foam installation.
    */
    pszTmp = CPLSPrintf( "PATH=%s;%s;%PATH%",
                         CPLFormFilename( CPLGetPath( pszExecPath ), pszFoamBinPath, NULL ),
                         CPLFormFilename( CPLGetPath( pszExecPath ), pszFoamLibPath, NULL ) );
    rc = _putenv( pszTmp );
    CPLFree( (void*)pszExecPath );
#endif /* defined(NINJAFOAM) && defined(FIRELAB_PACKAGE)*/

#endif
    /*
    ** Set windninja data if it isn't set.
    */
    if( !CSLTestBoolean( CPLGetConfigOption( "WINDNINJA_DATA", "FALSE" ) ) )
    {
        std::string osDataPath;
        osDataPath = FindDataPath( "tz_world.zip" );
        if( osDataPath != "" )
        {
            CPLSetConfigOption( "WINDNINJA_DATA", CPLGetPath( osDataPath.c_str() ) );
        }
    }
    rc = TRUE;
#ifndef DISABLE_THREDDS_UPDATE
    /*
    ** Disable VSI caching, this breaks nomads downloader if it's on.
    */
    CPLSetConfigOption( "VSI_CACHE", "FALSE" );

    /* Try to update our thredds file */
    rc = CSLTestBoolean( CPLGetConfigOption( "NINJA_DISABLE_THREDDS_UPDATE",
                                                 "NO" ) );
    if( rc == FALSE )
    {
        CPLDebug( "WINDNINJA", "Attempting to download the thredds.csv file" );
        NinjaCheckThreddsData( (void*) &rc );
        //CPLCreateThread( NinjaCheckThreddsData, (void*) &rc );
    }
#endif /* DISABLE_THREDDS_UPDATE */
#ifdef NINJA_ENABLE_CALL_HOME
    if( !CSLTestBoolean( CPLGetConfigOption( "NINJA_DISABLE_CALL_HOME", "NO" ) ) )

    {
        if( rc == TRUE )
        {
            CPLHTTPResult *poResult;
            poResult = CPLHTTPFetch( "http://windninja.org/cgi-bin/ninjavisit?visit=1", NULL );
            CPLHTTPDestroyResult( poResult );
        }
    }
#endif
    globalTimeZoneDB.load_from_file(FindDataPath("date_time_zonespec.csv"));
    return 0;
}

