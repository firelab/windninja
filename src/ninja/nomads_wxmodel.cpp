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

#include "nomads_wxmodel.h"

void NomadsWxModel::ToLower( std::string &s )
{
    unsigned int i = 0;
    for( ; i < s.size(); i++ )
        s[i] = tolower( s[i] );
}

NomadsWxModel::NomadsWxModel()
{
    bUseVSICurl = FALSE;
    osHost = "http://nomads.ncep.noaa.gov/";
    papszCachedDirList = NULL;
    papszCachedFileList = NULL;
}

NomadsWxModel::NomadsWxModel( std::string osModelShortName,
                              std::string osSubModelShortName )
{
    bUseVSICurl = FALSE;
    osHost = "http://nomads.ncep.noaa.gov/";
    this->osModelShortName = osModelShortName;
    this->osSubModelShortName = osSubModelShortName;
    ToLower( this->osModelShortName );
    ToLower( this->osSubModelShortName );
    osInventoryPath = "pub/data/nccf/com/" + this->osModelShortName + "/prod/";
    papszCachedDirList = NULL;
    papszCachedFileList = NULL;
}

NomadsWxModel::~NomadsWxModel()
{
    CSLDestroy( papszCachedDirList );
    CSLDestroy( papszCachedFileList );
}

/**
 * \brief Add n hours to a UTC formatted date/time string.
 *
 * Parse a UTC date/time string and add n hours.  Must increment the day every
 * time we pass 00Z.
 *
 * \param osDateTime the start date/time
 * \param nHour the number of hours to add to osDateTime
 * \return a new UTC formatted string, formatted the same as the input
 */
std::string NomadsWxModel::AddHours( std::string osDateTime, int nHoursToAdd )
{
    if( osDateTime.empty() )
    {
        return osDateTime;
    }
    char **papszTemp = NULL;
    int bDateDelimit = FALSE;
    int bTimeDelimit = FALSE;
    if( (signed)osDateTime.find( "-" ) > -1 )
    {
        bDateDelimit = TRUE;
    }
    if( (signed)osDateTime.find( ":" ) > -1 )
    {
        bTimeDelimit = TRUE;
    }
    papszTemp = CSLTokenizeString2( osDateTime.c_str(), "T",
                                    CSLT_STRIPLEADSPACES |
                                    CSLT_STRIPENDSPACES );

    if( CSLCount( papszTemp ) != 2 )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Could not parse date/time: '%s'",
                  osDateTime.c_str() );
        return osDateTime;
    }
    int nYear, nMonth, nDay;
    int nHour, nMinute;
    char **papszDate, **papszTime;
    if( bDateDelimit )
    {
        papszDate = CSLTokenizeString2( papszTemp[0], "-",
                                        CSLT_STRIPLEADSPACES |
                                        CSLT_STRIPENDSPACES );
        if( CSLCount( papszDate ) != 3 )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Could not parse date/time: '%s'",
                      osDateTime.c_str() );
            return osDateTime;
        }
        nYear = atoi( papszDate[0] );
        nMonth = atoi( papszDate[1] );
        nDay = atoi( papszDate[2] );
    }
    else
    {
        sscanf( papszTemp[0], "%04d%02d%02d", &nYear, &nMonth, &nDay );
    }
    if( bTimeDelimit )
    {
        papszTime = CSLTokenizeString2( papszTemp[1], ":",
                                        CSLT_STRIPLEADSPACES |
                                        CSLT_STRIPENDSPACES );
        if( CSLCount( papszTime ) != 2 )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Could not parse date/time" );
            return osDateTime;
        }
        nHour = atoi( papszTime[0] );
        nMinute = atoi( papszTime[1] );
    }
    else
    {
        sscanf( papszTemp[1], "%02d%02d", &nHour, &nMinute );
    }
    CSLDestroy( papszTemp );

    int nAddDays = ( nHour + nHoursToAdd ) / 24;
    int nAddHours = ( nHour + nHoursToAdd ) % 24;
    nDay += nAddDays;
    nHour = nAddHours;
    /*
    ** Check for overflow.
    */
    if( nHour > 23 )
    {
        nDay += nHour / 23;
        nHour = nHour % 23;
    }
    int nDaysInMonth = NinjaDaysInMonth[nMonth - 1];
    if( nMonth == 2 && IS_LEAP_YEAR( nYear ) )
    {
        CPLDebug( "NOMADS", "Detected leap year: %d", nYear );
        nDaysInMonth++;
    }

    if( nDay > nDaysInMonth )
    {
        nMonth += 1;
        nDay = nDay % nDaysInMonth;
    }
    if( nMonth > 12 )
    {
        nYear += 1;
        nMonth = nMonth % 12;
    }
    std::string osNewDateTime;
    if( bDateDelimit )
    {
        osNewDateTime = CPLSPrintf( "%d-%02d-%02dT", nYear, nMonth, nDay );
    }
    else
    {
        osNewDateTime = CPLSPrintf( "%d%02d%02dT", nYear, nMonth, nDay );
    }
    if( bTimeDelimit )
    {
        osNewDateTime = CPLSPrintf( "%s%02d:%02d", osNewDateTime.c_str(),
                                    nHour, nMinute );
    }
    else
    {
        osNewDateTime = CPLSPrintf( "%s%02d%02d", osNewDateTime.c_str(), 
                                    nHour, nMinute );
    }
    return osNewDateTime;
}

int NomadsWxModel::SetHost( std::string osNewHost )
{
    osHost = osNewHost;
    return 0;
}

int NomadsWxModel::SetInventoryPath( std::string osNewPath )
{
    osInventoryPath = osNewPath;
    return 0;
}

int NomadsWxModel::SetSubsetPath( std::string osNewPath )
{
    osSubsetPath = osNewPath;
    return 0;
}

int NomadsWxModel::SetModelName( std::string osNewName )
{
    osModelName = osNewName;
    return 0;
}

int NomadsWxModel::SetModelShortName( std::string osNewName )
{
    osModelShortName = osNewName;
    return 0;
}

int NomadsWxModel::SetSubModelShortName( std::string osNewName )
{
    osSubModelShortName = osNewName;
    return 0;
}

int NomadsWxModel::SetSubModelName( std::string osNewName )
{
    osSubModelName = osNewName;
    return 0;
}

int NomadsWxModel::SetWindHeight( double dfHeight )
{
    dfWindHeight = dfHeight;
    return 0;
}

int NomadsWxModel::SetTempHeight( double dfHeight )
{
    dfTempHeight = dfHeight;
    return 0;
}

int NomadsWxModel::SetDimension( NomadsDimension eDim )
{
    eDimension = eDim;
    return 0;
}

int NomadsWxModel::SetWindType( NomadsWindType eType )
{
    eWindType = eType;
    return 0;
}

int NomadsWxModel::SetCloudType( NomadsCloudType eType )
{
    eCloudType = eType;
    return 0;
}

int NomadsWxModel::SetTempType( NomadsTempType eType )
{
    eTempType = eType;
    return 0;
}

int NomadsWxModel::SetUpdateInterval( int nInterval )
{
    nUpdateInterval = nInterval;
    return 0;
}

int NomadsWxModel::UpdateTimes( int bForce )
{
    //int nCurrentTime = 0;
    //if( nCurrentTime - nLastUpdateTime < nUpdateInterval && !bForce )
    if( bForce && FALSE )
    {
        CPLDebug( "NOMADS", "Skipping update, not enough time has passed " \
                            "since last update" );
        return 0;
    }
    std::string osUrl = osHost + osInventoryPath;
    char **papszDirList = NULL;
    papszDirList = ReadNomadsDir( osUrl.c_str() );
    if( papszDirList == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not connect to nomads server" );
        return 1;
    }

    int nCount = CSLCount( papszDirList );
    FilterModelList( papszDirList );

    nCount = CSLCount( papszDirList );
    long nMaxDate;
    long nMinDate;
    char *pszLast = GetDateSpan( papszDirList, &nMinDate, &nMaxDate );
    std::string osFirstUrl = FormFinalUrl( osUrl, pszLast );
    CPLDebug( "NOMADS", "Counting files in %s", osFirstUrl.c_str() );
    char **papszFileList = NULL;
    papszFileList = ReadNomadsDir( osFirstUrl );
    nCount = CSLCount( papszFileList );
    if( papszFileList == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not connect to nomads server" );
        CSLDestroy( papszDirList );
        CSLDestroy( papszFileList );
        return 1;
    }
    /*
    ** Search for the sub-model and forecast run times.
    */
    int nForecastRunHour;
    int nLastHour;
    int nForecastPreviousRunHour;
    FilterSubModelList( papszFileList, &nForecastRunHour, &nLastHour,
                        &nForecastPreviousRunHour );
    nCount = CSLCount( papszFileList );
    CPLDebug( "NOMADS", "Submodel filter left %d files.", nCount );

    if( nLastHour != GetLastForecastHour() && !IsDynamicTime() )
    {
        /*
        ** If it is the first run of the day, we have to step back a day.
        ** We need to add support for this, it is currently unsupported
        */
        if( nForecastRunHour == 0 )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not parse server data." );
            CSLDestroy( papszDirList );
            CSLDestroy( papszFileList );
            return 1;
        }

        /* Check if last run is valid */
        std::vector<int>anHours = GetForecastRunTimes();
        int bValid = FALSE;
        for( unsigned int i = 0; i < anHours.size(); i++ )
        {
            if( nForecastPreviousRunHour == anHours[i] )
            {
                bValid = TRUE;
                break;
            }
        }
        if( !bValid )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Forecast run time is invalid for this model" );
            CSLDestroy( papszDirList );
            CSLDestroy( papszFileList );
            return 1;
        }
        nForecastRunHour = nForecastPreviousRunHour;
    }

    osStartTime = CPLSPrintf( "%ldT0000", nMinDate );
    osEndTime = CPLSPrintf( "%ldT0000", nMaxDate );
    osEndTime = AddHours( AddHours( osEndTime, nForecastRunHour), nLastHour );
    CPLDebug( "NOMADS", "Last forecast hour: %d", nLastHour );
    CPLDebug( "NOMADS", "Last forecast run: %dZ", nForecastRunHour );
    CPLDebug( "NOMADS", "Start time: %s", osStartTime.c_str() );
    CPLDebug( "NOMADS", "End time:   %s", osEndTime.c_str() );

    CSLDestroy( papszDirList );
    CSLDestroy( papszFileList );
    return 0;
}

/**
 * \brief Read a Nomads inventory directory and return a list of folders or
 *        that match the model name.
 *
 * \param osUrl url to query
 * \return a list of strings that start with 'modelname.' to be freed by the
 *           caller using CSLDestroy()
 */
char ** NomadsWxModel::ReadNomadsDir( std::string osUrl )
{
    if( !EQUALN( osUrl.c_str(), "/vsicurl/", 9 ) )
    {
        osUrl = "/vsicurl/" + osUrl;
    }
    CPLDebug( "NOMADS", "Reading remote directory from: %s", osUrl.c_str() );
    char **papszFileList = NULL;
    papszFileList = VSIReadDir( osUrl.c_str() );
    if( papszFileList == NULL )
    {
        return NULL;
    }
    int nCount = CSLCount( papszFileList );
    CPLDebug( "NOMADS", "Found %d files/folders in folder: %s",
              nCount, osUrl.c_str() );
    return papszFileList;
}

/**
 * \brief Find the earliest and latest day from a file list of UTC date
 * strings.  The min/max dates are set as integers and return a pointer to the
 * max date string.
 *
 * \param papszFileList list to classify
 * \param [out] pnMinDate minimum date to set
 * \param [out] pnMaxDate maximum date to set
 * \return a pointer too the string that holds the max date for further parsing
 *         if need be.  This string belongs to the string list and should not
 *         be freed unless destroying the whole list.
 */
char * NomadsWxModel::GetDateSpan( char **papszFileList, long *pnMinDate,
                                   long *pnMaxDate )
{
    if( papszFileList == NULL )
    {
        return 0;
    }
    int nCount = CSLCount( papszFileList );
    std::string osDate;
    int nOffset = osModelShortName.size() + 1;
    long nDate;
    long nMinDate = LONG_MAX;
    long nMaxDate = LONG_MIN;
    int nIndex = 0;
    for( int i = 0; i < nCount; i++ )
    {
        osDate = papszFileList[i] + nOffset;
        nDate = atoi( osDate.c_str() );
        if( nDate > 0 )
        {
            if( nDate > nMaxDate )
            {
                nMaxDate = atoi( osDate.substr(0, 8).c_str() );
                nIndex = i;
            }
            if( nDate < nMinDate )
            {
                nMinDate = atoi( osDate.substr(0, 8).c_str() );
            }
        }
    }
    if( pnMinDate != NULL )
    {
        *pnMinDate = nMinDate;
    }
    if( pnMaxDate != NULL )
    {
        *pnMaxDate = nMaxDate;
    }
    return papszFileList[nIndex];
}

int NomadsWxModel::FilterModelList( char **papszFileList )
{
    int nCount = CSLCount( papszFileList );
    if( nCount < 1 )
    {
        CPLDebug( "NOMADS", "FilterModelList() got an empty list" );
        return 1;
    }
    std::string osFilter = osModelShortName + ".";
    int i = 0;
    while( TRUE )
    {
        if( !EQUALN( papszFileList[i], osFilter.c_str(), osFilter.size() ) )
        {
            papszFileList = CSLRemoveStrings( papszFileList, i, 1, NULL );
            nCount = CSLCount( papszFileList );
            i = 0;
        }
        else
        {
            i++;
        }
        if( i == nCount || nCount == 0 )
            break;
    }
    return 0;
}

int NomadsWxModel::FilterSubModelList( char **papszFileList, int *pnForecastRun,
                                       int *pnLastForecast, int *pnSecondToLastRun )
{
    int nCount = CSLCount( papszFileList );
    if( nCount < 1 )
    {
        CPLDebug( "NOMADS", "FilterModelList() got an empty list" );
        return 1;
    }
    std::string osFilter = GetModelFilter();
    std::string osSubFilter = GetSubModelFilter();
    int nSubOffset = ( osFilter + ".t00z." ).size();
    int nRunOffset = ( osFilter + ".t" ).size();
    int nHourOffset = ( osFilter + ".t00z." + osSubFilter ).size();
    int i = 0;
    int nRun = INT_MIN;
    int nHour = INT_MIN;
    int nPenultimateRun = INT_MIN;
    int bInitPenultimateRun = FALSE;
    while( TRUE )
    {
        if( strlen( papszFileList[i] ) < (unsigned)nHourOffset ||
            !EQUALN( papszFileList[i] + nSubOffset,
                     osSubFilter.c_str(), osSubFilter.size() ) )
        {
            papszFileList = CSLRemoveStrings( papszFileList, i, 1, NULL );
            nCount = CSLCount( papszFileList );
            i = 0;
        }
        else
        {
            if( atoi( papszFileList[i] + nRunOffset ) > nRun )
            {
                nPenultimateRun = nRun;
                nRun = atoi( papszFileList[i] + nRunOffset );
                /*
                ** In some cases, there is only one forecast hour (gfs), so
                ** there is no chance for nPenultimateRun to get set.  We set
                ** it here on the first pass.
                */
                if( !bInitPenultimateRun )
                {
                    nPenultimateRun = nRun;
                    bInitPenultimateRun = TRUE;
                }
            }
            if( atoi( papszFileList[i] + nHourOffset ) > nHour )
            {
                nHour = atoi( papszFileList[i] + nHourOffset );
            }
            i++;
        }
        if( i == nCount || nCount == 0 )
            break;
    }
    if( pnForecastRun != NULL )
    {
        *pnForecastRun = nRun;
    }
    if( pnSecondToLastRun != NULL)
    {
        *pnSecondToLastRun = nPenultimateRun;
    }
    if( pnLastForecast != NULL )
    {
        *pnLastForecast = nHour;
    }
    return 0;
}

/**
 * \brief Generate a list of files to be downloaded for a given model.
 *
 * \param osStart the earliest time data is needed.
 * \param osEnd the latest time data is needed.
 * \return a list of filenames
 */

#include <iostream>
std::vector<std::string> GetForecastRunTimess()
{
    std::vector<std::string> result;
    result.push_back("00Z");
    result.push_back("06Z");
    result.push_back("12Z");
    result.push_back("18Z");
    return result;
}

std::vector<std::string> GetForecastHoursSequence()
{
    std::stringstream ss;
    std::vector<std::string> result;
    for(int i=0;i<=36;i++)
    {
        if (i < 10)
        {
            ss << "0";
        }
        ss << i;
        result.push_back(ss.str());
        ss.str("");
    }
    for(int i=39;i<=84;i=i+3)
    {
        ss << i;
        result.push_back(ss.str());
        ss.str("");
    }
    return result;
}

std::string GetLatestRun()
{
    return "20130812T1200";
}

/**
 * \brief Generate a list of timesteps to download from the server.
 *
 * Take in a start time and an end time as std::string and return a 
 * sorted list of timesteps in the format ForecastRun:ForecastHour.
 *
 * \param startTime the starting date/time
 * \param endTime the ending date/time
 * \return a vector<string> containing all intervening timesteps
 */
//TODO Switch to char** return type.
//TODO Create GetForecastRunTimes(), GetForecastHoursSequence(), GetLatestRun().

std::vector<std::string> NomadsWxModel::GenerateDownloadList(std::string startTime,
                                                           std::string endTime)
{
    /* XXX: KYLE */

    /* XXX: KYLE */
    std::stringstream ss;
    std::vector<std::string> result;
    std::vector<int> oForecastRunTimes = GetForecastRunTimes();
    std::vector<std::string> forecastRunTimes = GetForecastRunTimess();
    std::vector<std::string> forecastHourSequence = GetForecastHoursSequence();
    int nRunsPerDay = forecastRunTimes.size();
    int nRunInterval = 24/nRunsPerDay;
    std::string lastRun = GetLatestRun();
    lastRun = AddHours(lastRun, nRunInterval);

    int i = 0;
    int j = 1;
    int currentSteps = 1;
    UNREFERENCED_PARAM( currentSteps );
    std::string currentTime = startTime;

    char** papszTemp = CSLTokenizeString2( endTime.c_str(), "T", CSLT_STRIPLEADSPACES | CSLT_STRIPENDSPACES );
    if( CSLCount( papszTemp ) != 2 )
    {
        CPLError( CE_Warning, CPLE_AppDefined, "Could not parse date/time" );
    }
    int nEndDate = atoi( papszTemp[0] );                       //20130811 first time through
    int nEndTime = atoi( papszTemp[1] )/100;                   //    0900 first time through
    bool boolEnd = false;
    int nCurrentDate;
    int nCurrentTime;

    while(boolEnd == false)
    {
        if(AddHours(startTime, i) != lastRun) // We have the run data for this time step
        {
            currentTime = AddHours(startTime, i);
            char** papszTemp = CSLTokenizeString2( currentTime.c_str(), "T", CSLT_STRIPLEADSPACES | CSLT_STRIPENDSPACES );
            if( CSLCount( papszTemp ) != 2 )
            {
                CPLError( CE_Warning, CPLE_AppDefined, "Could not parse date/time" );
            }
            nCurrentDate = atoi( papszTemp[0] );                       //20130811 first time through
            nCurrentTime = atoi( papszTemp[1] )/100;                   //    0900 first time through
            if( nCurrentDate > nEndDate && nCurrentTime > nEndTime)
            {
                boolEnd = true;
            }
            std::string run = forecastRunTimes[(nCurrentTime/nRunInterval)]; //   "06Z" first time through
            std::string time = forecastHourSequence[nCurrentTime];           //    "09" first time through
            ss << nCurrentDate << run << ":" << time;
            std::string temp = ss.str();
            ss.str("");
            result.push_back(temp);
            i++;
        }
        else // We are past the existing run data and must use data from previous dates
        {
            currentTime = "20130812T0000";
            char** papszTemp = CSLTokenizeString2( AddHours(currentTime,atoi(forecastHourSequence[nCurrentTime + j].c_str())).c_str(), "T", CSLT_STRIPLEADSPACES | CSLT_STRIPENDSPACES );
            if( CSLCount( papszTemp ) != 2 )
            {
                CSLDestroy( papszTemp );
                CPLError( CE_Warning, CPLE_AppDefined, "Could not parse date/time" );
            }
            int nCheckDate = atoi( papszTemp[0] );
            int nCheckTime = atoi( papszTemp[1] )/100;
            CSLDestroy( papszTemp );
            if( nCheckDate >= nEndDate && nCheckTime >= nEndTime)
            {
                boolEnd = true;
            }
            std::string run = forecastRunTimes[(nCurrentTime/nRunInterval)];
            std::string time = forecastHourSequence[nCurrentTime + j]; // Add j here instead of modifying date
            currentSteps = atoi( time.c_str() );
            ss << nCurrentDate << run << ":" << time;
            std::string temp = ss.str();
            ss.str("");
            result.push_back(temp);
            j++;
        }
    }
    return result;
}

std::string NomadsWxModel::FormFinalUrl( std::string osUrl, const char *pszLastFolder )
{
    std::string osFinalUrl;
    osFinalUrl = osUrl + std::string( pszLastFolder );
    return osFinalUrl;
}

#ifdef NINJA_BUILD_TESTING

int NomadsWxModel::TestUpdateTimes()
{
    return UpdateTimes();
}

int NomadsWxModel::TestGenerateDownloadList()
{
    std::vector<std::string> expected;
    expected.push_back("2013081106Z:09");
    expected.push_back("2013081106Z:10");
    expected.push_back("2013081106Z:11");
    expected.push_back("2013081112Z:12");
    expected.push_back("2013081112Z:13");
    expected.push_back("2013081112Z:14");
    expected.push_back("2013081112Z:15");
    expected.push_back("2013081112Z:16");
    expected.push_back("2013081112Z:17");
    expected.push_back("2013081118Z:18");
    expected.push_back("2013081118Z:19");
    expected.push_back("2013081118Z:20");
    expected.push_back("2013081118Z:21");
    expected.push_back("2013081118Z:22");
    expected.push_back("2013081118Z:23");
    expected.push_back("2013081218Z:24");
    expected.push_back("2013081200Z:01");
    expected.push_back("2013081200Z:02");
    expected.push_back("2013081200Z:03");
    expected.push_back("2013081200Z:04");
    expected.push_back("2013081200Z:05");
    expected.push_back("2013081206Z:06");
    expected.push_back("2013081206Z:07");
    expected.push_back("2013081206Z:08");
    expected.push_back("2013081206Z:09");
    expected.push_back("2013081206Z:10");
    expected.push_back("2013081206Z:11");
    expected.push_back("2013081212Z:12");
    expected.push_back("2013081212Z:13");
    expected.push_back("2013081212Z:14");
    expected.push_back("2013081212Z:15");
    expected.push_back("2013081212Z:16");
    expected.push_back("2013081212Z:17");
    expected.push_back("2013081212Z:18");
    expected.push_back("2013081212Z:19");
    expected.push_back("2013081212Z:20");
    expected.push_back("2013081212Z:21");
    expected.push_back("2013081212Z:22");
    expected.push_back("2013081212Z:23");
    expected.push_back("2013081212Z:24");
    expected.push_back("2013081212Z:25");
    expected.push_back("2013081212Z:26");
    expected.push_back("2013081212Z:27");
    expected.push_back("2013081212Z:28");
    expected.push_back("2013081212Z:29");
    expected.push_back("2013081212Z:30");
    expected.push_back("2013081212Z:31");
    expected.push_back("2013081212Z:32");
    expected.push_back("2013081212Z:33");
    expected.push_back("2013081212Z:34");
    expected.push_back("2013081212Z:35");
    expected.push_back("2013081212Z:36");
    expected.push_back("2013081212Z:39");
    expected.push_back("2013081212Z:42");
    expected.push_back("2013081212Z:45");
    expected.push_back("2013081212Z:48");
    expected.push_back("2013081212Z:51");
    expected.push_back("2013081212Z:54");
    expected.push_back("2013081212Z:57");
    expected.push_back("2013081212Z:60");
    expected.push_back("2013081212Z:63");
    expected.push_back("2013081212Z:66");
    std::vector<std::string> times = GenerateDownloadList("20130811T0900", "20130814T1800");
    if( expected.size() != times.size() )
    {
       return 1;
    }
    for( unsigned int i = 0;i < expected.size(); i++ )
    {
         if(expected[i] != times[i])
         {
             return 1;
         }
         i++;
    }
    return 0;
}

int NomadsWxModel::TestUpdateTimesAndBuildFileList()
{
    UpdateTimes( TRUE );
    std::vector<std::string> aosFiles =
        GenerateDownloadList( AddHours( osEndTime, -6 ), osEndTime );

    if( aosFiles.size() < 1 ) 
        return 1;
    return 0;
}

int NomadsWxModel::TestAddHours()
{
    std::string osStart = "20130101T0000";
    const char *pszFormat = "20130101T%04d";
    std::string osTest;
    for( int i = 0; i < 24; i++ )
    {
        if( AddHours( osStart, i ) != CPLSPrintf( pszFormat, i * 100 ) )
            return 1;
    }
    /* Over a day */
    if( AddHours( osStart, 24 ) != "20130102T0000" )
        return 1;
    /* Around a month boundary */
    osTest = AddHours( osStart, 24 * 30 );
    if( osTest != "20130131T0000" )
        return 1;
    osTest = AddHours( osStart, 24 * 31 );
    if( osTest != "20130201T0000" )
        return 1;
    /* Febuary leap/non-leap */
    osStart = "20130227T2300";
    osTest = AddHours( osStart, 1 );
    if( osTest != "20130228T0000" )
        return 1;
    osStart = "20130228T2300";
    osTest = AddHours( osStart, 1 );
    if( osTest != "20130301T0000" )
        return 1;
    osStart = "20120228T2300";
    osTest = AddHours( osStart, 1 );
    if( osTest != "20120229T0000" )
        return 1;
    /* Over normal month */
    osStart = "20131031T0000";
    osTest = AddHours( osStart, 24 );
    if( osTest != "20131101T0000" )
        return 1;
    /* Over year */
    osStart = "20131231T2300";
    osTest = AddHours( osStart, 1 );
    if( osTest != "20140101T0000" )
        return 1;
    return 0;
}

#endif /* NINJA_BUILD_TESTING */

#endif /* WITH_NOMADS_SUPPORT */

