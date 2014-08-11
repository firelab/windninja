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

/*
** FIXME!!!
** Strange compilation failure on win32 msvc, see test_srtm.cpp
*/
#ifndef WIN32
#include <boost/test/unit_test.hpp>
#include "gdal_priv.h"

#include "fetch_factory.h"
#include "ninja.h"

/******************************************************************************
*                        "STRM" TEST FIXTURE
******************************************************************************/
struct GmtedTestData
{
    GmtedTestData()
    {
        GDALAllRegister();
        OGRRegisterAll();
        fetch = NULL;
        poDS  = NULL;
        pszFilename = "out.tif";
        RASTER_X_SIZE = 738;
        RASTER_Y_SIZE = 919;
    }
    ~GmtedTestData()
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
    double adfCenter[2];
    double adfBuffer[2];

    SurfaceFetch *fetch;
    GDALDataset  *poDS;

    std::string pszFilename;
    int   RASTER_X_SIZE;
    int   RASTER_Y_SIZE;
};
/******************************************************************************
*                        "GMTED" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       gmted/us
*       gmted/world
******************************************************************************/

BOOST_FIXTURE_TEST_SUITE( gmted, GmtedTestData )


/**
* Test GMTED surface for Mackay, ID, US
*/
BOOST_AUTO_TEST_CASE( us )
{

    /* mackay */
    adfBbox[0] =   44.0249023401036;
    adfBbox[1] = -113.463446144564;
    adfBbox[2] =   43.7832152227745;
    adfBbox[3] = -113.749693430469;

    fetch = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_GMTED);

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


    VSIUnlink(pszFilename.c_str());
    /*If test makes it here, fetch and poDS are not NULL and should be free'd
    for next test. */
    delete fetch;
    GDALClose( (GDALDatasetH) poDS );
    fetch = NULL;
    poDS  = NULL;
}

 /**
*
*/
BOOST_AUTO_TEST_CASE( world )
{
    /* mt everest */
    adfCenter[0] = 86.925285;
    adfCenter[1] = 27.988046;
    adfBuffer[0] = 30.0;
    adfBuffer[1] = 30.0;

    fetch = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_GMTED);

    int rc = fetch->FetchPoint( adfCenter, adfBuffer, lengthUnits::kilometers,
                               90.0, pszFilename.c_str(), NULL );

    BOOST_REQUIRE( rc >= 0 );

    poDS = (GDALDataset*)GDALOpen( pszFilename.c_str(), GA_Update );
    BOOST_REQUIRE_MESSAGE( NULL != poDS, "GDALOpen returned NULL" );

    BOOST_REQUIRE_MESSAGE( !GDALHasNoData( poDS, 1 ), "poDS contains no data after GDALOpen" );

    poDS->GetGeoTransform(adfGeoTransform);

    int nXSize, nYSize;
    nXSize = poDS->GetRasterXSize();
    nYSize = poDS->GetRasterYSize();

    BOOST_CHECK( 0 != nXSize );
    BOOST_CHECK( 0 != nYSize );


    delete fetch;
    VSIUnlink(pszFilename.c_str());
    GDALClose( (GDALDatasetH) poDS );
    fetch = NULL;
    poDS  = NULL;
}


BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "GMTED" BOOST TEST SUITE
*****************************************************************************/

#endif /* WIN32 */



