/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  Test utm zone calculations
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

#ifndef WIN32
#include "ninja_conv.h"

#include <boost/test/unit_test.hpp>

#include "gdal_util.h"

BOOST_AUTO_TEST_SUITE(utm)

struct utmTest {
  double x;
  double y;
  int z;
}utmTest;
/*
** Test on western hemisphere and one eastern, we'll use Boise and Adelaide.
*/
BOOST_AUTO_TEST_CASE(boise) {
  struct utmTest utmTests[]{
      { -116.24053, 43.56704, 32611 },
      { -476.24053, 43.56704, 32611 },
      { -836.24053, 43.56704, 32611 },
  };
  int i = 0;
  int got = 0;
  for(i = 0; i < sizeof(utmTests) / sizeof(utmTest); i++) {
      got = GetUTMZoneInEPSG( utmTests[i].x, utmTests[i].y );
      BOOST_REQUIRE( got == utmTests[i].z );
  }
}

BOOST_AUTO_TEST_CASE(adelaide) {
  struct utmTest utmTests[]{
      { 138.601111, -34.928889, 32754 },
      { 498.601111, -34.928889, 32754 },
      { 858.601111, -34.928889, 32754 },
  };
  int i = 0;
  int got = 0;
  for(i = 0; i < sizeof(utmTests) / sizeof(utmTest); i++) {
      got = GetUTMZoneInEPSG( utmTests[i].x, utmTests[i].y );
      BOOST_REQUIRE( got == utmTests[i].z );
  }
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* WIN32 */
