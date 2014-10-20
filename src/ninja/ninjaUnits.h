/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  A collection of units conversion functions and enums
* Author:   Jason Forthofer <jforthofer@gmail.com>
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

#ifndef	NINJA_UNITS_H
#define NINJA_UNITS_H

#include "ninjaException.h"
#include "ascii_grid.h"
#include "string.h"


// Length -------------------------------------------------------------------------
class lengthUnits{
public:
    enum eLengthUnits{
        feet,
        meters,				//base windninja unit
        miles,
        kilometers,
        feetTimesTen,
        metersTimesTen
    };
    static void toBaseUnits(double& value, eLengthUnits units);
    static void fromBaseUnits(double& value, eLengthUnits units);
    static void toBaseUnits(AsciiGrid<double>& grid, eLengthUnits units);
    static void fromBaseUnits(AsciiGrid<double>& grid, eLengthUnits units);

    static void toBaseUnits(double& value, std::string units);
    static void fromBaseUnits(double& value, std::string units);
    static void toBaseUnits(AsciiGrid<double>& grid, std::string units);

    static std::string getString(eLengthUnits unit);
    static eLengthUnits getUnit(std::string unit);
    static void fromBaseUnits(AsciiGrid<double>& grid, std::string units);

private:
};


// Velocity ------------------------------------------------------------------------
class velocityUnits{
public:
    enum eVelocityUnits{
        metersPerSecond,	//base windninja unit
        milesPerHour,
        kilometersPerHour
    };
    static void toBaseUnits(double& value, eVelocityUnits units);
    static void fromBaseUnits(double& value, eVelocityUnits units);
    static void toBaseUnits(AsciiGrid<double>& grid, eVelocityUnits units);
    static void fromBaseUnits(AsciiGrid<double>& grid, eVelocityUnits units);

    static void toBaseUnits(double& value, std::string units);
    static void fromBaseUnits(double& value, std::string units);
    static void toBaseUnits(AsciiGrid<double>& grid, std::string units);
    static void fromBaseUnits(AsciiGrid<double>& grid, std::string units);

    static std::string getString(eVelocityUnits unit);
    static eVelocityUnits getUnit(std::string unit);
private:
};


// Temperature ---------------------------------------------------------------------
class temperatureUnits{
public:
    enum eTempUnits{
        K,					//base windninja unit
        C,
        R,
        F
    };
    static void toBaseUnits(double& value, eTempUnits units);
    static void fromBaseUnits(double& value, eTempUnits units);
    static void toBaseUnits(AsciiGrid<double>& grid, eTempUnits units);
    static void fromBaseUnits(AsciiGrid<double>& grid, eTempUnits units);

    static void toBaseUnits(double& value, std::string units);
    static void fromBaseUnits(double& value, std::string units);
    static void toBaseUnits(AsciiGrid<double>& grid, std::string units);
    static void fromBaseUnits(AsciiGrid<double>& grid, std::string units);

    static std::string getString(eTempUnits unit);
    static eTempUnits getUnit(std::string unit);
private:
};

// Cloud and Canopy Cover ---------------------------------------------------------------------
class coverUnits{
public:
    enum eCoverUnits{
        fraction,			//base windninja unit
        percent,
        canopyCategories	// categories ->    1 = 1-20%
        //					2 = 21-50%
        //					3 = 51-80%
        //					4 = 81-100%
        //					0 or 99 = 0%
    };
    static void toBaseUnits(double& value, eCoverUnits units);
    static void fromBaseUnits(double& value, eCoverUnits units);
    static void toBaseUnits(AsciiGrid<double>& grid, eCoverUnits units);
    static void fromBaseUnits(AsciiGrid<double>& grid, eCoverUnits units);

    static void toBaseUnits(double& value, std::string units);
    static void fromBaseUnits(double& value, std::string units);
    static void toBaseUnits(AsciiGrid<double>& grid, std::string units);
    static void fromBaseUnits(AsciiGrid<double>& grid, std::string units);

    static std::string getString(eCoverUnits unit);
    static eCoverUnits getUnit(std::string unit);
private:
};

#endif /* NINJA_UNITS_H */
