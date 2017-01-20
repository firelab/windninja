/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Calculates diurnal wind in a cell of a spatial grid
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

#ifndef CELL_DIURNAL_H
#define CELL_DIURNAL_H

//#define CELL_DIURNAL_DEBUG

#include <iostream>
#include <iomanip>
#include <fstream>
	
#include <string>
#include "Elevation.h"
#include "Aspect.h"
#include "Slope.h"
#include "Shade.h"
#include "air.h"
#include "solar.h"
#include "SurfProperties.h"
#include "constants.h"

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Class that stores and computes the diurnal component of wind flow for
one cell.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
class cellDiurnal
{	

    public:
        cellDiurnal(Elevation const* incomingDem, Shade const* shd, 
                Solar *solarInput, double const downDragCoeff,
                double const downEntrainmentCoeff, double const upDragCoeff, 
                double const upEntrainmentCoeff);
        cellDiurnal();
        ~cellDiurnal();

        cellDiurnal(cellDiurnal &c);

        Elevation const* dem;
        Shade const* shade;
        
        void create(Elevation const* incomingDem, Shade const* shd, Solar *solarInput);

        void initialize(double& Xord, double& Yord, const double& asp,
        const double& slp, const double& cloudCover,
        const double& airTemperature, const double& WindSpeed,
        const double& Z, const double& Albedo, const double& Bowen,
        const double& Cg, const double& Anthropogenic, const double& Roughness,
        const double& Rough_h, const double& Rough_d);

        //Function computes diurnal (u,v,w) wind components for cell (i,j)
        //Note: other variables such as vegetation, cloud cover, etc. are specified at
        //construction of cellDiurnal class or changed using set functions
        void compute_cell_diurnal_wind(int I, int J, double *U, double *V,
                                     double *W, double *height, double *L,
                                     double *U_star, double *BL_height);	

        //Function computes the Obukov length, friction velocity, and boundary layer height.
        void compute_cell_diurnal_parameters(int I, int J, double *L, double *U_star, double *BL_height);
        void compute_solarIntensity();
        void compute_Qsw();

        int i, j;  //The i,j of the cell we're computing for
        double Qsw; //Shortwave radiation incident at cell surface (W/m^2) (corrected for cloud cover, water vapor, dust)
        double CloudCover;  //Cloud cover NOTE: range 0-1, NOT 0-100
        double aspect;
        double slope;

    private:
	void compute_Qh();
	void compute_Bl_height();
	void compute_cellHillDist();
	void compute_S();
	void compute_UVW();

        //stability function used in surface layer similarity profile computation
	double stability_function(double z_over_L, double L_switch);
	
	double u, v, w;	//The u,v,w components of the diurnal wind
        //Monin-Obukov length, friction velocity, and atmospheric boundary layer height
	double l, u_star, u_star_old, bl_height;

	double xord;  //coordinates of the location
	double yord;
	double airTemp;	//The air temperature
	
	double z;  //height of incoming windspeed
	double albedo;
	double bowen;
	double cg;
	double anthropogenic;
	double roughness;
	double rough_h;
	double rough_d;
		
	double Qh;  //Sensible heat flux
	bool diurnal_wind;  //Flag that tells us to compute diurnal wind (true) or not (false) based on surface heat flux
	bool up_down;  //Flag that tells us if flow is upslope (0) or downslope (1)
	double hillValleyDist;	//hilltop (downslope flows) or valley bottom (upslope flows) distance (meters)
	double flow_thickness_ratio;
	double elev_change; //change in elevation from cell to hilltop/valley bottom (always positive, ie. absolute value)
	double sinAlphaLocal; //local sine alpha computed in hillDist
	double S;  //Diurnal windspeed (m/s)

        //Flag used to switch computation of hillValleyDist to include distance to shaded/unshaded cell
        //false => don't include shaded/unshaded cells    true(default) => include shaded/unshaded cells
        //NOTE: not sure if this should be true or not, true is only based on intuition at this point. 
        //Other models (CALMET) don't do this
	bool cellDist_shadeFlag;

        double xComponent, yComponent;
	double X, Y, xStart, yStart;  //keeps track of current x and y coordinate along tracking path
	double stepMultiplier;  //adjusts how far each step upslope or downslope is along tracking path
        double g;  //acceleration of gravity
	double Cd_downslope; //surface drag coefficient for downslope flow
	double entrainment_coeff_downslope; //entrainment coefficent for downslope flow
	double Cd_upslope; //surface drag coefficient for upslope flow
	double entrainment_coeff_upslope; //entrainment coefficent for upslope flow
        double theta, phi;
        double sinAlpha, z_over_x, Le;
        double epsilon;
	double c1;
	double c2;
	double c3;
        double sinPsi;
	double gamma;  //constant in Qh computation for stable ABL
	double inputWindspeed; //input windspeed (used for computing Qh)
	
	double v_star;  //intermediate variable in Qh computation for stable ABL
	double d3;  //intermediate variable in Qh computation for stable ABL
	double theta_star;
	double zm;  //used in Qh computation for stable ABL

	double stop_crit;  //stopping criteria for iterative solution of friction velocity
                           //and Monin-Obukov length for unstable case

	double value;  //intermediate variable used in stability_function
	double xxx;  //another intermediate variable used in stability_function
	
	double a1;
	double a2;
	double b1;
	double b2;

	double Qstar;  //Intermediate variable

	double elevOld, elevNew, dist;
	AsciiGrid<double>::interpTypeEnum interp_order;
	Air air;  //class holding air properties as function of temperature
	Solar diurnalSolar;  //class used to compute solar intensity

#ifdef STABILITY
    friend class Stability;
#endif
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif	//CELL_DIURNAL_H
