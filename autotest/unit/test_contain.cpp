/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Test for NinjaOGRContain
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
#include <string>

#include "ninja_conv.h"
#include "gdal_util.h"

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        "CONTAIN" TEST FIXTURE
******************************************************************************/
struct ContainTestData
{
    ContainTestData()
    {
        OGRRegisterAll();
        oFile = FindDataPath( "us_srtm_region.shp" );
    }
    ~ContainTestData()
    {

    }
    std::string oFile;
};

/******************************************************************************
*                        "CONTAIN" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       contain/points
*       contain/polygon
*       contain/polygon2
*****************************************************************************/


BOOST_FIXTURE_TEST_SUITE( contain, ContainTestData )

/**
* Test NinjaOGRContain with various points. One passing case and one failing.
*/
BOOST_AUTO_TEST_CASE( points )
{
    /* Mackay, Idaho */
    const char *pszWktMackay = "POINT( -113.613611 43.911944 )";
    int bContains;
    bContains = NinjaOGRContain( pszWktMackay, oFile.c_str(), NULL );
    BOOST_CHECK( bContains );

    /* Mongolia-ish?  Should fail. */
    const char *pszWktMongolia = "POINT( 103.0 49.0 )";
    bContains = NinjaOGRContain( pszWktMongolia, oFile.c_str(), NULL );
    BOOST_CHECK( !bContains );

    /* Hawaii */
    const char *pszWktHawaii = "POINT( -155.485438 19.664368 )";
    bContains = NinjaOGRContain( pszWktHawaii, oFile.c_str(), NULL );
    BOOST_CHECK( bContains );
    
    /* Southern Alaska */
    const char *pszWktSouthAlaska = "POINT( -159.348676 56.291966 )";
    bContains = NinjaOGRContain( pszWktSouthAlaska, oFile.c_str(), NULL );
    BOOST_CHECK( bContains );
    
    /* Northern Alaska */
    const char *pszWktNorthAlaska = "POINT( -160.794447 62.412105 )";
    bContains = NinjaOGRContain( pszWktNorthAlaska, oFile.c_str(), NULL );
    BOOST_CHECK( !bContains );
}

/**
* Test NinjaOGRContain with various polygons. One passing case and one failing.
*/
BOOST_AUTO_TEST_CASE( polygon )
{
    /* Mackay, Idaho */
    const char *pszWktMackay = "POLYGON((-113.633285 43.926589,"
                                        "-113.633285 43.904773,"
                                        "-113.577621 43.904773,"
                                        "-113.577621 43.926589,"
                                        "-113.633285 43.926589))";
    int bContains;
    bContains = NinjaOGRContain( pszWktMackay, oFile.c_str(), NULL );
    BOOST_CHECK( bContains );
    /*
    const char *pszWktBadOrd = "POLYGON((-113.577621 43.926589,"
                               "         -113.633285 43.926589,"
                               "         -113.633285 43.904773,"
                               "         -113.633285 43.904773,"
                               "         -113.577621 43.926589))";
    bContains = NinjaOGRContain( pszWktBadOrd, oFile.c_str(), NULL );
    if( !bContains )
        return 1;
    */

    //Should fail
    const char *pszWktOut = "POLYGON((101.160397 47.066675,"
                            "         101.260397 47.066675,"
                            "         101.260397 46.966675,"
                            "         101.160397 46.966675,"
                            "         101.160397 47.066675))";
    bContains = NinjaOGRContain( pszWktOut, oFile.c_str(), NULL );
    BOOST_CHECK( !bContains );
}

/*

*/
BOOST_AUTO_TEST_CASE( polygon2 )
{
    const char *pszWktBadOrd = "POLYGON((-113.577621 43.926589,"
                               "         -113.633285 43.926589,"
                               "         -113.633285 43.904773,"
                               "         -113.577621 43.904773,"
                               "         -113.577621 43.926589))";
    int bContains = NinjaOGRContain( pszWktBadOrd, oFile.c_str(), NULL );
    BOOST_CHECK( bContains );
}


BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "CONTAIN" BOOST TEST SUITE
*****************************************************************************/
