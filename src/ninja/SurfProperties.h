/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing gridded surface parameters (roughness, etc)
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

#ifndef SURF_PROPERTIES_H
#define SURF_PROPERTIES_H

//#define SURF_PROPERTIES_DEBUG

#include "Elevation.h"
#include "Aspect.h"
#include "Slope.h"
#include "Shade.h"
#include "ascii_grid.h"
#include "ninjaUnits.h"

class surfProperties
{	

public:

	surfProperties();

	  //kss
	  surfProperties(const surfProperties &rhs);
	  surfProperties &operator=(const surfProperties &rhs);

	~surfProperties();
	void deallocate();

	AsciiGrid<double> Roughness;
	lengthUnits::eLengthUnits RoughnessUnits;
	AsciiGrid<double> Rough_d;
	lengthUnits::eLengthUnits Rough_dUnits;
	AsciiGrid<double> Rough_h;
	lengthUnits::eLengthUnits Rough_hUnits;
	AsciiGrid<double> Albedo;
	AsciiGrid<double> Bowen;
	AsciiGrid<double> Cg;
	AsciiGrid<double> Anthropogenic;
	double Windspeed;				//needed in surface friction velocity and Monin-Obukov length computation in diurnal stuff
	AsciiGrid<double> windSpeedGrid;	//used for surface friction velocity and Monin-Obukov length computation when surface field initialization method is used (NDFD, etc.)
	double Z;						//height of windspeed (above top of vegetation)
	bool windGridExists;			//bool to determine if a wind grid or non-grid wind should be used
	

	bool set_windspeed(double windspeed);	//always in m/s
	bool set_windspeed(AsciiGrid<double> &speedGrid);	//always in m/s
	bool resample_in_place(double resampleCellSize, AsciiGrid<double>::interpTypeEnum interpType);
    bool BufferGridInPlace( int nAddCols=1, int nAddRows=1 );

private:
	
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif	//SURF_PROPERTIES_H
