/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download weather data from nomads
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#ifdef WITH_NOMADS_SUPPORT

#ifndef NOMADS_WX_MODEL_H_
#define NOMADS_WX_MODEL_H_

#include <string>

#include "cpl_port.h"
#include "cpl_conv.h"
#include "cpl_http.h"

#include <vector>
#include <sstream>

#include "ninja_conv.h"
#include "utc_time.h"

class NomadsWxModel
{

public:
    enum NomadsDimension { TWO_DIM, THREE_DIM };
    enum NomadsWindType { UVW, SPD_DIR };
    enum NomadsCloudType { PERCENT, FRACTION, BOOLEAN, MISSING };
    enum NomadsTempType { MIN_MAX, NORMAL };

    NomadsWxModel();
    NomadsWxModel( std::string osModelName, std::string osSubModelName );
    virtual ~NomadsWxModel();

    //virtual int FetchData( std::string osUrl, const char *pszOutputFile );

    virtual std::string GetHost() { return osHost; }
    virtual std::string GetInventoryPath() { return osInventoryPath; }
    virtual std::string GetSubsetPath() { return osSubsetPath; }
    virtual std::string GetModelName() { return osModelName; }
    virtual std::string GetModelShortName() { return osModelShortName; }
    virtual std::string GetSubModelShortName() { return osSubModelShortName; }
    virtual std::string GetSubModelName() { return osSubModelShortName; }
    virtual double GetWindHeight() { return dfWindHeight; }
    virtual double GetTempHeight() { return dfTempHeight; }
    virtual NomadsDimension GetDimension() { return eDimension; }
    virtual NomadsWindType GetWindType() { return eWindType; }
    virtual NomadsCloudType GetCloudType() { return eCloudType; }
    virtual NomadsTempType GetTempType() { return eTempType; }

    virtual int SetHost( std::string osNewHost );
    virtual int SetInventoryPath( std::string osNewPath );
    virtual int SetSubsetPath( std::string osNewPath );
    virtual int SetModelName( std::string osNewName );
    virtual int SetModelShortName( std::string osNewName );
    virtual int SetSubModelShortName( std::string osNewName );
    virtual int SetSubModelName( std::string osNewName );
    virtual int SetWindHeight( double dfHeight );
    virtual int SetTempHeight( double dfHeight );
    virtual int SetDimension( NomadsDimension eDim );
    virtual int SetWindType( NomadsWindType eType );
    virtual int SetCloudType( NomadsCloudType eType );
    virtual int SetTempType( NomadsTempType eType );

    virtual int UpdateTimes( int bForce = FALSE );
    virtual std::string GetStartTime() { return osStartTime; }
    virtual std::string GetEndTime() { return osEndTime; }
    int FilterModelList( char **papszFileList );
    int FilterSubModelList( char **papszFileList, int *pnForecastRun,
                            int *pnLastForecast, int *pnSecondToLastForecast );

    virtual int GetUpdateInterval() { return nUpdateInterval; }
    virtual int SetUpdateInterval( int nInterval );

    /* Pure virtual */
    virtual int GetLastForecastHour() = 0;
    virtual std::vector<int> GetForecastHourSequence() = 0;
    virtual std::vector<int> GetForecastRunTimes() = 0;
    virtual std::vector<std::string> GetValidSubModelNames() = 0;
    /* Override if necessary */
    virtual std::string FormFinalUrl( std::string osUrl, const char *pszLastFolder );
    virtual std::string GetModelFilter() { return osModelShortName; }
    virtual std::string GetSubModelFilter() { return osSubModelShortName; }
    virtual int IsDynamicTime() { return FALSE; }

#ifdef NINJA_BUILD_TESTING
    int TestUpdateTimes();
    int TestGenerateDownloadList();
    int TestUpdateTimesAndBuildFileList();
    int TestAddHours();
#endif

protected:
    int bUseVSICurl;
    std::string osHost;
    std::string osInventoryPath;
    std::string osSubsetPath;
    std::string osModelName;
    std::string osModelShortName;
    std::string osSubModelName;
    std::string osSubModelShortName;

    char **papszCachedDirList;
    char **papszCachedFileList;

    NomadsDimension eDimension;
    NomadsWindType eWindType;
    NomadsCloudType eCloudType;
    NomadsTempType eTempType;

    double dfWindHeight;
    double dfTempHeight;

    std::string osStartTime;
    std::string osEndTime;

    int nLastUpdateTime;
    int nUpdateInterval;

    char ** ReadNomadsDir( std::string osUrl );
    char * GetDateSpan( char **papszFileList, long *pnMinDate,
                        long *pnMaxDate  );
    virtual std::vector<std::string> GenerateDownloadList( std::string osStart, std::string osEnd );
    static std::string AddHours( std::string osDateTime, int nHoursToAdd );

    void ToLower( std::string &s );

};

#endif /* NOMADS_WX_MODEL_H_ */

#endif /* WITH_NOMADS_SUPPORT */

