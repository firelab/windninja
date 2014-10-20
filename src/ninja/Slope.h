/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for slope calculations across a spatial grid
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

#ifndef SLOPE_H
#define SLOPE_H

#include "ascii_grid.h"
#include "Elevation.h"

#ifdef _OPENMP
#include <omp.h>
#endif

//#define SLOPE_DEBUG

class Slope : public AsciiGrid<double>
{
public:
	Slope();
	Slope(Elevation const* elev, int number_CPUs);
	Slope(Elevation const* elev, int slope_units, int number_CPUs);
	~Slope();
	bool compute_gridSlope();
	bool compute_gridSlope(Elevation const* elev, int number_threads);
	bool read_slope(std::string filename);
	bool read_slope(std::string filename, int slope_units);
	enum units_enum{degrees, percent};	//Still need to figure out how units should work...
	bool set_units(int u){units = u; return true;}
	bool set_num_threads(int t){number_CPUs = t; return true;}
	double compute_celldzdx(double a, double d, double g, double c, double f, double i, double e);
	double compute_celldzdy(double a, double b, double c, double g, double h, double i, double e);

	Elevation const* elevation;

private:
	bool grid_made;
	int units;
	int number_CPUs;

	
};

#endif	//SLOPE_H
