/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  A class that computes several different types of vertical wind profiles.
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

#ifndef WIND_PROFILE_H
#define WIND_PROFILE_H

#include "ninjaMathUtility.h"

class windProfile
{
	public:

		windProfile();								//Default constructor
		~windProfile();                              // Destructor
		
		//windProfile(windProfile const& m);               // Copy constructor
		//windProfile& operator= (windProfile const& m);   // Assignment operator

		enum eProfile{
			uniform,
			logarithmic,				//log profile for neutral stability
			power_law_askervein,		//power law USED FOR ASKERVEIN HILL
			monin_obukov_similarity		//log profile using Monin-Obukov similarity for non-neutral stability
		};

		eProfile profile_switch;
		double inputWindSpeed;
		double inputWindHeight;			//above vegetation		
		double Roughness;
		double Rough_h;
		double Rough_d;
		double AGL;						//desired wind height "above ground level" (AGL)!
		double ObukovLength;
		double ABL_height;
		double powerLawPower;			//used in power_law_askervein

		double getWindSpeed();		//function returns wind velocity given the inputs (profile_switch, AGL, etc...)
        double monin_obukov(double z, double const& U1, double const& z1, double const& z0, double const& L);
		double stability_function(double const& z_over_L, double const& L_switch);
		
		

	private:
		double inwindheight;		//height of input wind (from z=0 of log profile)
		double velocity;			//velocity computed at height AGL
		double vel7z0;				//used in monin_obukov_similarity
};

#endif /* WIND_PROFILE_H */

