/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class that calculates shade for a spatial grid
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

#ifndef SHADE_H
#define SHADE_H

#include "ascii_grid.h"
#include "Elevation.h"
#include "ninjaMathUtility.h"
//#include <conio.h>  // This can be taken out!! only here for debugging...
#include <stdio.h>  // This can be taken out!! only here for debugging...

#ifdef _OPENMP
#include <omp.h>
#endif

//#define SHADE_DEBUG

//Computes shadows, given terrain and sun angles ( shaded => true;  unshaded => false )
//                                               ( shaded =>   1 ;  unshaded =>   0   )
class Shade : public AsciiGrid<short>
{
public:
	Shade();
	Shade(Elevation const* elev, double Theta, double Phi, int number_CPUs);
	~Shade();
	bool read_shade(std::string filename);
	inline bool set_theta(double sunAngle){theta = sunAngle; return true;}
	inline bool set_phi(double sunElev){phi = sunElev; return true;}
	bool set_num_threads(int t){number_CPUs = t; return true;}

	bool compute_gridShade(Elevation const* elev, double Theta, double Phi, int number_threads);

	static const short shaded = 1;
	static const short unshaded = 0;

private:
	bool grid_made;
	int number_CPUs;
	Elevation const* elevation;
	AsciiGrid<double> *flagMap;
	AsciiGrid<double> *elevation_norm;
	double smalll;
	//int *X;			//a pointer, holds the current X position as we travel along the heightmap
	//int *Y;			//a pointer, holds the current Y position as we travel along the heightmap
	//int iX;			//represents the outer loop variable of our loops
	//int iY;			//represents the inner loop variable of our loops
	int diriX;		//if less than 0 then we travel to the left after we	process the current point, else travel right
	int diriY;		//if less than 0 then we travel up after we process the	current point, else travel down
	double x_light;
	double y_light;
	double z_light;
	double lightDirXMagnitude;
	double lightDirYMagnitude;
    int sideClosestToSun;   // 0=>west  1=>east  2=>south  3=>north
	int sizeiX;		//size of array for outer loop
	int sizeiY;		//size of array for inner loop
	double phi;	//sun elevation	
	double theta;	//sun angle
	long inner_loop_num;
	long outer_loop_num;

	bool compute_gridShade();
	bool track_along_ray(double px, double py, int *X, int *Y);
	
};

#endif	//SHADE_H
