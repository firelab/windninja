/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:
 * Author:   Levi Malott <lmnn3@mst.edu>
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


#include "gdal_priv.h"

#include "fetch_factory.h"
#include "ninja.h"
#include <string>

#include <boost/test/unit_test.hpp>
/******************************************************************************
*                        "SRTM" TEST FIXTURE
******************************************************************************/
struct SrtmTestData
{
    SrtmTestData()
    {
        GDALAllRegister();
        OGRRegisterAll();
        fetch = NULL;
        poDS  = NULL;
        pszFilename = "out.tif";
        RASTER_X_SIZE = 738;
        RASTER_Y_SIZE = 919;
    }
    ~SrtmTestData()
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
    int   RASTER_X_SIZE;
    int   RASTER_Y_SIZE;
};

/******************************************************************************
*                        "SRTM" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       srtm/us_box
*       srtm/world_point
*       srtm/world_box
*       srtm/gdal
******************************************************************************/

BOOST_FIXTURE_TEST_SUITE( srtm, SrtmTestData )

/**
* Test bounding box region fetching functionality on US map.
*/
BOOST_AUTO_TEST_CASE( us_box )
{

    /*Initialize bounding box around Mackay, Idaho*/
    adfBbox[0] =  44.0249023401036;
    adfBbox[1] = -113.463446144564;
    adfBbox[2] =  43.7832152227745;
    adfBbox[3] = -113.749693430469;

    fetch = FetchFactory::GetSurfaceFetch(FetchFactory::US_SRTM);
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

    BOOST_CHECK( CPLIsEqual(  279635.0, (int)(adfGeoTransform[0] + 0.5) ) );
    BOOST_CHECK( CPLIsEqual(      30.0,       adfGeoTransform[1] ) );
    BOOST_CHECK( CPLIsEqual(       0.0,       adfGeoTransform[2] ) );
    BOOST_CHECK( CPLIsEqual( 4878315.0, (int)(adfGeoTransform[3] + 0.5) ) );
    BOOST_CHECK( CPLIsEqual(       0.0,       adfGeoTransform[4]) );
    BOOST_CHECK( CPLIsEqual(     -30.0,       adfGeoTransform[5]) );


    int nXSize, nYSize;
    nXSize = poDS->GetRasterXSize();
    nYSize = poDS->GetRasterYSize();

    BOOST_CHECK_EQUAL( RASTER_X_SIZE, nXSize );
    BOOST_CHECK_EQUAL( RASTER_Y_SIZE, nYSize );

    /*If test makes it here, fetch and poDS are not NULL and should be free'd
        for next test. */
    delete fetch;
    GDALClose( (GDALDatasetH) poDS );
    fetch = NULL;
    poDS  = NULL;
}

/**
* Test fetch point coordinate on world map.
*/
BOOST_AUTO_TEST_CASE( world_point )
{
    double adfCenter[2];
    double adfBuffer[2];

    /*Initialize point around Mt. Everest*/
    adfCenter[0] = 86.923596;
    adfCenter[1] = 27.985818;
    adfBuffer[0] = 20.0;
    adfBuffer[1] = 20.0;

    SurfaceFetch *fetch =
        FetchFactory::GetSurfaceFetch( FetchFactory::WORLD_SRTM );

    BOOST_REQUIRE_MESSAGE( NULL != fetch, "GetSurfaceFetch returned NULL" );

    int rc = fetch->FetchPoint( adfCenter, adfBuffer, lengthUnits::kilometers,
                                90.0, pszFilename.c_str(), NULL );
    BOOST_REQUIRE( rc >= 0 );

    poDS = (GDALDataset*) GDALOpen( pszFilename.c_str(), GA_Update );

    BOOST_REQUIRE_MESSAGE( NULL != poDS, "GDALOpen returned NULL" );

    if(rc > 0)
    {
        GDALFillBandNoData( poDS, 1, 100 );
    }

    BOOST_REQUIRE_MESSAGE( !GDALHasNoData( poDS, 1 ), "poDS contains no data after GDALOpen" );


    int nXSize, nYSize;
    nXSize = poDS->GetRasterXSize();
    nYSize = poDS->GetRasterYSize();

    BOOST_CHECK( nXSize >= 1 );
    BOOST_CHECK( nYSize >= 1 );
    /*If test makes it here, fetch and poDS are not NULL and should be free'd
        for next test. */
    delete fetch;
    GDALClose( (GDALDatasetH) poDS );
    fetch = NULL;
    poDS  = NULL;
}

/*TODO: THIS FAILS, HAVE TO FIX DATELINE STUFF */
/**
* Test bounding box on world map.
*/
/*
BOOST_AUTO_TEST_CASE( world_box )
{
    return -1;
    GDALAllRegister();

    double adfBbox[4];
    const char* pszFilename = "out.tif";
    double adfGeoTransform[6];

    // dateline
    adfBbox[0] = 44.0249023401036;
    adfBbox[1] = 179.999;
    adfBbox[2] = 43.7832152227745;
    adfBbox[3] = 180.001;

    SurfaceFetch *fetch =
        FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_SRTM);

    int rc = fetch->FetchBoundingBox(adfBbox, 30.0, pszFilename.c_str(), NULL);
    delete fetch;
    if(rc < 0)
        return rc;

    GDALDataset* poDS;
    poDS = (GDALDataset*)GDALOpen(pszFilename.c_str(), GA_Update);
    if(poDS != NULL)
        return -1;

    if(rc > 0)
    {
        GDALFillBandNoData(poDS, 1, 100);
    }
    if(GDALHasNoData(poDS, 1))
    {
        GDALClose((GDALDatasetH)poDS);
        return 1;
    }

    poDS->GetGeoTransform(adfGeoTransform);

    int nXSize, nYSize;
    nXSize = poDS->GetRasterXSize();
    nYSize = poDS->GetRasterYSize();

    GDALClose((GDALDatasetH)poDS);
    VSIUnlink(pszFilename);
}
*/

BOOST_AUTO_TEST_CASE( gdal )
{
    /* mackay */
    adfBbox[0] = 44.0249023401036;
    adfBbox[1] = -113.463446144564;
    adfBbox[2] = 43.7832152227745;
    adfBbox[3] = -113.749693430469;

    CPLString osPath = FindDataPath( "mackay.tif" );

    fetch = FetchFactory::GetSurfaceFetch(FetchFactory::CUSTOM_GDAL, osPath.c_str() );

    int rc = fetch->FetchBoundingBox(adfBbox, 30.0, pszFilename.c_str(), NULL);
    BOOST_REQUIRE( rc >= 0 );

    poDS = (GDALDataset*)GDALOpen(pszFilename.c_str(), GA_Update);
    BOOST_REQUIRE_MESSAGE( NULL != poDS, "GDALOpen returned NULL" );

    if(rc > 0)
    {
        GDALFillBandNoData(poDS, 1, 100);
    }
    BOOST_REQUIRE_MESSAGE( !GDALHasNoData( poDS, 1 ), "poDS contains no data after GDALOpen" );

    poDS->GetGeoTransform(adfGeoTransform);

    BOOST_CHECK( CPLIsEqual(  279635.0, (int)(adfGeoTransform[0] + 0.5) ) );
    BOOST_CHECK( CPLIsEqual(      30.0,       adfGeoTransform[1] ) );
    BOOST_CHECK( CPLIsEqual(       0.0,       adfGeoTransform[2] ) );
    BOOST_CHECK( CPLIsEqual( 4878315.0, (int)(adfGeoTransform[3] + 0.5) ) );
    BOOST_CHECK( CPLIsEqual(       0.0,       adfGeoTransform[4]) );
    BOOST_CHECK( CPLIsEqual(     -30.0,       adfGeoTransform[5]) );

    int nXSize, nYSize;
    nXSize = poDS->GetRasterXSize();
    nYSize = poDS->GetRasterYSize();

    BOOST_CHECK( RASTER_X_SIZE >= 1 );
    BOOST_CHECK( RASTER_Y_SIZE >= 1 );

    VSIUnlink(pszFilename.c_str());

    /*If test makes it here, fetch and poDS are not NULL and should be free'd
    for next test. */
    delete fetch;
    GDALClose( (GDALDatasetH) poDS );
    fetch = NULL;
    poDS  = NULL;
}

BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "SRTM" BOOST TEST SUITE
*****************************************************************************/

