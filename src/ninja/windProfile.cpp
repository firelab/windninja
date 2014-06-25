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

#include "windProfile.h"

windProfile::windProfile()
{
	profile_switch = uniform;
	inputWindSpeed = 0.0;
	inputWindHeight = 0.0;			//AGL!		
	Roughness = 0.0;
	Rough_h = 0.0;
	Rough_d = 0.0;
	AGL = 0.0;						//desired wind height AGL!
	ObukovLength = 0.0;
	ABL_height = 0.0; 
	powerLawPower = 0.143;			//for Askervein study...?
}

windProfile::~windProfile()
{
	
}

double windProfile::getWindSpeed()
{
	//Initialize u0, v0, and w0 at all nodes (could enter coding here to interpolate from data such as MM5)
    if(profile_switch==uniform)         //uniform profile
    {
		if(AGL==0.0)
		{
			velocity = 0.0;
			return velocity;
		}else{
			velocity = inputWindSpeed;
			return velocity;
		}
    }else if(profile_switch==logarithmic)   //log law equation profile
	{
		if(AGL==0.0)
		{
			velocity = 0.0;
			return velocity;
		}else
		{
			inwindheight = (inputWindHeight + Rough_h) - (Rough_d);	//height of input wind (from z=0 of log profile)
            if(AGL < (Rough_d + Roughness))	//if we're below the log profile, return velocity of zero
			{
				velocity = 0.0;
				return velocity;
			}else{
				velocity = inputWindSpeed*((log((AGL-Rough_d)/Roughness))/(log((inwindheight)/Roughness)));
				return velocity;
			}
		}
    }else if(profile_switch==power_law_askervein)   //power law equation profile
    {
		 if(AGL==0.0)
		 {
			velocity = 0.0;
			return velocity;
		 }else
         {
              velocity = inputWindSpeed*std::pow((AGL/inputWindHeight),powerLawPower);
			  return velocity;
         }
	}else if(profile_switch==monin_obukov_similarity)   //Monin-Obukov similarity profile
    {
		 if(AGL==0.0)
		 {
			velocity = 0.0;
			return velocity;
		 }else
		 {		
				inwindheight = (inputWindHeight + Rough_h) - (Rough_d);       //height of input wind (from z=0 of log profile)

                if(inwindheight < Roughness)  //if the input wind is at a height where the log profile isn't defined (can happen on output interpolation), just linearly interpolate
                {
                    velocity = inputWindSpeed * (AGL/(inwindheight + Rough_d));
                    return velocity;
                }else{  //else, just do standard profile stuff
                    if(AGL < (Rough_d + 7.0*Roughness))	//linearly interpolate, as in AERMOD, if below 7*z0
                    {
                        vel7z0 = monin_obukov(7.0*Roughness, inputWindSpeed, inwindheight, Roughness, ObukovLength);	//compute windspeeds at 7*z0 height
                        velocity = vel7z0 * (AGL/(7.0*Roughness + Rough_d));
                        return velocity;
                    }else if(AGL < (Rough_d + ABL_height))	//if below ABL top, monin-obukov similarity (log profile)
                    {
                        velocity = monin_obukov((AGL - Rough_d), inputWindSpeed, inwindheight, Roughness, ObukovLength);
                        return velocity;
                    }else{	//else we're above the ABL...
                        velocity = monin_obukov(ABL_height, inputWindSpeed, inwindheight, Roughness, ObukovLength);
                        return velocity;
                    }
                }
         }
	}else
	    throw std::runtime_error("Could not identify profile switch.\n");
}

double windProfile::monin_obukov(double const& z, double const& U1, double const& z1, double const& z0, double const& L)
{
	double speed;
	
	if(L == 0.0)
		speed = U1*(log(z/z0))/(log(z1/z0));
	else
		speed = U1*(log(z/z0)-stability_function(z/L,L))/(log(z1/z0)-stability_function(z1/L,L));

	return speed;
}

double windProfile::stability_function(double const& z_over_L, double const& L_switch)
{	//function that computes the stability function for the vertical wind profile (similarity theory)
	//z_over_L is the height divided by the Monin-Obukov length
	//L_switch indicates the sign of the Monin-Obukov length (either positive value for stable or negative value for unstable)
	
	double value;

	if(L_switch >= 0.0)	//stable conditions
	{
//		value = -5.0*z_over_L;	//usual computation
		value = -17.0*(1.0 - exp(-0.29*z_over_L));	//improved computation from Van Ulden and Holtslag, 1985

	}else{	//unstable conditions
		double xxx = std::pow((1.0-16.0*z_over_L),0.25);									//usual computation
		value = 2.0*log((1.0+xxx)/2.0) + log((1+xxx*xxx)/2.0) - 2.0*atan(xxx) + pi/2.0;
//		value = std::pow((1.0-16.0*z_over_L),0.25) - 1.0;	//improved computation from Van Ulden and Holtslag, 1985
	}

	return value;
}
