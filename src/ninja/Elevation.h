/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing elevation data
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

#ifndef ELEVATION_H
#define ELEVATION_H

#include "ninjaException.h"
#include "ascii_grid.h"
//#include <windows.h>
//#include <process.h>
#include <string>

class Elevation : public AsciiGrid<double>
{
public:
	
	enum eElevDistanceUnits{
		feet,
		meters,
	};
	
	Elevation();
	Elevation(std::string filename);
	Elevation(std::string filename, eElevDistanceUnits elev_units);

	//kss
	Elevation(const Elevation &rhs);
	Elevation &operator=(const Elevation &rhs);

	Elevation(int nC, int nR, double xL, double yL, double cS, double nDV, eElevDistanceUnits units);

	~Elevation();

        void readFromMemory(const double* dem, const int nXSize, const int nYSize,
                            const double* geoRef, std::string prj);
	void read_elevation(std::string filename);
	void read_elevation(std::string filename, eElevDistanceUnits elev_units);
	std::string fileName;
	eElevDistanceUnits elevationUnits;	//these are the vertical units, ALL HORIZONTAL UNITS MUST ALWAYS BE IN METERS, INCLUDING WHEN THEY ARE READ IN!
        GDALDatasetH hDS; //in-memory dataset for DEM for API
	bool grid_made;	
        void setAngleFromNorth(double angle);
        double getAngleFromNorth();

private:
        double angleFromNorth; //N-S grid line angle departe from true north in the DEM projection
};

#endif	//ELEVATION_H
