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

#include <string>

#include "nomads_model_def.h"

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        NOMADS test suite
******************************************************************************/

/*
** Expected failures (currently):
** rtma_update_times
** BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES( rtma_update_times, 1 )
*/

struct NomadsTestData
{
    std::vector<NomadsWxModel*>models;
    unsigned int nModelCount;

    NomadsTestData()
    {
        models.push_back( new NamWxModel( "awphys" ) );
        models.push_back( new RapWxModel( "awp130bgrbf" ) );
        models.push_back( new GfsWxModel( "master.grbf" ) );
        models.push_back( new RtmaWxModel() );
        models.push_back( new HiResWWxModel( "westarw/awp4km" ) );
        models.push_back( new NamWxModel( "awip12" ) );
        models.push_back( new HiResWWxModel( "westarw/awpreg" ) );
        models.push_back( new HiResWWxModel( "westnmm/awpreg" ) );
        models.push_back( new NamWxModel( "conusnest.hiresf" ) );
        models.push_back( new NamWxModel( "alaskanest.hiresf" ) );
        models.push_back( new NamWxModel( "hawaiinest.hiresf" ) );
        models.push_back( new NamWxModel( "priconest.hiresf" ) );
        models.push_back( new NamWxModel( "firewxnest.hiresf" ) );
        models.push_back( new NamWxModel( "smartconus" ) );
        models.push_back( new NamWxModel( "smartak" ) );
        models.push_back( new NamWxModel( "smarthi" ) );
        models.push_back( new NamWxModel( "smartpr" ) );
        models.push_back( new NamWxModel( "smartconus2p5" ) );
        nModelCount = models.size();
    }
    ~NomadsTestData()
    {
        for( unsigned int i = 0; i < nModelCount; i++ )
            delete models[i];
    }
};

BOOST_FIXTURE_TEST_SUITE( nomadsclient, NomadsTestData )

BOOST_AUTO_TEST_CASE( nam_awphys_update_times )
{
    BOOST_CHECK( !models[0]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( rap_update_times )
{
    BOOST_CHECK( !models[1]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( gfs_update_times )
{
    BOOST_CHECK( !models[2]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( rtma_update_times )
{
    //BOOST_CHECK( !models[3]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( westarw_awp4km_update_times )
{
    //BOOST_CHECK( !models[4]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_awip12_update_times )
{
    BOOST_CHECK( !models[5]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( westarw_awpreg_update_times )
{
    BOOST_CHECK( !models[6]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( westnmm_awpreg_update_times )
{
    BOOST_CHECK( !models[7]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_conusnest_update_times )
{
    BOOST_CHECK( !models[8]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_alaskanest_update_times )
{
    BOOST_CHECK( !models[9]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_hawaiinest_update_times )
{
    BOOST_CHECK( !models[10]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_priconest_update_times )
{
    BOOST_CHECK( !models[11]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_firewxnest_update_times )
{
    BOOST_CHECK( !models[12]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_smartconus_update_times )
{
    BOOST_CHECK( !models[13]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_smartak_update_times )
{
    BOOST_CHECK( !models[14]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_smarthi_update_times )
{
    BOOST_CHECK( !models[15]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_smartprico_update_times )
{
    BOOST_CHECK( !models[16]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( nam_smartconus2p5_update_times )
{
    BOOST_CHECK( !models[17]->TestUpdateTimes() );
}

BOOST_AUTO_TEST_CASE( generate_time_list )
{
    NamWxModel wx;
    BOOST_CHECK( !wx.TestGenerateDownloadList() );
}

BOOST_AUTO_TEST_CASE( full_test_1 )
{
    //NamWxModel nam = NamWxModel( "awphys" );
    //BOOST_CHECK( !nam.TestUpdateTimesAndBuildFileList() );
    //PASS
}

BOOST_AUTO_TEST_CASE( nam_creation )
{
    NamWxModel nam = NamWxModel( "awphys" );
    BOOST_CHECK( nam.GetSubModelName() == "awphys" );
    BOOST_CHECK( nam.GetLastForecastHour() == 84 );
    std::vector<int> oHours = nam.GetForecastRunTimes();
    std::vector<int> oExpected;
    oExpected.push_back( 0 );
    oExpected.push_back( 6 );
    oExpected.push_back( 12 );
    oExpected.push_back( 18 );
    for( unsigned int i = 0; i < oExpected.size();i++ )
        BOOST_CHECK( oExpected[i] == oHours[i] );

    oHours = nam.GetForecastHourSequence();
    BOOST_CHECK( oHours[0] == 0 );
    BOOST_CHECK( oHours[oHours.size() - 1] == 84 );
    BOOST_CHECK( oHours[37] == 39 );
}

BOOST_AUTO_TEST_CASE( addhours )
{
    NamWxModel wx;
    BOOST_CHECK( !wx.TestAddHours() );
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* WITH_NOMADS_SUPPORT */

