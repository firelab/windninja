/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test AsciiGrid interpolation performance
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
#include "ninja_conv.h"
#include "omp_guard.h"

#include <boost/test/unit_test.hpp>

/******************************************************************************
*                        "GRID_INTERP" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*       grid_interp/order
******************************************************************************/

BOOST_AUTO_TEST_SUITE( grid_interp )

/** 
* Test opening a grid and different interpolation methods
*/
BOOST_AUTO_TEST_CASE( order )
{
    GDALAllRegister();
    std::string oPath = FindDataPath("mackay.tif");
    AsciiGrid<double>grid;
    grid.GDALReadGrid(oPath, 1);
    grid.resample_Grid_in_place(100, AsciiGrid<double>::order0);
    //NEED SOME SORT OF CHECK HERE
    
    
    grid.resample_Grid_in_place(100, AsciiGrid<double>::order1);
    //NEED SOME SORT OF CHECK HERE
    
}


BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "GRID_INTERP" BOOST TEST SUITE
*****************************************************************************/
