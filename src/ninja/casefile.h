/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Input/Output Handling of Casefile
 * Author:   Rui Zhang <ruizhangslc2017@gmail.com>
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

#ifndef CASEFILE_H
#define CASEFILE_H

#include <fstream>
#include <sstream>
#include <cpl_string.h>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/local_time/local_time_io.hpp>

#ifdef _OPENMP
#include <omp.h>
#include "omp_guard.h"
#endif  // #ifdef _OPENMP

#ifdef _OPENMP
extern omp_lock_t netCDF_lock;
#endif  // #ifdef _OPENMP

class CaseFile
{

private:

    std::string caseZipFile;
    std::string finalCaseZipFile;
    bool isZipOpen;
    void* zipHandle;

    int setCaseZipFileCount;

public:

    CaseFile();

    void setCaseZipFile(std::string caseZippFile);
    void updateCaseZipFile(std::string newCaseZipFile);
    void renameCaseZipFile();

    void openCaseZipFile();
    void closeCaseZipFile();

    void addFileToZip(const std::string& withinZipPathedFilename, const std::string& fileToAdd);

    bool getIsZipOpen();
    std::string getCaseZipFile();

    std::string getCurrentTime();
    std::string convertDateTimeToStd(const boost::local_time::local_date_time& ninjaTime);

};

#endif	/* CASEFILE_H */

