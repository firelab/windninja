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

#include "solar.h"
Solar::Solar()
: solarTime(boost::local_time::not_a_date_time)
{
	aspect = noDataValue;
	theta = noDataValue;
	phi = noDataValue;
	interval = noDataValue;
	latitude = noDataValue;
	longitude = noDataValue;
	slope = noDataValue;
	solarIntensity = noDataValue;

	solarPosData = new posdata;

	S_init(solarPosData);
}

Solar::Solar(const boost::local_time::local_date_time& time_in,
		double latitude_in, double longitude_in, 
		double aspect_in, double slope_in)
: solarTime(time_in)
{
	aspect = aspect_in;
	theta = noDataValue;
	phi = noDataValue;
	interval = 0;
	latitude = latitude_in;
	longitude = longitude_in;
	slope = slope_in;
	solarIntensity = noDataValue;

	solarPosData = new posdata;

	set_allSolarPosData();

	call_solPos();
}

Solar::Solar(Solar &s)
: solarTime(boost::local_time::not_a_date_time)
{
	aspect = s.aspect;
	theta = s.theta;
	phi = s.phi;
	solarTime = s.solarTime;
	interval = s.interval;
	latitude = s.latitude;
	longitude = s.longitude;
	slope = s.slope;
	solarIntensity = s.solarIntensity;

	solarPosData = new posdata;

	set_allSolarPosData();
	
}


Solar::~Solar()
{
	if(solarPosData)
		delete solarPosData;	
}

//bool Solar::compute_solar(int day_in, int month_in, int year_in,
//	int second_in, int minute_in, int hour_in,
//	double latitude_in, double longitude_in,
//	double timeZone_in, double aspect_in, double slope_in)
//{
//	aspect = aspect_in;
//	theta = noDataValue;
//	phi = noDataValue;
//	solarTime = ;
//	interval = 0;
//	latitude = latitude_in;
//	longitude = longitude_in;
//	solarTime.set_minute(minute_in);
//	solarDate.set_month(month_in);
//	solarTime.set_second(second_in);
//	slope = slope_in;
//	solarDate.set_year(year_in);
//	solarIntensity = noDataValue;
//	solarTime.set_timezone(timeZone_in);
//
//	if(solarPosData)
//		delete solarPosData;
//	solarPosData = new posdata;
//
//	set_allSolarPosData();
//
//	call_solPos();
//
//	return true;
//}
	
bool Solar::compute_solar(boost::local_time::local_date_time time_in,
	double latitude_in, double longitude_in, 
	double aspect_in, double slope_in)
{
	aspect = aspect_in;
	theta = noDataValue;
	solarTime = time_in;
	phi = noDataValue;
	interval = 0;
	latitude = latitude_in;
	longitude = longitude_in;
	slope = slope_in;
	solarIntensity = noDataValue;

	if(solarPosData)
		delete solarPosData;
	solarPosData = new posdata;

	set_allSolarPosData();

	call_solPos();

	return true;
}

bool Solar::set_allSolarPosData()
{
	S_init(solarPosData);
	solarPosData->aspect = aspect;
	solarPosData->azim = theta;
	solarPosData->day = solarTime.local_time().date().day();
	solarPosData->elevref = phi;
	solarPosData->hour = solarTime.local_time().time_of_day().hours();
	solarPosData->interval = interval;
	solarPosData->latitude = latitude;
	solarPosData->longitude = longitude;
	solarPosData->minute = solarTime.local_time().time_of_day().minutes();
	solarPosData->month = solarTime.local_time().date().month();
	solarPosData->second = solarTime.local_time().time_of_day().seconds();
	solarPosData->tilt = slope;
	solarPosData->year = solarTime.local_time().date().year();

	//Compute offset from UTC, including if in daylight savings or not
	boost::posix_time::ptime UTC_time(solarTime.utc_time());
	boost::posix_time::ptime Local_time(solarTime.local_time());
	boost::posix_time::time_duration offset = Local_time - UTC_time;
//	if(offset.is_negative())
//		offset = offset.invert_sign();

	//Get offset from UTC in decimal hours
	solarPosData->timezone = ((double) offset.total_seconds())/3600.0;

	solarPosData->etrtilt = solarIntensity;

	return true;
}

bool Solar::call_solPos()
{
	set_allSolarPosData();

	solarPosData->function = (S_ALL & ~S_DOY);

	errorCode = S_solpos(solarPosData);
	theta = solarPosData->azim;
	phi = solarPosData->elevref;
	solarIntensity = solarPosData->etrtilt;
	
	return true;
}

bool Solar::print_allSolarPosData()
{
	//Compute offset from UTC, including if in daylight savings or not
	boost::posix_time::ptime UTC_time(solarTime.utc_time());
	boost::posix_time::ptime Local_time(solarTime.local_time());
	boost::posix_time::time_duration offset = UTC_time - Local_time;
	if(offset.is_negative())
		offset = offset.invert_sign();

	std::cout << "Solar class data:" << std::endl
		<< "aspect = " << aspect << std::endl
		<< "theta = " << theta << std::endl
		<< "solarTime = " << solarTime.local_time() << std::endl
		<< "phi = " << phi << std::endl
		<< "interval = " << interval << std::endl
		<< "latitude = " << latitude << std::endl
		<< "longitude = " << longitude << std::endl
		<< "slope = " << slope << std::endl
		<< "timeZone = " << offset.hours() << std::endl
		<< std::endl << "solarIntensity = " << solarIntensity;

	std::cout << std::endl << std::endl;

	std::cout << "posdata struct data:" << std::endl
		<< "aspect = " << solarPosData->aspect << std::endl
		<< "azim = " << solarPosData->azim << std::endl
		<< "day = " << solarPosData->day << std::endl
		<< "elevref = " << solarPosData->elevref << std::endl
		<< "hour = " << solarPosData->hour << std::endl
		<< "interval = " << solarPosData->interval << std::endl
		<< "latitude = " << solarPosData->latitude << std::endl
		<< "longitude = " << solarPosData->longitude << std::endl
		<< "minute = " << solarPosData->minute << std::endl
		<< "month = " << solarPosData->month << std::endl
		<< "second = " << solarPosData->second << std::endl
		<< "tilt = " << solarPosData->tilt << std::endl
		<< "timezone = " << solarPosData->timezone << std::endl
		<< "year = " << solarPosData->year << std::endl
		<< std::endl << "etrtilt = " << solarPosData->etrtilt << std::endl;

	return true;
}


Solar &Solar::operator=(Solar &S)
{
	if(&S != this)
	{
		aspect = S.aspect;
		theta = S.theta;
		phi = S.phi;
		solarTime = S.solarTime;
		interval = S.interval;
		latitude = S.latitude;
		longitude = S.longitude;
		slope = S.slope;
		solarIntensity = S.solarIntensity;

          if(solarPosData)
               delete solarPosData;
		solarPosData = new posdata;

		set_allSolarPosData();
	}
	return *this;
}
