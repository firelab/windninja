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
{
}

farsiteAtm::~farsiteAtm()
{

}

void farsiteAtm::reset(std::size_t numRuns)
{
    times.resize(numRuns, boost::local_time::local_date_time(boost::local_time::not_a_date_time));
    speedNames.resize(numRuns);
    directionNames.resize(numRuns);
}

void farsiteAtm::push(unsigned int runNumber, boost::local_time::local_date_time inTime, std::string inSpeedName, std::string inDirectionName)
{
    //TODO: setup a way to handle replicate times, when "!inTime.is_not_a_date_time()", if that is even necessary
    times[runNumber] = inTime;
    speedNames[runNumber] = inSpeedName;
    directionNames[runNumber] = inDirectionName;
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
    unsigned int numRuns = times.size();

    unsigned int numAtmFiles = 1;
    if(writeSeparateAtmFiles == true)
    {
        numAtmFiles = numRuns;
    }

    for(unsigned int atmIdx = 0; atmIdx < numAtmFiles; atmIdx++)
    {
        std::string filePath = CPLGetPath(speedNames[atmIdx].c_str());
        std::string fileroot(CPLGetBasename(speedNames[atmIdx].c_str()));
        // remove the _vel part from the file basename
        size_t stringPos = fileroot.find("_vel");
        if(stringPos != fileroot.npos)
        {
            fileroot.erase(stringPos);
        }
        std::string filename(CPLFormFilename(filePath.c_str(), fileroot.c_str(), "atm"));

        // Open atm file for writing, if already exists replace
        std::ofstream outputFile(filename.c_str(), std::fstream::trunc);
        if(outputFile.bad())
        {
            std::stringstream outMessage;
            outMessage << "Problem writing the FARSITE atmosphere file (*.asc) called " << filename \
                   << ".\nSometimes this happens because the file is already open by another program.\n";
            throw std::runtime_error(outMessage.str());
        }

        outputFile << "WINDS\n";

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

        //Make a default local/date time to use if there isn't any valid local date/time
        boost::gregorian::date d(2011,boost::gregorian::Jan,1);
        boost::posix_time::time_duration td(12,0,0,0);
        boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("MST-07"));   //doesn't matter what time zone is used...
        boost::local_time::local_date_time defaultDateTime(d, td, zone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

        // for numAtmFiles = 1;
        unsigned int fileStartIdx = atmIdx;
        unsigned int fileStopIdx = numRuns;
        if(writeSeparateAtmFiles == true)
        {
            // for numAtmFiles = numRuns;
            fileStartIdx = atmIdx;
            fileStopIdx = atmIdx+1;
        }

        //Write the data
        for(unsigned int fileIdx = fileStartIdx; fileIdx < fileStopIdx; fileIdx++)
        {
            if(times[fileIdx].is_not_a_date_time()) //if invalid time, just output default time
            {
                outputFile << defaultDateTime;
            }
            else
            {
                outputFile << times[fileIdx];
            }

            if(stripPaths == true)
            {
                outputFile << " " << CPLGetFilename(speedNames[fileIdx].c_str());
                outputFile << " " << CPLGetFilename(directionNames[fileIdx].c_str());
            }
            else
            {
                outputFile << " " << speedNames[fileIdx];
                outputFile << " " << directionNames[fileIdx];
            }

            outputFile << "\n";
        }  // for(unsigned int fileIdx = fileStartIdx; fileIdx < fileStopIdx; fileIdx++)

        outputFile.close();
    }  // for(unsigned int atmIdx = 0; atmIdx < numAtmFiles; atmIdx++)

    return true;
}
