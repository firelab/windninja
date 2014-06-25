/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Test Array2D functions
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


#include "ascii_grid.h"
#include "Array2D.h"

#include <boost/test/unit_test.hpp>
/******************************************************************************
*                        "ARRAY2D" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       array2d/constructor
******************************************************************************/

BOOST_AUTO_TEST_SUITE( array2d )

/**
* Test constructor
*/
BOOST_AUTO_TEST_CASE( constructor )
{
    Array2D<int>a( 10, 5, -9999 );

    BOOST_CHECK( a.get_numRows() == 10 );
    BOOST_CHECK( a.get_numCols() == 5 );

    a( 0, 0 ) = 1;
    BOOST_CHECK( a( 0, 0 ) == 1 );

    int i = a( 0, 0 );
    BOOST_CHECK( i == 1 );

    i = 10;
    BOOST_CHECK( a( 0, 0 ) == 1 );

    int &j = a( 0, 0 );
    j = 10;
    BOOST_CHECK( a( 0, 0 ) == 10 );
}

BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "ARRAY2D" BOOST TEST SUITE
*****************************************************************************/
