/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test reading the input point comparison
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
#ifdef WITH_LCP_CLIENT

#include <string>

#include "landfireclient.h"

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        BEGIN "landfireclient" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       landfireclient/mackay
*       landfireclient/alaska
*       landfireclient/hawaii
*       landfireclient/unzip
******************************************************************************/
struct LandfireTestData
{
    LandfireTestData()
    {
        GDALAllRegister();
        OGRRegisterAll();
        osLcpFile = "lcp.lcp";
        osPrjFile = "lcp.prj";
        CPLSetConfigOption( "LCP_DOWNLOAD_WAIT", "10" );
    }
    ~LandfireTestData()
    {
        if( CPLCheckForFile( (char*)osLcpFile.c_str(), NULL ) )
        {
            VSIUnlink( osLcpFile.c_str() );
        }
        if( CPLCheckForFile( (char*)osPrjFile.c_str(), NULL ) )
        {
            VSIUnlink( osPrjFile.c_str() );
        }
    }
    double adfBbox[4];
    CPLString osLcpFile;
    CPLString osPrjFile;
    LandfireClient lfcFetcher;
};

BOOST_FIXTURE_TEST_SUITE( landfireclient, LandfireTestData )

BOOST_AUTO_TEST_CASE( mackay )
{
    /*
    ** This is a typical size for windninja, but the landfire client is slow.
    ** Increase the time out period so we can hope to get a real answer and a
    ** real-world test.
    */
    /* mackay */
    adfBbox[0] =   44.0249023401036;
    adfBbox[1] = -113.463446144564;
    adfBbox[2] =   43.7832152227745;
    adfBbox[3] = -113.749693430469;

    int rc = lfcFetcher.FetchBoundingBox( adfBbox, 0.0, osLcpFile.c_str() , NULL );
    BOOST_CHECK( CPLCheckForFile( (char*)osLcpFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( CPLCheckForFile( (char*)osPrjFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( alaska )
{
    /* alaska */
    adfBbox[0] =  63.797157;
    adfBbox[1] = -146.738977;
    adfBbox[2] = 63.78351;
    adfBbox[3] = -146.769276;

    int rc = lfcFetcher.FetchBoundingBox( adfBbox, 0.0, osLcpFile.c_str() , NULL );
    BOOST_CHECK( CPLCheckForFile( (char*)osLcpFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( CPLCheckForFile( (char*)osPrjFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( alaska_bad_geom )
{
    /* alaska */
    adfBbox[0] =  67.272;
    adfBbox[1] = -149.851;
    adfBbox[2] = 67.17;
    adfBbox[3] = -149.99;

    int rc = lfcFetcher.FetchBoundingBox( adfBbox, 0.0, osLcpFile.c_str() , NULL );
    BOOST_CHECK( CPLCheckForFile( (char*)osLcpFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( CPLCheckForFile( (char*)osPrjFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( alaska_out )
{
    /* alaska */
    adfBbox[0] =  56.7644;
    adfBbox[1] = -157.5026;
    adfBbox[2] = 56.6964;
    adfBbox[3] = -157.5511;

    int rc = lfcFetcher.FetchBoundingBox( adfBbox, 0.0, osLcpFile.c_str() , NULL );
    BOOST_CHECK( rc == SURF_FETCH_E_BAD_INPUT );
}

BOOST_AUTO_TEST_CASE( hawaii )
{
    /* hawaii */
    adfBbox[0] =  19.7356;
    adfBbox[1] = -155.385;
    adfBbox[2] =  19.7345;
    adfBbox[3] = -155.400;

    int rc = lfcFetcher.FetchBoundingBox( adfBbox, 0.0, osLcpFile.c_str() , NULL );
    BOOST_CHECK( CPLCheckForFile( (char*)osLcpFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( CPLCheckForFile( (char*)osPrjFile.c_str(), NULL ) == TRUE );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( unzip )
{
    BOOST_CHECK( lfcFetcher.SelfTest() == 0 );
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* WITH_LCP_CLIENT */
/******************************************************************************
*                        END "landfireclient" BOOST TEST SUITE
*****************************************************************************/
