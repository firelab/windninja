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

#include "nomads_utc.h"

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        Test utc time class
******************************************************************************/

struct UtcData
{
    UtcData()
    {
        NomadsUtcCreate( &u );
    }
    ~UtcData()
    {
        NomadsUtcFree( u );
    }
    nomads_utc *u;
};

BOOST_FIXTURE_TEST_SUITE( utc, UtcData )

BOOST_AUTO_TEST_CASE( create_1 )
{
    BOOST_CHECK( u );
}

BOOST_AUTO_TEST_CASE( add_hours_1 )
{
    NomadsUtcNow( u );
    int h = u->ts->tm_hour;
    NomadsUtcAddHours( u, 1 );
    BOOST_CHECK_EQUAL( (h + 1) % 24, u->ts->tm_hour );
}

BOOST_AUTO_TEST_CASE( add_hours_2 )
{
    NomadsUtcNow( u );
    int h = u->ts->tm_hour;
    NomadsUtcAddHours( u, 2 );
    BOOST_CHECK_EQUAL( (h + 2) % 24, u->ts->tm_hour );
}

BOOST_AUTO_TEST_CASE( add_hours_3 )
{
    NomadsUtcNow( u );
    int h = u->ts->tm_hour;
    NomadsUtcAddHours( u, 25 );
    BOOST_CHECK_EQUAL( (h + 25) % 24, u->ts->tm_hour );
}

BOOST_AUTO_TEST_CASE( strptime_1 )
{
    NomadsUtcStrpTime( u, "20140611T10:17:58", "%Y%m%dT%H:%M:%S" );
    BOOST_CHECK_EQUAL( u->ts->tm_year, 2014 - 1900 );
    BOOST_CHECK_EQUAL( u->ts->tm_mon, 5 );
    BOOST_CHECK_EQUAL( u->ts->tm_mday, 11 );
    BOOST_CHECK_EQUAL( u->ts->tm_hour, 10 );
    BOOST_CHECK_EQUAL( u->ts->tm_min, 17 );
    BOOST_CHECK_EQUAL( u->ts->tm_sec, 58 );
}

BOOST_AUTO_TEST_CASE( now_1 )
{
    NomadsUtcNow( u );
    BOOST_CHECK( u->ts->tm_year >= 2014 - 1900 );
}

BOOST_AUTO_TEST_SUITE_END()

