/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Test nomads fetching
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

#ifdef WITH_NOMADS_SUPPORT

#include "gdal.h"
#include "nomads.h"

#include <boost/test/unit_test.hpp>

static int CheckBands( const char *pszVsiPath )
{

    GDALDatasetH hDS;
    GDALRasterBandH hBand;
    const char *pszMetadata;
    const char *pszVsiFilename;
    int i, j, k;
    int nFiles, nBands;
    char **papszFiles = NULL;
    const char *pszTestPath;
    if( strstr( pszVsiPath, ".zip" ) )
        pszTestPath = CPLStrdup( CPLSPrintf( "/vsizip/%s", pszVsiPath ) );
    else
        pszTestPath = CPLStrdup( pszVsiPath );
    papszFiles = VSIReadDir( pszTestPath );
    nFiles = CSLCount( papszFiles );
    for( i = 0; i < nFiles; i++ )
    {
        SKIP_DOT_AND_DOTDOT( papszFiles[i] );
        pszVsiFilename = CPLSPrintf( "%s/%s", pszTestPath, papszFiles[i] );
        hDS = GDALOpen( pszVsiFilename, GA_ReadOnly );
        if( !hDS )
        {
            CSLDestroy( papszFiles );
            CPLFree( (void*)pszTestPath );
            return 1;
        }
        nBands = GDALGetRasterCount( hDS );
        k = 0;
        for( j = 0; j < nBands; j++ )
        {
            hBand = GDALGetRasterBand( hDS, j + 1 );
            pszMetadata = GDALGetMetadataItem( hBand, "GRIB_ELEMENT", "" );
            if( EQUAL( pszMetadata, "TCDC" ) ||
                EQUAL( pszMetadata, "TMP" ) ||
                EQUAL( pszMetadata, "UGRD" ) ||
                EQUAL( pszMetadata, "VGRD" ) )
            {
                k++;
            }
        }
        GDALClose( hDS );
        if( k < 4 )
        {
            break;
        }
    }
    CSLDestroy( papszFiles );
    CPLFree( (void*)pszTestPath );
    return k;
}

static int CheckZip( const char *pszVsiPath )
{
    if( strstr( pszVsiPath, ".zip" ) && !EQUALN( pszVsiPath, "/vsizip/", 8 ) )
        pszVsiPath = CPLSPrintf( "/vsizip/%s", pszVsiPath );
    char **papszFiles = NULL;
    papszFiles = VSIReadDir( pszVsiPath );
    int rc = 0;
    int n = CSLCount( papszFiles );
    int i;
    for( i = 0; i < n; i++ )
    {
        if( !EQUAL( papszFiles[i], "." ) && !EQUAL( papszFiles[i], ".." ) )
            rc++;
    }
    CSLDestroy( papszFiles );
    return rc;
}

struct setup
{
    setup()
    {
        GDALAllRegister();
        adfMackay[0] = -113.749693430469;
        adfMackay[1] = -113.463446144564;
        adfMackay[2] = 44.0249023401036;
        adfMackay[3] = 43.7832152227745;
        adfAlaska[0] = -146.769276;
        adfAlaska[1] = -146.438977;
        adfAlaska[2] = 63.797157;
        adfAlaska[3] = 63.48351;
        adfHawaii[0] = -155.400;
        adfHawaii[1] = -155.185;
        adfHawaii[2] = 19.7356;
        adfHawaii[3] = 19.4345;
        adfAfrica[0] = 19.495;
        adfAfrica[1] = 19.780;
        adfAfrica[2] = 12.195;
        adfAfrica[3] = 11.780;
        adfSouthAmerica[0] = -11.060;
        adfSouthAmerica[1] = -10.745;
        adfSouthAmerica[2] = -51.10;
        adfSouthAmerica[3] = -51.55;
        adfMackayLarger[0] = adfMackay[0] - 2.0;
        adfMackayLarger[1] = adfMackay[1] + 2.0;
        adfMackayLarger[2] = adfMackay[2] + 2.0;
        adfMackayLarger[3] = adfMackay[3] - 2.0;
        adfTooSmallForGfs[0] = -113.74;
        adfTooSmallForGfs[1] = -113.72;
        adfTooSmallForGfs[2] = 44.02;
        adfTooSmallForGfs[3] = 44.00;
        pszVsiPath = CPLStrdup( CPLGenerateTempFilename( "NOMADS_TEST" ) );
    }
    ~setup()
    {
        VSIUnlink( pszVsiPath );
        CPLFree( (void*)pszVsiPath );
    }

    double adfMackay[4];
    double adfAlaska[4];
    double adfHawaii[4];
    double adfAfrica[4];
    double adfSouthAmerica[4];
    double adfMackayLarger[4];
    double adfTooSmallForGfs[4];
    const char *pszVsiPath;
};

BOOST_FIXTURE_TEST_SUITE( simplenomadsclient, setup )

BOOST_AUTO_TEST_CASE( download_1 )
{
    const char *pszKey = NULL;
    const char *pszWhere = NULL;
    const char *pszVsiType = NULL;
    pszWhere = boost::unit_test::framework::master_test_suite().argv[1];
    pszKey = boost::unit_test::framework::master_test_suite().argv[2];
    int n, hours;
    hours = atoi( boost::unit_test::framework::master_test_suite().argv[3] );
    n = atoi( boost::unit_test::framework::master_test_suite().argv[4] );
    int erc = atoi( boost::unit_test::framework::master_test_suite().argv[5] );
    pszVsiType = boost::unit_test::framework::master_test_suite().argv[6];
    double *pdfBbox;
    if( EQUAL( pszWhere, "mackay" ) )
        pdfBbox = adfMackay;
    else if( EQUAL( pszWhere, "alaska" ) )
        pdfBbox = adfAlaska;
    else if( EQUAL( pszWhere, "hawaii" ) )
        pdfBbox = adfHawaii;
    else if( EQUAL( pszWhere, "africa" ) )
        pdfBbox = adfAfrica;
    else if( EQUAL( pszWhere, "south_america" ) )
        pdfBbox = adfSouthAmerica;
    else if( EQUAL( pszWhere, "mackay_large" ) )
        pdfBbox = adfMackayLarger;
    else if( EQUAL( pszWhere, "small" ) )
        pdfBbox = adfTooSmallForGfs;
    else
        BOOST_REQUIRE( 0 );
    int rc = 0;
    if( EQUAL( pszVsiType, "zip" ) )
    {
        const char *p = pszVsiPath;
        pszVsiPath = CPLStrdup( CPLSPrintf( "%s.zip", pszVsiPath ) );
        CPLFree( (void*)p );
    }
    else if( EQUAL( pszVsiType, "path" ) )
    {
        VSIMkdir( pszVsiPath, 0777 );
    }
    else
        BOOST_REQUIRE( 0 );
    CPLSetConfigOption( "NOMADS_MAX_FCST_REWIND", "3" );
    rc = NomadsFetch( pszKey, NULL, hours, 1, pdfBbox, pszVsiPath, NULL, NULL );
    BOOST_REQUIRE_EQUAL( rc, erc );
    if( rc == 0 )
    {
        rc = CheckZip( pszVsiPath );
        BOOST_CHECK( rc >= n - 1 );
        rc = CheckBands( pszVsiPath );
        if( EQUALN( pszKey, "rtma", 4 ) && !EQUAL( pszKey, "rtma_conus" ) )
            BOOST_CHECK_EQUAL( rc, 3 );
        else if EQUAL( pszKey, "narre" )
            BOOST_CHECK_EQUAL( rc, 2 );
        else if EQUAL( pszKey, "gfs_global" )
            BOOST_CHECK_EQUAL( rc, 3 );
        else
            /* Sometimes we get double variables */
            BOOST_CHECK( rc >= 4 );
    }
}

BOOST_AUTO_TEST_CASE( download_past )
{
    int rc = 0;
    nomads_utc *u;
    NomadsUtcCreate( &u );
    NomadsUtcNow( u );
    NomadsUtcAddHours( u, -96 );
    VSIMkdir( pszVsiPath, 0777 );
    NomadsUtcStrfTime( u, "%Y%m%dT%H:%M:%S" );
    rc = NomadsFetch( "nam_conus", u->s, 6, 1, adfMackay, pszVsiPath, NULL, NULL );
    NomadsUtcFree( u );
    BOOST_REQUIRE_EQUAL( rc, 0 );
    rc = CheckZip( pszVsiPath );
    BOOST_CHECK_EQUAL( rc, 7 );
    rc = CheckBands( pszVsiPath );
    BOOST_CHECK_EQUAL( rc, 4 );
}

BOOST_AUTO_TEST_CASE( stride_valid_1 )
{
    int rc = 0;
    VSIMkdir( pszVsiPath, 0777 );
    rc = NomadsFetch( "nam_conus", NULL, 6, 2, adfMackay, pszVsiPath, NULL, NULL );
    BOOST_REQUIRE_EQUAL( rc, 0 );
    rc = CheckZip( pszVsiPath );
    BOOST_CHECK( rc >= 4 );
    rc = CheckBands( pszVsiPath );
    BOOST_CHECK( rc <= 4 );
}

BOOST_AUTO_TEST_CASE( stride_valid_2 )
{
    int rc = 0;
    VSIMkdir( pszVsiPath, 0777 );
    rc = NomadsFetch( "nam_conus", NULL, 49, 6, adfMackay, pszVsiPath, NULL, NULL );
    BOOST_REQUIRE_EQUAL( rc, 0 );
    rc = CheckZip( pszVsiPath );
    BOOST_CHECK( rc >= 8 );
    rc = CheckBands( pszVsiPath );
    BOOST_CHECK( rc <= 4 );
}

BOOST_AUTO_TEST_CASE( stride_invalid_1 )
{
    /* not implemented */
}

BOOST_AUTO_TEST_CASE( form_name_1 )
{
    char *s;
    s = NomadsFormName( "gfs_global", ' ' );
    BOOST_REQUIRE( s );
#if defined(NOMADS_GFS_0P5DEG)
    BOOST_CHECK( EQUAL( s, "NOMADS GFS GLOBAL 0.5 DEG" ) );
#elif defined(NOMADS_GFS_1P0DEG)
    BOOST_CHECK( EQUAL( s, "NOMADS GFS GLOBAL 1.0 DEG" ) );
#else /* NOMADS_GFS_0.25DEG is default */
    BOOST_CHECK( EQUAL( s, "NOMADS GFS GLOBAL 0.25 DEG" ) );
#endif
}

BOOST_AUTO_TEST_SUITE_END()


#endif /* WITH_NOMADS_SUPPORT */

