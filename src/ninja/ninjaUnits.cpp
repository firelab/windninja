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

#include "ninjaUnits.h"

void lengthUnits::toBaseUnits(double& value, lengthUnits::eLengthUnits units)
{
	switch(units)
	{
		case feet:
			value = value / 3.28084;
			break;
		case meters:
			break;
		case miles:
			value = value * 1609.344;
			break;
		case kilometers:
			value = value * 1000;
			break;
		case feetTimesTen:
			value = value  / 32.8084;
			break;
		case metersTimesTen:
			value = value / 10.0;
			break;
		default:
			throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
	}

	/*
    if(units == feet)
        value = value / 3.28084;
    else if(units == meters)
        //value = value;
    	break;
    else if(units == miles)
        value = value * 1609.344;
    else if(units == kilometers)
        value = value * 1000;
    else if(units == feetTimesTen)
        value = value  / 32.8084;
    else if(units == metersTimesTen)
        value = value / 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
        */
}

void lengthUnits::fromBaseUnits(double& value, lengthUnits::eLengthUnits units)
{
	switch(units)
		{
			case feet:
				value = value * 3.28084;
				break;
			case meters:
				break;
			case miles:
				value = value / 1609.344;
				break;
			case kilometers:
				value = value / 1000;
				break;
			case feetTimesTen:
				value = value  * 32.8084;
				break;
			case metersTimesTen:
				value = value * 10.0;
				break;
			default:
				throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
		}

	/*
    if(units == feet)
        value = value * 3.28084;
    else if(units == meters)
        //value = value;
    	break;
    else if(units == miles)
        value = value / 1609.344;
    else if(units == kilometers)
        value = value / 1000;
    else if(units == feetTimesTen)
        value = value  * 32.8084;
    else if(units == metersTimesTen)
        value = value * 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
     */
}

void lengthUnits::toBaseUnits(AsciiGrid<double>& grid, lengthUnits::eLengthUnits units)
{
	switch(units)
			{
				case feet:
					grid = grid / 3.28084;
					break;
				case meters:
					break;
				case miles:
					grid = grid * 1609.344;
					break;
				case kilometers:
					grid = grid * 1000;
					break;
				case feetTimesTen:
					grid = grid  / 32.8084;
					break;
				case metersTimesTen:
					grid = grid / 10.0;
					break;
				default:
					throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
			}
	/*
	if(units == feet)
        grid = grid / 3.28084;
    else if(units == meters)
    	break;
    else if(units == miles)
        grid = grid * 1609.344;
    else if(units == kilometers)
        grid = grid * 1000;
    else if(units == feetTimesTen)
        grid = grid  / 32.8084;
    else if(units == metersTimesTen)
        grid = grid / 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
    */
}

void lengthUnits::fromBaseUnits(AsciiGrid<double>& grid, lengthUnits::eLengthUnits units)
{
	switch(units)
				{
					case feet:
						grid = grid * 3.28084;
						break;
					case meters:
						break;
					case miles:
						grid = grid / 1609.344;
						break;
					case kilometers:
						grid = grid / 1000;
						break;
					case feetTimesTen:
						grid = grid  * 32.8084;
						break;
					case metersTimesTen:
						grid = grid * 10.0;
						break;
					default:
						throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
				}

	/*
    if(units == feet)
        grid = grid * 3.28084;
    else if(units == meters)
        //grid = grid;
    	break;
    else if(units == miles)
        grid = grid / 1609.344;
    else if(units == kilometers)
        grid = grid / 1000;
    else if(units == feetTimesTen)
        grid = grid  * 32.8084;
    else if(units == metersTimesTen)
        grid = grid * 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
	*/
}

void lengthUnits::toBaseUnits(double& value, std::string units)
{
    if(units == "ft")
        value = value / 3.28084;
    else if(units == "m")
    	return;
    else if(units == "mi")
        value = value * 1609.344;
    else if(units == "km")
        value = value * 1000;
    else if(units == "ftx10")
        value = value  / 32.8084;
    else if(units == "mx10")
        value = value / 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");

}

void lengthUnits::fromBaseUnits(double& value, std::string units)
{
    if(units == "ft")
        value = value * 3.28084;
    else if(units == "m")
        //value = value;
    	return;
    else if(units == "mi")
        value = value / 1609.344;
    else if(units == "km")
        value = value / 1000;
    else if(units == "ftx10")
        value = value  * 32.8084;
    else if(units == "mx10")
        value = value * 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
}

void lengthUnits::toBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "ft")
        grid = grid / 3.28084;
    else if(units == "m")
        //grid = grid;
    	return;
    else if(units == "mi")
        grid = grid * 1609.344;
    else if(units == "km")
        grid = grid * 1000;
    else if(units == "ftx10")
        grid = grid  / 32.8084;
    else if(units == "mx10")
        grid = grid / 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
}

void lengthUnits::fromBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "ft")
        grid = grid * 3.28084;
    else if(units == "m")
        //grid = grid;
    	return;
    else if(units == "mi")
        grid = grid / 1609.344;
    else if(units == "km")
        grid = grid / 1000;
    else if(units == "ftx10")
        grid = grid  * 32.8084;
    else if(units == "mx10")
        grid = grid * 10.0;
    else
        throw std::domain_error("Length units problem in lengthUnits::toBaseUnits().");
}

std::string lengthUnits::getString(eLengthUnits unit)
{
    std::string s;
    if(unit == feet)
        s = "ft";
    else if(unit ==	meters)
        s = "m";
    else if(unit == miles)
        s = "mi";
    else if(unit == kilometers)
        s = "km";
    else if(unit == feetTimesTen)
        s = "ftx10";
    else if(unit ==	metersTimesTen)
        s = "mx10";
    else
        s = "";

    return s;
}

lengthUnits::eLengthUnits lengthUnits::getUnit(std::string unit)
{
    if(unit == "ft")
        return feet;
    else if(unit == "m")
        return  meters;
    else if(unit == "mi")
        return  miles;
    else if(unit == "km")
        return  kilometers;
    else if(unit == "ftx10")
        return  feetTimesTen;
    else if(unit == "mx10")
        return  metersTimesTen;
    else
        throw std::logic_error("Unit cannot be identified.");
}

void velocityUnits::toBaseUnits(double& value, velocityUnits::eVelocityUnits units)
{
    if(units == metersPerSecond)
        //value = value;
    	return;
    else if(units == milesPerHour)
        value = value * 0.44704;
    else if (units == kilometersPerHour)
        value = value * 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::toBaseUnits().");
}

void velocityUnits::fromBaseUnits(double& value, velocityUnits::eVelocityUnits units)
{
    if(units == metersPerSecond)
        //value = value;
    	return;
    else if(units == milesPerHour)
        value = value / 0.44704;
    else if (units == kilometersPerHour)
        value = value / 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::fromBaseUnits().");
}

void velocityUnits::toBaseUnits(AsciiGrid<double>& grid, velocityUnits::eVelocityUnits units)
{
    if(units == metersPerSecond)
        //grid = grid;
    	return;
    else if(units == milesPerHour)
        grid = grid * 0.44704;
    else if (units == kilometersPerHour)
        grid = grid * 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::toBaseUnits().");
}

void velocityUnits::fromBaseUnits(AsciiGrid<double>& grid, velocityUnits::eVelocityUnits units)
{
    if(units == metersPerSecond)
        //grid = grid;
    	return;
    else if(units == milesPerHour)
        grid = grid / 0.44704;
    else if (units == kilometersPerHour)
        grid = grid / 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::fromBaseUnits().");
}

void velocityUnits::toBaseUnits(double& value, std::string units)
{
    if(units == "mps")
        //value = value;
    	return;
    else if(units == "mph")
        value = value * 0.44704;
    else if (units == "kph")
        value = value * 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::toBaseUnits().");
}

void velocityUnits::fromBaseUnits(double& value, std::string units)
{
    if(units == "mps")
        //value = value;
    	return;
    else if(units == "mph")
        value = value / 0.44704;
    else if (units == "kph")
        value = value / 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::fromBaseUnits().");
}

void velocityUnits::toBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "mps")
        //grid = grid;
    	return;
    else if(units == "mph")
        grid = grid * 0.44704;
    else if (units == "kph")
        grid = grid * 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::toBaseUnits().");
}

void velocityUnits::fromBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "mps")
        //grid = grid;
    	return;
    else if(units == "mph")
        grid = grid / 0.44704;
    else if (units == "kph")
        grid = grid / 0.27778;
    else
        throw std::domain_error("Velocity units problem in velocityUnits::fromBaseUnits().");
}

std::string velocityUnits::getString(eVelocityUnits unit)
{
    std::string s;
    if(unit == metersPerSecond)
        s = "mps";
    else if(unit ==	milesPerHour)
        s = "mph";
    else if(unit == kilometersPerHour)
        s = "kph";
    else
        s = "";

    return s;
}

velocityUnits::eVelocityUnits velocityUnits::getUnit(std::string unit)
{
    if(unit == "mps")
        return  metersPerSecond;
    else if(unit == "mph")
        return  milesPerHour;
    else if(unit == "kph")
        return  kilometersPerHour;
    else
        throw std::logic_error("Cannot identify unit.");
}

void temperatureUnits::toBaseUnits(double& value, temperatureUnits::eTempUnits units)
{
    if(units == K)
        //value = value;
    	return;
    else if(units == C)
        value = value + 273.15;
    else if(units == R)
        value = value *(5.0/9.0);
    else if(units == F)
        value = (value+459.67)*(5.0/9.0);
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

void temperatureUnits::fromBaseUnits(double& value, temperatureUnits::eTempUnits units)
{
    if(units == K)
        //value = value;
    	return;
    else if(units == C)
        value = value - 273.15;
    else if(units == R)
        value = value *(9.0/5.0);
    else if(units == F)
        value = (value*(9.0/5.0))-459.67;
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

void temperatureUnits::toBaseUnits(AsciiGrid<double>& grid, temperatureUnits::eTempUnits units)
{
    if(units == K)
        //grid = grid;
    	return;
    else if(units == C)
        grid = grid + 273.15;
    else if(units == R)
        grid = grid * (5.0/9.0);
    else if(units == F)
        grid =  (grid+459.67)*(5.0/9.0);
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

void temperatureUnits::fromBaseUnits(AsciiGrid<double>& grid, temperatureUnits::eTempUnits units)
{
    if(units == K)
        //grid = grid;
    	return;
    else if(units == C)
        grid = grid - 273.15;
    else if(units == R)
        grid = grid *(9.0/5.0);
    else if(units == F)
        grid = (grid*(9.0/5.0))-459.67;
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

void temperatureUnits::fromBaseUnits(double& value, std::string units)
{
    if(units == "K")
        //value = value;
    	return;
    else if(units == "C")
        value = value - 273.15;
    else if(units == "R")
        value = value *(9.0/5.0);
    else if(units == "F")
        value = (value*(9.0/5.0))-459.67;
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

void temperatureUnits::toBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "K")
        //grid = grid;
    	return;
    else if(units == "C")
        grid = grid + 273.15;
    else if(units == "R")
        grid = grid * (5.0/9.0);
    else if(units == "F")
        grid =  (grid+459.67)*(5.0/9.0);
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

void temperatureUnits::fromBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "K")
        //grid = grid;
    	return;
    else if(units == "C")
        grid = grid - 273.15;
    else if(units == "R")
        grid = grid *(9.0/5.0);
    else if(units == "F")
        grid = (grid*(9.0/5.0))-459.67;
    else
        throw std::domain_error("Temperature units problem in temperatureUnits::toBaseUnits().");
}

std::string temperatureUnits::getString(eTempUnits unit)
{
    std::string s;
    if(unit == K)
        s = "K";
    else if(unit ==	C)
        s = "C";
    else if(unit == R)
        s = "R";
    else if(unit == F)
        s = "F";
    else
        s = "";

    return s;
}

temperatureUnits::eTempUnits temperatureUnits::getUnit(std::string unit)
{
    if(unit == "K")
        return  K;
    else if(unit == "C")
        return  C;
    else if(unit == "R")
        return  R;
    else if(unit == "F")
        return  F;
    else
        throw std::logic_error("Cannot identify unit.");
}

void coverUnits::toBaseUnits(double& value, coverUnits::eCoverUnits units)
{
    if(units == fraction)
        //value = value;
    	return;
    else if(units == percent)
        value = value/100.0;
    else if(units == canopyCategories)
    {
        if(value == 1.0)
            value = 0.1;
        else if(value == 2.0)
            value = 0.35;
        else if(value == 3.0)
            value = 0.65;
        else if(value == 4.0)
            value = 0.90;
        else if(value == 0.0)
            value = 0.0;
        else if(value == 99.0)
            value = 0.0;
        else
            throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
}

void coverUnits::fromBaseUnits(double& value, coverUnits::eCoverUnits units)
{
    if(units == fraction)
        //value = value;
    	return;
    else if(units == percent)
        value = value*100.0;
    else if(units == canopyCategories)
    {
        if(value <= 0.0)
            value = 0.0;
        else if(value <= 0.20)
            value = 1.0;
        else if(value <= 0.50)
            value = 2.0;
        else if(value <= 0.80)
            value = 3.0;
        else if(value <= 1.0)
            value = 4.0;
        else
            throw std::domain_error("Cloud cover units in coverUnits::toBaseUnits() cannot be greater than 100 percent.");
    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::fromBaseUnits().");
}

void coverUnits::toBaseUnits(AsciiGrid<double>& grid, coverUnits::eCoverUnits units)
{
    if(units == fraction)
        //grid = grid;
    	return;
    else if(units == percent)
        grid = grid / 100.0;
    else if(units == canopyCategories)
    {
        for(int i=0; i<grid.get_nRows(); i++)
        {
            for(int j=0; j<grid.get_nCols(); j++)
            {
                if(grid(i,j) == 1.0)
                    grid(i,j) = 0.1;
                else if(grid(i,j) == 2.0)
                    grid(i,j) = 0.35;
                else if(grid(i,j) == 3.0)
                    grid(i,j) = 0.65;
                else if(grid(i,j) == 4.0)
                    grid(i,j) = 0.90;
                else if(grid(i,j) == 0.0)
                    grid(i,j) = 0.0;
                else if(grid(i,j) == 99.0)
                    grid(i,j) = 0.0;
                else
                    throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
            }
        }


    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
}

void coverUnits::fromBaseUnits(AsciiGrid<double>& grid, coverUnits::eCoverUnits units)
{
    if(units == fraction)
        //grid = grid;
    	return;
    else if(units == percent)
        grid = grid * 100.0;
    else if(units == canopyCategories)
    {

        for(int i=0; i<grid.get_nRows(); i++)
        {
            for(int j=0; j<grid.get_nCols(); j++)
            {
                if(grid(i,j) <= 0.0)
                    grid(i,j) = 0.0;
                else if(grid(i,j) <= 0.20)
                    grid(i,j) = 1.0;
                else if(grid(i,j) <= 0.50)
                    grid(i,j) = 2.0;
                else if(grid(i,j) <= 0.80)
                    grid(i,j) = 3.0;
                else if(grid(i,j) <= 1.0)
                    grid(i,j) = 4.0;
                else
                    throw std::domain_error("Cloud cover units in coverUnits::toBaseUnits() cannot be greater than 100 percent.");
            }
        }
    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::fromBaseUnits().");
}

void coverUnits::toBaseUnits(double& value, std::string units)
{
    if(units == "fraction")
        //value = value;
    	return;
    else if(units == "percent")
        value = value/100.0;
    else if(units == "canopy_categories")
    {
        if(value == 1.0)
            value = 0.1;
        else if(value == 2.0)
            value = 0.35;
        else if(value == 3.0)
            value = 0.65;
        else if(value == 4.0)
            value = 0.90;
        else if(value == 0.0)
            value = 0.0;
        else if(value == 99.0)
            value = 0.0;
        else
            throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
}

void coverUnits::fromBaseUnits(double& value, std::string units)
{
    if(units == "fraction")
        //value = value;
    	return;
    else if(units == "percent")
        value = value*100.0;
    else if(units == "canopy_categories")
    {
        if(value <= 0.0)
            value = 0.0;
        else if(value <= 0.20)
            value = 1.0;
        else if(value <= 0.50)
            value = 2.0;
        else if(value <= 0.80)
            value = 3.0;
        else if(value <= 1.0)
            value = 4.0;
        else
            throw std::domain_error("Cloud cover units in coverUnits::toBaseUnits() cannot be greater than 100 percent.");
    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::fromBaseUnits().");
}

void coverUnits::toBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "fraction")
        //grid = grid;
    	return;
    else if(units == "percent")
        grid = grid / 100.0;
    else if(units == "canopy_categories")
    {
        for(int i=0; i<grid.get_nRows(); i++)
        {
            for(int j=0; j<grid.get_nCols(); j++)
            {
                if(grid(i,j) == 1.0)
                    grid(i,j) = 0.1;
                else if(grid(i,j) == 2.0)
                    grid(i,j) = 0.35;
                else if(grid(i,j) == 3.0)
                    grid(i,j) = 0.65;
                else if(grid(i,j) == 4.0)
                    grid(i,j) = 0.90;
                else if(grid(i,j) == 0.0)
                    grid(i,j) = 0.0;
                else if(grid(i,j) == 99.0)
                    grid(i,j) = 0.0;
                else
                    throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
            }
        }


    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::toBaseUnits().");
}

void coverUnits::fromBaseUnits(AsciiGrid<double>& grid, std::string units)
{
    if(units == "fraction")
        //grid = grid;
    	return;
    else if(units == "percent")
        grid = grid * 100.0;
    else if(units == "canopy_categories")
    {

        for(int i=0; i<grid.get_nRows(); i++)
        {
            for(int j=0; j<grid.get_nCols(); j++)
            {
                if(grid(i,j) <= 0.0)
                    grid(i,j) = 0.0;
                else if(grid(i,j) <= 0.20)
                    grid(i,j) = 1.0;
                else if(grid(i,j) <= 0.50)
                    grid(i,j) = 2.0;
                else if(grid(i,j) <= 0.80)
                    grid(i,j) = 3.0;
                else if(grid(i,j) <= 1.0)
                    grid(i,j) = 4.0;
                else
                    throw std::domain_error("Cloud cover units in coverUnits::toBaseUnits() cannot be greater than 100 percent.");
            }
        }
    }else
        throw std::domain_error("Cloud cover units problem in coverUnits::fromBaseUnits().");
}

std::string coverUnits::getString(eCoverUnits unit)
{
    std::string s;
    if(unit == fraction)
        s = "fraction";
    else if(unit ==	percent)
        s = "percent";
    else if(unit == canopyCategories)
        s = "canopy_category";
    else
        s = "";

    return s;
}

coverUnits::eCoverUnits coverUnits::getUnit(std::string unit)
{
    if(unit == "fraction")
        return  fraction;
    else if(unit == "percent")
        return  percent;
    else if(unit == "canopy_category")
        return  canopyCategories;
    else
        throw std::logic_error("Cannot identify unit.");
}
