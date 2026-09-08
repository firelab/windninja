/******************************************************************************
*
* Project:  WindNinja
* Purpose:  Unit tests for ninjaUnits
* Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
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

#include <boost/test/unit_test.hpp>
#include "ninjaUnits.h"

/******************************************************************************
*                        "BASE_UNITS" BOOST TEST SUITE
*******************************************************************************
*   Tests:
*      base_units/length_to_base_units
******************************************************************************/

BOOST_AUTO_TEST_SUITE(base_units)

BOOST_AUTO_TEST_CASE(length_to_base_units)
{
    double value;
    const double tolerance = 1e-4;
    
    // Test feet conversion
    value = 3.28084;
    lengthUnits::toBaseUnits(value, lengthUnits::feet);
    BOOST_CHECK_CLOSE(value, 1.0, tolerance);
    
    // Test meters (no change)
    value = 5.0;
    lengthUnits::toBaseUnits(value, lengthUnits::meters);
    BOOST_CHECK_CLOSE(value, 5.0, tolerance);
    
    // Test miles conversion
    value = 1.0;
    lengthUnits::toBaseUnits(value, lengthUnits::miles);
    BOOST_CHECK_CLOSE(value, 1609.344, tolerance);
    
    // Test kilometers conversion
    value = 1.0;
    lengthUnits::toBaseUnits(value, lengthUnits::kilometers);
    BOOST_CHECK_CLOSE(value, 1000.0, tolerance);
    
    // Test feetTimesTen conversion
    value = 32.8084;
    lengthUnits::toBaseUnits(value, lengthUnits::feetTimesTen);
    BOOST_CHECK_CLOSE(value, 1.0, tolerance);
    
    // Test metersTimesTen conversion
    value = 10.0;
    lengthUnits::toBaseUnits(value, lengthUnits::metersTimesTen);
    BOOST_CHECK_CLOSE(value, 1.0, tolerance);
    
    // Test invalid unit handling
    value = 1.0;
    BOOST_CHECK_THROW(
        lengthUnits::toBaseUnits(value, static_cast<lengthUnits::eLengthUnits>(999)),
        std::domain_error
    );
}

BOOST_AUTO_TEST_SUITE_END()
/******************************************************************************
*                        END "BASE UNITS" BOOST TEST SUITE
*****************************************************************************/
