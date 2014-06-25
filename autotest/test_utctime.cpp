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

#include "utc_time.h"

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        Test utc time class
******************************************************************************/

BOOST_AUTO_TEST_SUITE( utc )

BOOST_AUTO_TEST_CASE( create_1 )
{
    NinjaUtcTime t( 2014, 6, 11, 10, 17, 58 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 6 );
    BOOST_CHECK_EQUAL( t.GetDay(), 11 );
    BOOST_CHECK_EQUAL( t.GetHour(), 10 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_1 )
{
    NinjaUtcTime t( 2014, 6, 11, 10, 17, 58 );
    t.AddHours( 1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 6 );
    BOOST_CHECK_EQUAL( t.GetDay(), 11 );
    BOOST_CHECK_EQUAL( t.GetHour(), 11 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_2 )
{
    NinjaUtcTime t( 2014, 6, 11, 23, 17, 58 );
    t.AddHours( 1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 6 );
    BOOST_CHECK_EQUAL( t.GetDay(), 12 );
    BOOST_CHECK_EQUAL( t.GetHour(), 0 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_3 )
{
    NinjaUtcTime t( 2014, 6, 30, 23, 17, 58 );
    t.AddHours( 1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 7 );
    BOOST_CHECK_EQUAL( t.GetDay(), 1 );
    BOOST_CHECK_EQUAL( t.GetHour(), 0 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_4 )
{
    NinjaUtcTime t( 2014, 12, 31, 23, 17, 58 );
    t.AddHours( 1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2015 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 1 );
    BOOST_CHECK_EQUAL( t.GetDay(), 1 );
    BOOST_CHECK_EQUAL( t.GetHour(), 0 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_5 )
{
    NinjaUtcTime t( 2014, 6, 11, 10, 17, 58 );
    t.AddHours( -1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 6 );
    BOOST_CHECK_EQUAL( t.GetDay(), 11 );
    BOOST_CHECK_EQUAL( t.GetHour(), 9 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_6 )
{
    NinjaUtcTime t( 2014, 6, 11, 0, 17, 58 );
    t.AddHours( -1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 6 );
    BOOST_CHECK_EQUAL( t.GetDay(), 10 );
    BOOST_CHECK_EQUAL( t.GetHour(), 23 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_7 )
{
    NinjaUtcTime t( 2014, 6, 1, 0, 17, 58 );
    t.AddHours( -1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2014 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 5 );
    BOOST_CHECK_EQUAL( t.GetDay(), 31 );
    BOOST_CHECK_EQUAL( t.GetHour(), 23 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( add_hours_8 )
{
    NinjaUtcTime t( 2014, 1, 1, 0, 17, 58 );
    t.AddHours( -1 );
    BOOST_CHECK_EQUAL( t.GetYear(), 2013 );
    BOOST_CHECK_EQUAL( t.GetMonth(), 12 );
    BOOST_CHECK_EQUAL( t.GetDay(), 31 );
    BOOST_CHECK_EQUAL( t.GetHour(), 23 );
    BOOST_CHECK_EQUAL( t.GetMinute(), 17 );
    BOOST_CHECK_EQUAL( t.GetSecond(), 58 );
}

BOOST_AUTO_TEST_CASE( strftime_1 )
{
    NinjaUtcTime t( 2014, 6, 11, 10, 17, 58 );
    const char *expected = "20140611T10:17:58";
    const char *s = t.StrFTime( "%Y%m%dT%H:%M:%S" );
    int rc;
    rc = strcmp( expected, s );
    BOOST_CHECK_EQUAL( rc, 0 );
}

BOOST_AUTO_TEST_CASE( now_1 )
{
    NinjaUtcTime t;
    BOOST_CHECK( t.GetYear() >= 2014 );
    BOOST_CHECK( t.GetMonth() >= 6 );
    BOOST_CHECK( t.GetDay() >= 17 );
}

BOOST_AUTO_TEST_SUITE_END()

