/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:
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

/*
*/
#ifndef WIN32
#include <boost/test/unit_test.hpp>

#include <string>

#include "fetch_factory.h"
#include "ninja.h"

#include "cpl_conv.h"

#include "gdal_priv.h"

/******************************************************************************
*                        "SRTM" TEST FIXTURE
******************************************************************************/
struct GdalTestData
{
    GdalTestData()
    {
        GDALAllRegister();
        OGRRegisterAll();
        fetch = NULL;
        poDS  = NULL;
        pszFilename =
            CPLFormFilename( NULL, CPLGenerateTempFilename( "GDAL_TEST" ), ".tif" );
    }
    ~GdalTestData()
    {
        /* SrtmFetch may leave a file */
        if( CPLCheckForFile( (char*) pszFilename.c_str(), NULL ) )
        {
            VSIUnlink( pszFilename.c_str() );
        }
        if( NULL != fetch )
        {
            delete fetch;
        }
        if( NULL != poDS )
        {
            GDALClose( (GDALDatasetH) poDS );
        }
    }
    double adfBbox[4];
    double adfGeoTransform[6];

    SurfaceFetch *fetch;
    GDALDataset  *poDS;

    std::string pszFilename;
};

/******************************************************************************
*                        GDAL FETCH BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       srtm/us_box
*       srtm/world_point
*       srtm/world_box
*       srtm/gdal
******************************************************************************/

BOOST_FIXTURE_TEST_SUITE( gdal_fetch, GdalTestData )

/**
* Test bounding box region fetching functionality on US map.
*/
BOOST_AUTO_TEST_CASE( us_box )
{

    /*Initialize bounding box around Mackay, Idaho*/
    adfBbox[0] =  44.0249023401036 - 0.05;
    adfBbox[1] = -113.463446144564 - 0.05;
    adfBbox[2] =  43.7832152227745 + 0.05;
    adfBbox[3] = -113.749693430469 + 0.05;

    const char *pszDataPath;
    pszDataPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    BOOST_REQUIRE_MESSAGE( pszDataPath != NULL, "WINDNINJA_DATA not set" );
    const char *pszPath = CPLFormFilename( pszDataPath, "mackay", ".tif" );
    fetch = FetchFactory::GetSurfaceFetch(FetchFactory::CUSTOM_GDAL, pszPath );
    BOOST_REQUIRE_MESSAGE( NULL != fetch, "GetSurfaceFetch returned NULL" );

    int rc = fetch->FetchBoundingBox(adfBbox, 30.0, pszFilename.c_str(), NULL);

    BOOST_REQUIRE( rc >= 0 );

    poDS = (GDALDataset*) GDALOpen( pszFilename.c_str(), GA_Update );

    BOOST_REQUIRE_MESSAGE( NULL != poDS, "GDALOpen returned NULL" );

    if(rc > 0)
    {
        GDALFillBandNoData( poDS, 1, 100 );
    }

    BOOST_REQUIRE_MESSAGE( !GDALHasNoData( poDS, 1 ), "poDS contains no data after GDALOpen" );

    poDS->GetGeoTransform(adfGeoTransform);

    BOOST_CHECK( poDS->GetRasterXSize() > 0 );
    BOOST_CHECK( poDS->GetRasterYSize() > 0 );
    delete fetch;
    GDALClose( (GDALDatasetH) poDS );
    fetch = NULL;
    poDS  = NULL;
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* WIN32 */

