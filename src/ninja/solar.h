/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Wrapper class for solpos and sol00
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#ifndef SOLAR_H
#define SOLAR_H

#include <iostream>
	
#include "solpos00.h"
#ifndef Q_MOC_RUN
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#endif

#include "ninja_conv.h"

class Solar
{
public:

	Solar();
//	Solar(int day_in, int month_in, int year_in,
//		int second_in, int minute_in, int hour_in,
//		double latitude_in, double longitude_in,
//		double timeZone_in, double aspect_in, double slope_in);
	Solar(const boost::local_time::local_date_time& time_in,
		double latitude_in, double longitude_in, 
		double aspect_in, double slope_in);
	Solar(Solar &s);
	~Solar();

	static const int noDataValue = -9999;
	
	boost::local_time::local_date_time solarTime;

	inline double get_aspect(){return aspect;}
	inline double get_latitude(){return latitude;}
	inline double get_longitude(){return longitude;}
	inline int get_interval(){return interval;}
	inline double get_slope(){return slope;}

	inline double get_theta(){return theta;}
	inline double get_phi(){return phi;}
	inline double get_solarIntensity(){return solarIntensity;}

	inline int get_errorCode(){return errorCode;}
	
	inline bool set_aspect(double a){aspect = a;return true;}
	inline bool set_latitude(double lat){latitude = lat;return true;}
	inline bool set_longitude(double lon){longitude = lon;return true;}
	inline bool set_interval(int i){interval = i;return true;}
	inline bool set_slope(double s){slope = s;return true;}

//	bool compute_solar(int day_in, int month_in, int year_in,
//		int second_in, int minute_in, int hour_in,
//		double latitude_in, double longitude_in,
//		double timeZone_in, double aspect_in, double slope_in);
	
	bool compute_solar(boost::local_time::local_date_time time_in,
		double latitude_in, double longitude_in, 
		double aspect_in, double slope_in);

	bool print_allSolarPosData();

	//bool initialize_SolarForTesting();
    
	bool set_allSolarPosData();
	bool call_solPos();
//	bool call_solPos(int day_in, int month_in, int year_in,
//		int second_in, int minute_in, int hour_in,
//		double latitude_in, double longitude_in,
//		double timeZone_in, double aspect_in, double slope_in);

	Solar &operator=(Solar &S);

private:
	
	int errorCode;

	double aspect;
	double theta;
	double phi;
	double slope;	//tilt
	double latitude;
	double longitude;
	int interval;
	double solarIntensity;

	
    posdata* solarPosData;
    
    #ifdef STABILITY
    friend class Stability;
    #endif 
};

#endif	//SOLAR_H
