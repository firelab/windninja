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
    latitude = noDataValue;
    longitude = noDataValue;
    interval = noDataValue;
    aspect = noDataValue;
    slope = noDataValue;
    theta = noDataValue;
    phi = noDataValue;
    solarIntensity = noDataValue;
    year = -1;
    month = -1;
    day = -1;
    hour = -1;
    minute = -1;
    second = -1;
    offset = -1.0;

	solarPosData = new posdata;

	S_init(solarPosData);
}

Solar::Solar(const boost::local_time::local_date_time& time_in,
		double latitude_in, double longitude_in, 
		double aspect_in, double slope_in, double angleFromNorth)
: solarTime(time_in)
{
    latitude = latitude_in;
    longitude = longitude_in;
    interval = 0;
    // the raw input aspect value is expected to be in projected coordinates, except for the flat terrain case (slope and aspect have a value of 0.0),
    // in which case the raw input value is still treated as projected coordinates even though it is technically a geographic value,
    // the code still works despite this value difference because the aspect value is ignored when slope has a value of 0.0
    aspect = aspect_in;
    aspect = wrap0to360( aspect + angleFromNorth ); //convert FROM projected coordinates TO geographic
    slope = slope_in;
    theta = noDataValue;
    phi = noDataValue;
    solarIntensity = noDataValue;

    year = solarTime.local_time().date().year();
    month = solarTime.local_time().date().month();
    day = solarTime.local_time().date().day();
    hour = solarTime.local_time().time_of_day().hours();
    minute = solarTime.local_time().time_of_day().minutes();
    second = solarTime.local_time().time_of_day().seconds();

    //Compute offset from UTC, including if in daylight savings or not
    boost::posix_time::ptime UTC_time(solarTime.utc_time());
    boost::posix_time::ptime Local_time(solarTime.local_time());

    //Get offset from UTC in decimal hours
    offset = ((double) (Local_time - UTC_time).total_seconds())/3600.0;

    solarPosData = new posdata;

	set_allSolarPosData();

	call_solPos();
}

Solar::Solar(Solar &s)
: solarTime(s.solarTime)
{
    latitude = s.latitude;
    longitude = s.longitude;
    interval = s.interval;
    aspect = s.aspect;
    slope = s.slope;
    theta = s.theta;
    phi = s.phi;
    solarIntensity = s.solarIntensity;

    year = s.year;
    month = s.month;
    day = s.day;
    hour = s.hour;
    minute = s.minute;
    second = s.second;
    offset = s.offset;

    solarPosData = new posdata;

    set_allSolarPosData();

    call_solPos();
}


Solar::~Solar()
{
	if(solarPosData)
		delete solarPosData;	
}

//bool Solar::compute_solar(int day_in, int month_in, int year_in,
//	int second_in, int minute_in, int hour_in,
//	double latitude_in, double longitude_in,
//	double timeZone_in, double aspect_in, double slope_in, double angleFromNorth)
//{
//  latitude = latitude_in;
//  longitude = longitude_in;
//  interval = 0;
//  // the raw input aspect value is expected to be in projected coordinates, except for the flat terrain case (slope and aspect have a value of 0.0),
//  // in which case the raw input value is still treated as projected coordinates even though it is technically a geographic value,
//  // the code still works despite this value difference because the aspect value is ignored when slope has a value of 0.0
//  aspect = aspect_in;
//  aspect = wrap0to360( aspect + angleFromNorth ); //convert FROM projected coordinates TO geographic
//  slope = slope_in;
//  theta = noDataValue;
//  phi = noDataValue;
//  solarIntensity = noDataValue;
//  
//  //note that this time constructor expects input times in UTC, not local time
//  solarTime = boost::local_time::local_date_time( boost::gregorian::date(year_in, month_in, day_in),
//                                                  boost::posix_time::time_duration(hour_in,minute_in,second_in,0),
//                                                  timeZone_in,  // <-- um, this usually is NOT a double, is a pointer constructed from a string
//                                                  boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
//  
//  year = year_in;
//  month = month_in;
//  day = day_in;
//  hour = hour_in;
//  minute = minute_in;
//  second = second_in;
//  
//  //Compute offset from UTC, including if in daylight savings or not
//  boost::posix_time::ptime UTC_time(solarTime.utc_time());
//  boost::posix_time::ptime Local_time(solarTime.local_time());
//  
//  //Get offset from UTC in decimal hours
//  offset = ((double) (Local_time - UTC_time).total_seconds())/3600.0;
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
	double aspect_in, double slope_in, double angleFromNorth)
{
    latitude = latitude_in;
    longitude = longitude_in;
    interval = 0;
    // the raw input aspect value is expected to be in projected coordinates, except for the flat terrain case (slope and aspect have a value of 0.0),
    // in which case the raw input value is still treated as projected coordinates even though it is technically a geographic value,
    // the code still works despite this value difference because the aspect value is ignored when slope has a value of 0.0
    aspect = aspect_in;
    aspect = wrap0to360( aspect + angleFromNorth ); //convert FROM projected coordinates TO geographic
    slope = slope_in;
    theta = noDataValue;
    phi = noDataValue;
    solarIntensity = noDataValue;

    solarTime = time_in;

    year = solarTime.local_time().date().year();
    month = solarTime.local_time().date().month();
    day = solarTime.local_time().date().day();
    hour = solarTime.local_time().time_of_day().hours();
    minute = solarTime.local_time().time_of_day().minutes();
    second = solarTime.local_time().time_of_day().seconds();

    //Compute offset from UTC, including if in daylight savings or not
    boost::posix_time::ptime UTC_time(solarTime.utc_time());
    boost::posix_time::ptime Local_time(solarTime.local_time());

    //Get offset from UTC in decimal hours
    offset = ((double) (Local_time - UTC_time).total_seconds())/3600.0;

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
	solarPosData->day = day;
	solarPosData->elevref = phi;
	solarPosData->hour = hour;
	solarPosData->interval = interval;
	solarPosData->latitude = latitude;
	solarPosData->longitude = longitude;
	solarPosData->minute = minute;
	solarPosData->month = month;
	solarPosData->second = second;
	solarPosData->tilt = slope;
	solarPosData->year = year;
	solarPosData->timezone = offset;
	solarPosData->etrtilt = solarIntensity;

	return true;
}

bool Solar::call_solPos()
{
	set_allSolarPosData();

	solarPosData->function = (S_ALL & ~S_DOY);
	errorCode = S_solpos(solarPosData);
    if( errorCode != 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Solar::call_solPos() call failed with error %d!", errorCode );
        return false;
    }

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
        << "latitude = " << latitude << std::endl
        << "longitude = " << longitude << std::endl
        << "interval = " << interval << std::endl
        << "aspect = " << aspect << std::endl
        << "slope = " << slope << std::endl
        << "theta = " << theta << std::endl
        << "phi = " << phi << std::endl
        << "solarTime = " << solarTime.local_time() << std::endl
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
        latitude = S.latitude;
        longitude = S.longitude;
        interval = S.interval;
        aspect = S.aspect;
        slope = S.slope;
        theta = S.theta;
        phi = S.phi;
        solarIntensity = S.solarIntensity;

        solarTime = S.solarTime;

        year = S.year;
        month = S.month;
        day = S.day;
        hour = S.hour;
        minute = S.minute;
        second = S.second;
        offset = S.offset;

        if(solarPosData)
            delete solarPosData;
        solarPosData = new posdata;

        set_allSolarPosData();

        call_solPos();
	}
	return *this;
}
