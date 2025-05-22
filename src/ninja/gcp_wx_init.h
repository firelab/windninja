
/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  Handle GCP fetching for weather model data
* Author:   masonwillman <masonwillman0@gmail.com>
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

#ifndef GCP_WX_MODEL_H_
#define GCP_WX_MODEL_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

#include "gdal_utils.h"
#include "cpl_http.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "wxModelInitialization.h"

class GCPWxModel : public wxModelInitialization
{

public:
  GCPWxModel();
  GCPWxModel( std::string filename );
  GCPWxModel( const char *pszModelKey );
  GCPWxModel(GCPWxModel const&A);
  virtual ~GCPWxModel();

  virtual std::string fetchForecast( std::string demFile, int nhours);

  virtual std::vector<blt::local_date_time>
  getTimeList(const char *pszVariable, blt::time_zone_ptr timeZonePtr);

  virtual void set3dGrids( WindNinjaInputs &input, Mesh const& mesh );
  virtual bool identify( std::string fileName );
  const char ** FindModelKey( const char *pszFilename );
  virtual std::vector<std::string> getVariableList();
  virtual std::string getForecastIdentifier();
  virtual std::string getForecastReadable();
  virtual double getGridResolution();
  virtual int getStartHour();
  virtual int getEndHour();
  virtual void checkForValidData();
  virtual double Get_Wind_Height() { return 10; }
  virtual void setSurfaceGrids( WindNinjaInputs &input,
                               AsciiGrid<double> &airGrid,
                               AsciiGrid<double> &cloudGrid,
                               AsciiGrid<double> &uGrid,
                               AsciiGrid<double> &vGrid,
                               AsciiGrid<double> &wGrid );

  void setDateTime(boost::gregorian::date date1, boost::gregorian::date date2, std::string hours1, std::string hours2);


private:
  const char *pszKey;
  const char **ppszModelData;

  int InitializeForecastTimes();
  std::string findBands(std::string idxFilePath, std::vector<std::string> variables);
  char* FindForecast(const char* pszFilePath, time_t nTime);
  std::vector<std::vector<std::string>> getOptions(const std::vector<std::string>& bands, const std::string buffer[4]);
//  bool CopyFileToVSI(const std::string& srcPath, const std::string& destPath);
  static void ThreadFunc(void* pData);
  static int fetchData( boost::posix_time::ptime dt, std::string outPath, std::vector<std::vector<std::string>> options, int i );

  std::string privateKey;
  std::string clientEmail;

  boost::gregorian::date startDate;
  boost::gregorian::date endDate;
  std::string starthours;
  std::string endhours;
  struct ThreadParams
  {
    boost::posix_time::ptime dt;
    std::string outPath;
    std::vector<std::vector<std::string>> options;
    int i;
  };

};

#endif /* GCP_WX_MODEL_H_ */


