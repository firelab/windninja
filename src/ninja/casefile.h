/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Input/Output Handling of Casefile for VTK
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

#include <iostream>
#include <fstream>
#include <filesystem>
#include "cpl_vsi.h"
#include "gdal.h"
#include <cpl_conv.h>
#include <cpl_string.h>
#include <cpl_error.h> 
#include <ctime>
#include "cpl_minizip_zip.h"
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/local_time/local_time_io.hpp>
#include <chrono>
#include <mutex>
#include <vector>

class CaseFile {

private:
    static std::string zipfilename;

    static std::string directory;
    static bool zipalreadyopened; 

    static std::vector<boost::local_time::local_date_time> timesforWX; 
    static std::vector<double> boundingboxarr; 
    static bool downloadedfromdem; 
    static std::string elevsource; 

public:
    CaseFile(); 

    void addFileToZip(const std::string& zipFilePath, const std::string& dirPath, const std::string& fileToAdd, const std::string& usrlocalpath);

    void deleteFileFromPath(std::string directoryPath, std::string filenameToDelete);
    bool lookforzip(const std::string& zipFilePath, const std::string& directory); 
    bool isCfgFile(const std::string& filePath); 
    bool isVTKFile(const std::string& filePath); 

    std::string parse(const std::string& type, const std::string& path) ;
    std::string convertDateTime(const boost::local_time::local_date_time& ninjaTime); 
    bool lookfordate(const std::string& date) ;

    std::string getTime(); 
    std::string getdir() ; 
    void setdir(std::string dir); 

    std::string getzip () ; 
    void setzip(std::string zip); 

    void setZipOpen(bool zipopen); 
    bool getZipOpen(); 
    void setTimeWX(std::vector<boost::local_time::local_date_time> timeList) ; 
   
    std::vector<boost::local_time::local_date_time> getWXTIME() ; 

    void setBoundingBox ( std::vector<double> boundingboxarrr); 

    static void setElevSource (std::string elevsourcee ); 

    void setDownloadedFromDEM (bool downloadedfromdemm) ; 

    std::string getElevSource () ; 
    bool getDownloadedFromDEM () ; 

    std::vector<double> CaseFile::getBoundingBox (); 
};