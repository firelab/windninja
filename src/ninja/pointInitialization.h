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
//#include "cellDiurnal.h"

#include <limits>	//for large number
#include <math.h>

// shorten boost namespaces
namespace blt = boost::local_time;
namespace bpt = boost::posix_time;
namespace bg = boost::gregorian;

class pointInitialization : public initialize
{
    public:
        pointInitialization();
        virtual ~pointInitialization();

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
            std::string datumType;
            std::string coordType;
            boost::posix_time::ptime datetime;
        };

        virtual void initializeFields(WindNinjaInputs &input,
                        Mesh const& mesh,
                        wn_3dScalarField& u0,
                        wn_3dScalarField& v0,
                        wn_3dScalarField& w0,
                        AsciiGrid<double>& cloud);

        //master function for interpolation, and making wxStation stuff
        static vector<string> openCSVList(std::string csvPath);        
        static vector<wxStation> readWxStations(std::string demFileName,std::string timeZone);
        static vector<wxStation> interpolateFromDisk(std::string demFile,
                                            std::vector<boost::posix_time::ptime> timeList,
                                            std::string timeZone);

        static vector<preInterpolate> readDiskLine(std::string demFile,std::string stationLoc);
        static vector<std::string> fetchWxStationID();
        static bool checkWxStationSize(vector<std::string> wxStationIDs);
        static vector<wxStation> makeWxStation(vector<vector<preInterpolate> > data, std::string demFile); //prepares final product

        static vector<wxStation> interpolateNull(std::string demFileName,
                                                vector<vector<preInterpolate> > vecStations,
                                                std::string timeZone);

        static vector<vector<preInterpolate> > interpolateTimeData(std::string demFileName,
                                                vector<vector<preInterpolate> > vecStations,
                                                std::vector<boost::posix_time::ptime> timeList);

        static double interpolator(double iPoint, double lowX, double highX, double lowY, double highY);
        static double interpolateDirection(double lowDir, double highDir);
        static double unixTime(boost::posix_time::ptime time);

        static bool fetchStationFromBbox(std::string demFile,
                            std::vector<boost::posix_time::ptime> timeList,
                            std::string timeZone, bool latest);

        static bool fetchStationByName(std::string stationList,
                                        std::vector<boost::posix_time::ptime> timeList,
                                        std::string timeZone, bool latest);

        static void writeStationLocationFile(std::string stationPath,std::string demFile,bool current_data);
        static std::vector<boost::posix_time::ptime> getTimeList(int startYear, int startMonth,
                                                                int startDay, int startHour,
                                                                int startMinute, int endYear,
                                                                int endMonth, int endDay,
                                                                int endHour, int endMinute,
                                                                int nTimeSteps, std::string timeZone);
        static boost::posix_time::ptime generateSingleTimeObject(int year,
                                                                 int month,
                                                                 int day,
                                                                 int hour,
                                                                 int minute, std::string timeZone);
        static int checkFetchTimeDuration(std::vector<boost::posix_time::ptime> timeList);
        static void setCustomAPIKey(std::string api_token);

        static void fetchMetaData(std::string fileName, std::string demFile, bool write);
        static void SetRawStationFilename(std::string filename);
        static void setStationBuffer(double buffer, std::string units);
        static void storeFileNames(vector<std::string> statLoc);
        static void storeTZAbbrev(std::string tzAbbr);
        static void setLocalStartAndStopTimes(boost::local_time::local_date_time start,
                                              boost::local_time::local_date_time stop);
        static std::string generatePointDirectory(std::string demFile,std::string outPath, bool latest);
        static bool removeBadDirectory(std::string badStationPath);
        static void writeStationOutFile(std::vector<wxStation> stationVect,
                                        std::string basePathName,
                                        std::string demFileName,
                                        bool latest);
        static bool validateTimeData(vector<vector<preInterpolate> > wxStationData,vector<boost::posix_time::ptime> timeList);
        static int directTemporalInterpolation(int posIdx, int negIdx);
        static void unifyInterpolation(std::string data_source, vector<vector<preInterpolate> > rawStationVector, vector<vector<preInterpolate> > interpolatedWxData);

        static std::string rawStationFilename; //Need to be public, so that they can be accessed by the GUI
        static vector<boost::local_time::local_date_time> start_and_stop_times;
        static bool enforce_limits;
        static std::string error_msg;

    private:
        void setInitializationGrids(WindNinjaInputs& input);

        static std::string BuildTime(std::string year_0, std::string month_0,
                                std::string day_0, std::string clock_0,
                                std::string year_1, std::string month_1,
                                std::string day_1, std::string clock_1);

        static vector<std::string> UnifyTime(vector<boost::posix_time::ptime> timeList);
        static vector<std::string> Split(char* str, const char* delim);
        static vector<std::string>  InterpretCloudData(const double *dbCloud, int counter);

        static vector<std::string> CompareClouds(vector<std::string>low, vector<std::string>med,
                                            vector<std::string>high, int countlow,
                                            int countmed, int counthigh);

        static vector<std::string> UnifyClouds(const double *dvCloud, const double *dwCloud,
                                          const double *dxCloud, int count1,
                                          int count2,int count3, int backupcount);

        static vector<double> Irradiate(vector<std::string> solar_radiation,
                                        std::string timeZone,double lat,double lon, char** times);

        static vector<std::string> fixWindDir(const double *winddir, std::string filler, int count);

        static std::string BuildMultiUrl(std::string station_ids, std::string yearx,
                                         std::string monthx, std::string dayx,
                                         std::string clockx, std::string yeary,
                                         std::string monthy, std::string dayy,
                                         std::string clocky);

        static std::string BuildMultiLatest(std::string station_ids);

        static std::string BuildBboxUrl(std::string lat1, std::string lon1,
                                        std::string lat2, std::string lon2,
                                        std::string yearx, std::string monthx,
                                        std::string dayx, std::string clockx,
                                        std::string yeary, std::string monthy,
                                        std::string dayy, std::string clocky);

        static std::string BuildBboxLatest(std::string lat1, std::string lon1,
                                           std::string lat2, std::string lon2);

        static std::string BuildUnifiedBbox(double lat1, double lon1,
                                            double lat2, double lon2,
                                            std::string yearx, std::string monthx,
                                            std::string dayx, std::string clockx,
                                            std::string yeary, std::string monthy,
                                            std::string dayy, std::string clocky);

        static std::string BuildUnifiedLTBbox(double lat, double lon1, double lat2, double lon2);
        static double getStationBuffer();
        static double parseStationHeight(const char *name_list);
        static std::vector<std::string> outputToVec(const double* dataArray,int data_idx,int dataCount,std::string data_name);
        static std::vector<std::string> fixEmptySensor(std::vector<std::string> data_vec,std::string data_name,std::vector<std::string> valid_vec);
        static bool fetchStationData(std::string URL, std::string timeZone, bool latest);

        static std::string dtoken;
        static const std::string backup_token;
        static const std::string dvar;
        static const std::string ndvar;
        static const std::string baseUrl;
        static double stationBuffer;
        double dfInvDistWeight;
        static std::vector<std::string> stationFiles;
        static std::string tzAbbrev; //Time Zone Abbreviation

        friend class wxStation;

};
#endif /* POINT_INITIALIZATION_H */
