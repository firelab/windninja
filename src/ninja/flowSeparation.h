/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class that calculates flow separation for a spatial grid
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

#ifndef FLOWSEPARATION_H
#define FLOWSEPARATION_H

#include "ascii_grid.h"
#include "Elevation.h"
#include "ninjaMathUtility.h"

#ifdef _OPENMP
#include <omp.h>
#endif

//Computes flow separation in the lee of terrain, given terrain, input wind direction, and flow separation angle ( separated => true;  attached => false )
//                                               ( separated =>   1 ;  attached =>   0   )
class flowSeparation : public AsciiGrid<short>
{
public:
    flowSeparation();
    flowSeparation(Elevation const* elev, double Theta, double Phi, int number_CPUs);
    ~flowSeparation();
    bool read_separation(std::string filename);
    inline bool set_theta(double windDirection){theta = windDirection; return true;}
    inline bool set_phi(double separationAngle){phi = separationAngle; return true;}
    bool set_num_threads(int t){number_CPUs = t; return true;}

    bool compute_gridSeparation(Elevation const* elev, double Theta, double Phi, int number_threads);

    static const short separated = 1;
    static const short attached = 0;

private:
    bool grid_made;
    int number_CPUs;
    Elevation const* elevation;
    AsciiGrid<double> *flagMap;
    AsciiGrid<double> *elevation_norm;
    double smalll;
    int diriX; //if less than 0 then we travel to the left after we process the current point, else travel right
    int diriY; //if less than 0 then we travel up after we process the current point, else travel down
    double x_wind;
    double y_wind;
    double z_wind;
    double windDirXMagnitude;
    double windDirYMagnitude;
    int sideClosestToInlet;   // 0=>west  1=>east  2=>south  3=>north
    int sizeiX;		//size of array for outer loop
    int sizeiY;		//size of array for inner loop
    double phi;	//separation elevation	
    double theta;	//wind direction
    long inner_loop_num;
    long outer_loop_num;

    bool compute_gridSeparation();
    bool track_along_ray(double px, double py, int *X, int *Y);
};

#endif	//FLOWSEPARATION_H
