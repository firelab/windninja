/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Unit test for timezone functionality
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
 
#include "gdal_util.h"

#include <boost/test/unit_test.hpp>


/******************************************************************************
*                        "TIMEZONE" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       timezones/boise
******************************************************************************/

BOOST_AUTO_TEST_SUITE( timezones )

/** 
* Test Mackay, Idaho coordinate returns "America/Boise" timezone.
*/
BOOST_AUTO_TEST_CASE( boise )
{
    OGRRegisterAll();
    /* Mackay, Idaho */
    double dfX = -113.613611;
    double dfY = 43.911944;
    std::string tz = FetchTimeZone( dfX, dfY, NULL );
    BOOST_CHECK( tz == "America/Boise" );
}


BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "TIMEZONE" BOOST TEST SUITE
*****************************************************************************/



