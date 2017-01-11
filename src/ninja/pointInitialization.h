/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  A concrete class for initializing WindNinja wind fields using
 *			 the point initialization input method (weather stations)
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

#ifndef POINT_INITIALIZATION_H
#define POINT_INITIALIZATION_H

#include "initialize.h"
#include "cellDiurnal.h"

#include <limits>	//for large number

#include "ogr_api.h"
#include "sstream"
#include "algorithm"
#include "iterator"
#include "string"
#include "iostream"
#include "fstream"
#include "math.h"
#ifndef Q_MOC_RUN
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_io.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/local_time/local_time.hpp"
#endif // Q_MOC_RUN
#include "gdal.h"
#include "cpl_conv.h"
#include "gdal_priv.h"
#include "gdal_util.h"
#include "solar.h"
#include "math.h"





class pointInitialization : public initialize
{
	public:

		pointInitialization();								//Default constructor
		virtual ~pointInitialization();						// Destructor
		
		//pointInitialization(pointInitialization const& m);               // Copy constructor
		//pointInitialization& operator= (pointInitialization const& m);   // Assignment operator

		//Implementation of base class virtual function for initializing volume wind fields using the
		//domain averaged wind method.
		virtual void initializeFields(WindNinjaInputs &input,
		        Mesh const& mesh,
		        wn_3dScalarField& u0,
		        wn_3dScalarField& v0,
		        wn_3dScalarField& w0,
		        AsciiGrid<double>& cloud,
		        AsciiGrid<double>& L,
		        AsciiGrid<double>& u_star,
		        AsciiGrid<double>& bl_height);


        struct preInterpolate
        {
            std::string stationName;
            double lat;
            double lon;
            double projXord;
            double projYord;
            double xord;
            double yord;
            double height;
            double speed;
            double direction;
            double w_speed;
            double temperature;
            double cloudCover;
            double influenceRadius;
            lengthUnits::eLengthUnits heightUnits;
            velocityUnits::eVelocityUnits inputSpeedUnits;
            velocityUnits::eVelocityUnits w_speedUnits;
            temperatureUnits::eTempUnits tempUnits;
            coverUnits::eCoverUnits cloudCoverUnits;
            lengthUnits::eLengthUnits influenceRadiusUnits;
            string datumType;
            string coordType;
            boost::posix_time::ptime datetime;
        };

        static vector<wxStation> interpolateFromDisk(std::string stationFilename, //master function for interpolation, and making wxStation stuff
                                        std::string demFile,
                                        std::vector<boost::posix_time::ptime> timeList,std::string timeZone);
        static vector<preInterpolate> readDiskLine(std::string stationFilename,std::string demFile); //reads in the data from disk


        static vector<wxStation> makeWxStation(vector<vector<preInterpolate> > data, std::string csvFile, std::string demFile); //prepares final product


        static vector<wxStation> interpolateNull(std::string csvFileName,std::string demFileName,vector<vector<preInterpolate> > vecStations,std::string timeZone);
        static vector<vector<preInterpolate> > interpolateTimeData(std::string csvFileName,std::string demFileName,vector<vector<preInterpolate> > vecStations,std::vector<boost::posix_time::ptime> timeList);
        static double interpolator(double iPoint, double lowX, double highX, double lowY, double highY);
        static double interpolateDirection(double lowDir,double highDir);
        static double unixTime(boost::posix_time::ptime time);
        
        static void fetchTest(std::string stationFilename,
                              std::string demFile,
                              std::vector<boost::posix_time::ptime> timeList, std::string timeZone, bool latest);

        static bool fetchStationFromBbox(std::string stationFilename,
                                    std::string demFile,
                                    std::vector<boost::posix_time::ptime> timeList, std::string timeZone, bool latest);

        static bool fetchStationByName(std::string stationFilename,
                                       std::string stationList,
                                       std::vector<boost::posix_time::ptime> timeList, std::string timeZone, bool latest);


        static std::vector<boost::posix_time::ptime> getTimeList( int startYear, int startMonth,
                                                int startDay, int startHour, int startMinute, int endYear,
                                                int endMonth, int endDay, int endHour, int endMinute,
                                                int nTimeSteps, std::string timeZone );
        static std::vector<boost::posix_time::ptime> getSingleTimeList(std::string timeZone);

        static void fetchMetaData(std::string fileName, std::string demFile, bool write);

        static std::string localSolarTime;

        static double stationBuffer;
        static void set_stationBuffer(double buffer,std::string units);
        static double get_stationBuffer();

        void newAuto(AsciiGrid<double> &dem);
        int storeHour(int nHours);

    private:
                std::string rawStationFilename;
                static const std::string dtoken;
                static const std::string dvar;
                static const std::string ndvar;
                static string BuildTime(std::string year_0,std::string month_0, std::string day_0,std::string clock_0,std::string year_1,std::string month_1,std::string day_1,std::string clock_1);
                static vector<string> UnifyTime(vector<boost::posix_time::ptime> timeList);
                static string IntConvert(int a);
                static vector<string> Split(char* str,const char* delim);
                void stringtoaster(int null,vector<int> vecnull);
                static vector<string>  InterpretCloudData(const double *dbCloud,int counter);
                static vector<vector<string> > VectorInterpretCloudData(vector<const double*>dbCloud,int smallcount, int largecount);
                static vector<string> CompareClouds(vector<string>low,vector<string>med,vector<string>high,int countlow,int countmed, int counthigh);
                static vector<string> UnifyClouds(const double *dvCloud,const double *dwCloud,const double *dxCloud,int count1,int count2,int count3,int backupcount);
                static void StringPrinter(char **stringdat, int counter, std::string name);
                static void FloatPrinter(const double *data, int counter,std::string name);
                static void VectorPrinter(std::vector<std::string> cata,std::string name);
                void doubleVectorPrinter(vector<const double*> cata,std::string name,int counter);
                static vector<double> Irradiate(const double* solrad,int smallcount, int largecount,std::string timeZone,double lat, double lon,char** times);
                void UnifyRadiation(vector<double> radiation);
                static vector<string> fixWindDir(const double *winddir,std::string filler,int count);

                const char* BuildSingleUrl(std::string station_id, std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                static const char* BuildSingleLatest(std::string station_id,std::string svar,int past, bool extendnetwork,std::string netids);
                const char* BuildRadiusLatest(std::string station_id,std::string radius,std::string limit,std::string svar,int past);
                const char* BuildRadiusUrl(std::string staion_id, std::string radius,std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* BuildLatLonUrl(std::string lat, std::string lon, std::string radius, std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* BuildLatLonLatest(std::string lat, std::string lon, std::string radius, std::string limit,std::string svar,int past);

                static std::string BuildMultiUrl(std::string station_ids,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                static std::string BuildMultiLatest(std::string station_ids);

                static std::string BuildBboxUrl(std::string lat1,std::string lon1, std::string lat2, std::string lon2,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                static std::string BuildBboxLatest(std::string lat1, std::string lon1, std::string lat2, std::string lon2);
                static std::string BuildUnifiedBbox(double lat1,double lon1, double lat2,double lon2,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                static std::string BuildUnifiedLTBbox(double lat,double lon1, double lat2, double lon2);
                static void fetchStationData(std::string stationFilename, std::string URL,
                                    std::string timeZone, bool latest);

                double dfInvDistWeight;

};

#endif /* POINT_INITIALIZATION_H */
