/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test AsciiGrid buffer
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

#include "ascii_grid.h"
#include "omp_guard.h"

#include <iostream>

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        "BUFFER_GRID" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       buffer_grid/init_and_set
******************************************************************************/

BOOST_AUTO_TEST_SUITE( buffer_grid )

/** 
* 
*/
BOOST_AUTO_TEST_CASE( init_and_set )
{
    AsciiGrid<int>grid(2, 4, 0, 0, 1, -1, 10);
    grid.set_cellValue(0, 0, 0);
    grid.set_cellValue(1, 0, 5);
    grid.set_cellValue(2, 0, 5);
    grid.set_cellValue(3, 0, 5);
    grid.set_cellValue(3, 1, 20);
    AsciiGrid<int>buffer = grid.BufferGrid();
    
    BOOST_CHECK( buffer.get_cellValue(0, 0) == 0 );
    BOOST_CHECK( buffer.get_cellValue(1, 0) == 5 );
    BOOST_CHECK( buffer.get_cellValue(2, 0) == 5 );
    BOOST_CHECK( buffer.get_cellValue(3, 0) == 5 );
    BOOST_CHECK( buffer.get_cellValue(4, 0) == 5 );
    BOOST_CHECK( buffer.get_cellValue(3, 1) == 20 );
    BOOST_CHECK( buffer.get_cellValue(4, 1) == 20 );
    BOOST_CHECK( buffer.get_cellValue(3, 2) == 20 );
    BOOST_CHECK( buffer.get_cellValue(0, 1) == 10 );
    BOOST_CHECK( buffer.get_cellValue(0, 2) == 10 );
    BOOST_CHECK( buffer.get_cellValue(1, 1) == 10 );
    BOOST_CHECK( buffer.get_cellValue(1, 2) == 10 );
    BOOST_CHECK( buffer.get_cellValue(2, 1) == 10 );
    BOOST_CHECK( buffer.get_cellValue(2, 2) == 10 );
}

BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "BUFFER_GRID" BOOST TEST SUITE
*****************************************************************************/
