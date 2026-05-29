/******************************************************************************
 *
 * $Id: farsiteAtm.h 762 2011-02-03 14:53:47Z jaforthofer $
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

#ifndef FARSITE_ATM_H
#define FARSITE_ATM_H

#include <iostream>
#include <fstream>

#include <string>
#ifndef Q_MOC_RUN
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#endif
#include "ninjaMathUtility.h"
#include "ninjaUnits.h"
#include "cpl_conv.h"

/*
 * Class used for writing FARSITE atmosphere files (*.atm) when multiple ninja runs are done in a time sequence.
 */
class farsiteAtm
{	
public:
    farsiteAtm();
    ~farsiteAtm();

    void reset(std::size_t numRuns);
    void push(unsigned int runNumber, boost::local_time::local_date_time inTime, std::string inSpeedName, std::string inDirectionName, std::string inCloudCoverName);
    bool writeAtmFile(bool writeSeparateAtmFiles, velocityUnits::eVelocityUnits velocityUnits, double windHeight, bool stripPaths = true);

private:
    std::vector<boost::local_time::local_date_time> times;
    std::vector<std::string> speedNames;
    std::vector<std::string> directionNames;
    std::vector<std::string> cloudCoverNames;
};

#endif	//FARSITE_ATM_H
