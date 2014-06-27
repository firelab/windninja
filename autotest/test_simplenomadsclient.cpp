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

#define NOMADS_PATH "_nomads_simple"
#define NOMADS_ZIP "_nomads_simple.zip"

static int CheckBands( const char *pszVsiPath )
{

    GDALDatasetH hDS;
    GDALRasterBandH hBand;
    const char *pszMetadata;
    const char *pszVsiFilename;
    int i, j, k;
    int nFiles, nBands;
    char **papszFiles = NULL;
    if( strstr( pszVsiPath, ".zip" ) )
        pszVsiPath = CPLStrdup( CPLSPrintf( "/vsizip/%s", pszVsiPath ) );
    else
        pszVsiPath = CPLStrdup( pszVsiPath );
    papszFiles = VSIReadDir( pszVsiPath );
    nFiles = CSLCount( papszFiles );
    for( i = 0; i < nFiles; i++ )
    {
        if( EQUAL( papszFiles[i], "." ) || EQUAL( papszFiles[i], ".." ) )
            continue;
        pszVsiFilename = CPLSPrintf( "%s/%s", pszVsiPath, papszFiles[i] );
        hDS = GDALOpen( pszVsiFilename, GA_ReadOnly );
        if( !hDS )
        {
            CSLDestroy( papszFiles );
            CPLFree( (void*)pszVsiPath );
            return 1;
        }
        nBands = GDALGetRasterCount( hDS );
        if( nBands < 4 )
        {
            CSLDestroy( papszFiles );
            CPLFree( (void*)pszVsiPath );
            return 1;
        }
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
            CSLDestroy( papszFiles );
            CPLFree( (void*)pszVsiPath );
            return k;
        }
    }
    CSLDestroy( papszFiles );
    CPLFree( (void*)pszVsiPath );
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
        adfAlaska[1] = -146.738977;
        adfAlaska[2] = 63.797157;
        adfAlaska[3] = 63.78351;
        adfHawaii[0] = -155.400;
        adfHawaii[1] = -155.385;
        adfHawaii[2] = 19.7356;
        adfHawaii[3] = 19.7345;
        adfAfrica[0] = 19.780;
        adfAfrica[1] = 19.795;
        adfAfrica[2] = 12.195;
        adfAfrica[3] = 12.180;
        adfSouthAmerica[0] = -11.060;
        adfSouthAmerica[1] = -11.045;
        adfSouthAmerica[2] = -51.10;
        adfSouthAmerica[3] = -51.25;
        adfMackayLarger[0] = adfMackay[0] - 2.0;
        adfMackayLarger[1] = adfMackay[1] + 2.0;
        adfMackayLarger[2] = adfMackay[2] + 2.0;
        adfMackayLarger[3] = adfMackay[3] - 2.0;
    }
    ~setup()
    {
    }

    double adfMackay[4];
    double adfAlaska[4];
    double adfHawaii[4];
    double adfAfrica[4];
    double adfSouthAmerica[4];
    double adfMackayLarger[4];
};

BOOST_FIXTURE_TEST_SUITE( simplenomadsclient, setup )

BOOST_AUTO_TEST_CASE( download_1 )
{
    const char *pszKey = NULL;
    const char *pszWhere = NULL;
    const char *pszVsiType = NULL;
    const char *pszVsiPath = NULL;
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
    else
        BOOST_REQUIRE( 0 );
    int rc = 0;
    if( EQUAL( pszVsiType, "zip" ) )
    {
        VSIUnlink( pszVsiPath );
        pszVsiPath = NOMADS_ZIP;
    }
    else if( EQUAL( pszVsiType, "path" ) )
    {
        pszVsiPath = NOMADS_PATH;
        CPLUnlinkTree( pszVsiPath );
        VSIMkdir( NOMADS_PATH, 0777 );
    }
    else
        BOOST_REQUIRE( 0 );
    rc = NomadsFetch( pszKey, hours, pdfBbox, pszVsiPath, NULL, NULL );
    BOOST_REQUIRE_EQUAL( rc, erc );
    if( rc == 0 )
    {
        rc = CheckZip( pszVsiPath );
        BOOST_CHECK_EQUAL( rc, n );
        rc = CheckBands( pszVsiPath );
        if( EQUAL( pszKey, "rtma_conus" ) )
            BOOST_CHECK_EQUAL( rc, 3 );
        else if EQUAL( pszKey, "narre" )
            BOOST_CHECK_EQUAL( rc, 2 );
        else
            /* Sometimes we get double variables */
            BOOST_CHECK( rc >= 4 );
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* WITH_NOMADS_SUPPORT */

