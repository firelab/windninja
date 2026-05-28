/******************************************************************************
*
* $Id: farsiteAtm.cpp 816 2011-02-15 16:34:53Z jaforthofer $
*
* Project:  WindNinja
* Purpose:  For writing FARSITE atmosphere files (*.atm)
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

#include "farsiteAtm.h"

farsiteAtm::farsiteAtm()
: missingTimeFiller(boost::local_time::not_a_date_time)
{
    boost::gregorian::date d(1800,boost::gregorian::Jan,1);
    boost::posix_time::time_duration td(0,0,0,0);
    boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("UTC"));
    missingTimeFiller = boost::local_time::local_date_time(d, td, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
}

farsiteAtm::~farsiteAtm()
{

}

void farsiteAtm::push(boost::local_time::local_date_time inTime, std::string inSpeedName, std::string inDirectionName, std::string inCloudCoverName)
{
    vector<std::string> nameArray(3);
    nameArray[0] = inSpeedName;
    nameArray[1] = inDirectionName;
    nameArray[2] = inCloudCoverName;

    if(inTime.is_not_a_date_time())
    {
        data.insert(std::pair< boost::local_time::local_date_time, std::vector<std::string> > (missingTimeFiller, nameArray));
        missingTimeFiller += boost::gregorian::days(1);
    }
    else if(data.find(inTime) != data.end())
    {
        throw std::runtime_error("attempting to add a duplicate time into farsiteAtm list.");
    }
    else
    {
        data.insert(std::pair< boost::local_time::local_date_time, std::vector<std::string> > (inTime, nameArray));
    }
}

/**
* Writes a FARSITE atmosphere file (*.atm).
* This file can only be written if the units and output wind height are specific values:
* For ENGLISH -> speed in mph and wind height at 20 feet above the vegetation.
* For METRIC  -> speed in kph and wind height at 10 meters above the vegetation.
* Also, all of the Windninja runs must be writing their output files to the same directory,
* if not, the atmosphere file is not written.
*
* constructs the atm filename from the pushed speed files
*
* @param writeSeparateAtmFiles Whether to write a separate atm file for each run, or a single atm file for the combined set of runs.
* @param velocityUnits Units of the velocity file.
* @param windHeight Height of wind above vegetation.
* @param stripPaths Optional parameter, whether to use filenames with or without paths. Default is without paths.
* @return True if file is written, false if not written.
*/
bool farsiteAtm::writeAtmFile(bool writeSeparateAtmFiles, velocityUnits::eVelocityUnits velocityUnits, double windHeight, bool stripPaths)
{
    map<boost::local_time::local_date_time, std::vector<string> >::iterator endIter = std::next(data.begin());  // numAtmFiles = 1;
    if(writeSeparateAtmFiles == true)
    {
        endIter = data.end();  // numAtmFiles = data.size();
    }

    map<boost::local_time::local_date_time, std::vector<string> >::iterator iter;
    for(iter = data.begin(); iter != endIter; ++iter)
    {

    std::string filePath = CPLGetPath((*iter).second[0].c_str());
    std::string fileroot(CPLGetBasename((*iter).second[0].c_str()));
    // remove the _vel part from the file basename
    int stringPos = fileroot.find_last_of("_vel");
    if(stringPos != fileroot.npos)
    {
        fileroot.erase(stringPos);
    }
    std::string filename(CPLFormFilename(filePath.c_str(), fileroot.c_str(), "atm"));

    //Open atm file for writing, if already exists replace
    std::ofstream outputFile(filename.c_str(), std::fstream::trunc);
    if(outputFile.bad())
    {
        std::stringstream outMessage;
        outMessage << "Problem writing the FARSITE atmosphere file (*.asc) called " << filename \
               << ".\nSometimes this happens because the file is already open by another program.\n";
        throw std::runtime_error(outMessage.str());
    }

    outputFile << "WINDS_AND_CLOUDS\n";

    double windHeightFeet = windHeight;
    lengthUnits::fromBaseUnits(windHeightFeet, lengthUnits::feet);
    if(velocityUnits == velocityUnits::milesPerHour && FloatingPoint<double>(windHeightFeet).AlmostEquals(FloatingPoint<double>(20.0)))
    {
        outputFile << "ENGLISH\n";
    }
    else if(velocityUnits == velocityUnits::kilometersPerHour && FloatingPoint<double>(windHeight).AlmostEquals(FloatingPoint<double>(10.0)))
    {
        outputFile << "METRIC\n";
    }
    else
    {
        double heightInFeet = windHeight;
        lengthUnits::fromBaseUnits(heightInFeet, lengthUnits::feet);
        std::stringstream outMessage;
        outMessage << "FARSITE atmosphere file (*.atm) cannot be written because the speed units and/or output " \
                "wind height above ground are incorrect.\n  The possible choices are:\n    Wind speed in mph and wind " \
                "height at 20 feet.\n      or\n    Wind speed in kph and wind height at 10 meters.\n";
        outMessage << "Your wind speed units are in " << velocityUnits::getString(velocityUnits) << " and output wind height is " \
                << heightInFeet << " feet (" << windHeight << " meters)." << std::endl;
        throw std::runtime_error(outMessage.str());
    }

    boost::local_time::local_time_facet* timeOutputFacet;
    timeOutputFacet = new boost::local_time::local_time_facet();
    //NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
    //		THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
    //		https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
    //		http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203

    outputFile.imbue(std::locale(std::locale::classic(), timeOutputFacet));
    timeOutputFacet->format("%m %d %H%M");

    std::string tmp;

    //Make a default local/date time to use if there isn't any valid local date/time
    boost::gregorian::date d(2011,boost::gregorian::Jan,1);
    boost::posix_time::time_duration td(12,0,0,0);
    boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("MST-07"));   //doesn't matter what time zone is used...
    boost::local_time::local_date_time defaultDateTime(d, td, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    // for numAtmFiles = 1;, numFilesPerAtm = data.size();
    map<boost::local_time::local_date_time, std::vector<string> >::iterator fileStartIter = data.begin();
    map<boost::local_time::local_date_time, std::vector<string> >::iterator fileEndIter = data.end();
    if(writeSeparateAtmFiles == true)
    {
        // for numAtmFiles = data.size();, numFilesPerAtm = 1;
        fileStartIter = iter;
        fileEndIter = std::next(iter);
    }

    //Write the data
    map<boost::local_time::local_date_time, std::vector<string> >::iterator fileIter;
    for(fileIter = fileStartIter; fileIter != fileEndIter; ++fileIter)
    {
        if((*fileIter).first.is_not_a_date_time() || (*fileIter).first.utc_time().date().year() < 1900) //if invalid time, just output default time
        {
            outputFile << defaultDateTime;
        }
        else
        {
            outputFile << (*fileIter).first;
        }

        for(int j=0; j<3; j++)
        {
            if(stripPaths == true)
            {
                tmp = (*fileIter).second[j];
                tmp = std::string(CPLGetFilename(tmp.c_str()));
//                stringPos = tmp.find_last_of('/');
//                if(stringPos > 0)
//                    tmp = tmp.substr(stringPos+1);
                outputFile << " " << tmp;
            }
            else
            {
                outputFile << " " << (*fileIter).second[j];
            }
        }

        outputFile << "\n";
    }  // for(fileIter = fileStartIter; fileIter != fileEndIter; ++fileIter)

    outputFile.close();

    }  // for(iter = data.begin(); iter != endIter; ++iter)

    return true;
}
