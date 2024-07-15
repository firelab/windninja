/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Global initialization
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
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

#include "ninja_init.h"
#include <cstring>
#include <string>
#include <iostream>
#include <ctime>
#include "cpl_http.h"  
#include <stdarg.h>
#include <stdarg.h>
#include <sstream>
#include "ninja_version.h"

boost::local_time::tz_database globalTimeZoneDB;

/*
** Query the ninjastorm.firelab.org/sqlitetest/messages.txt and ask for the most up to date version.
** There are three current values:
**
** VERSION -> a semantic version string, comparable with strcmp()
** MESSAGE -> One or more messages to display to the user
** ABORT   -> There is a fundamental problem with windninja, and it should call
**            abort() after displaying a message.
*/

bool rawVersion(char * src, char * dest) {
    bool same = false;
    char src1[256]; 
    char dest1[256]; 
    int srcIndex = 0;
    int destIndex = 0;
    while (*src) {
        if (*src != '.') {
            src1[srcIndex++] = *src;
        }
        src++;
    }
    src1[srcIndex] = '\0'; 

    while (*dest) {
        if (*dest != '.') {
            dest1[destIndex++] = *dest;
        }
        dest++;
    }
    dest1[destIndex] = '\0';

    return strcmp(src1, dest1) == 0;

}

char * NinjaQueryServerMessages(bool checkAbort) { 
    const char* url = "https://ninjastorm.firelab.org/sqlitetest/messages.txt";

    CPLHTTPResult *poResult = CPLHTTPFetch(url, NULL);

    if (poResult != NULL) {
        const char* pszTextContent = reinterpret_cast<const char*>(poResult->pabyData);
         std::vector<std::string> lines;
        std::istringstream iss(pszTextContent);
        std::string line;
        int lineCount = 0;

        // Read all lines into the vector
        while (std::getline(iss, line)) {
            lines.push_back(line);
        }

        // Process all lines except the last two
        std::ostringstream oss;
        if (checkAbort) {
            for (size_t i = 0; i < lines.size(); ++i) {
               if (i == lines.size()-1) { // check final line 
                    oss << lines[i] << "\n";
                }
             }
        }
        else {
            for (size_t i = 0; i < lines.size(); ++i) {
            if (i == 1) {  
                if (!rawVersion(const_cast<char*>(lines[i].c_str()), const_cast<char*>(NINJA_VERSION_STRING))) {
                    oss << "You are using an outdated Ninja version, please update to version: " << lines[i] << "\n";
                } else {
                    oss << lines[i] << "\n";
                }
            } else if (i < lines.size() - 2) {  
                oss << lines[i] << "\n";
            }
            }
        }

        std::string result = oss.str();
        // return as char * 
        char* returnString = new char[result.length() + 1];
        std::strcpy(returnString, result.c_str());

        // Clean up
        CPLHTTPDestroyResult(poResult);

        return returnString;
    }
    return NULL;

}




void NinjaCheckThreddsData( void *rc )
{
    int *r;
    if( rc )
        r = (int*)rc;
    std::string p = FindDataPath( "thredds.csv" );
    if( p == "" )
    {
        *r = 1;
        return;
    }
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", "5" );
    CPLHTTPResult *poResult;
    poResult = CPLHTTPFetch( "http://windninja.org/cgi-bin/ninjavisit?thredds=1", NULL );
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", NULL );
    if( !poResult || poResult->nStatus != 0 || poResult->nDataLen == 0 )
    {
        CPLDebug( "NINJA", "Failed to download thredds file." );
        *r = 1;
        return;
    }
    VSILFILE *fout;
    fout = VSIFOpenL( p.c_str(), "wb" );
    if( !fout )
    {
        *r = 1;
        CPLHTTPDestroyResult( poResult );
        return;
    }
    if( !VSIFWriteL( poResult->pabyData, poResult->nDataLen, 1, fout ) )
        *r = 1;
    else
        *r = 0;
    VSIFCloseL( fout );
    CPLHTTPDestroyResult( poResult );
    return;
}
/*
** Initialize global singletons and environments.
*/
int NinjaInitialize(const char *pszGdalData, const char *pszWindNinjaData)
{
    //set GDAL_DATA and WINDNINJA_DATA
    GDALAllRegister();
    OGRRegisterAll();    


    if(!CPLCheckForFile(CPLStrdup(CPLFormFilename(CPLStrdup(pszGdalData), "gdalicon.png", NULL)), NULL))
    {
        CPLDebug("WINDNINJA", "Invalid path for GDAL_DATA: %s", pszGdalData);
        return 2;
    }
    if(!CPLCheckForFile(CPLStrdup(CPLFormFilename(CPLStrdup(pszWindNinjaData), "date_time_zonespec.csv", NULL)), NULL))
    {
        CPLDebug("WINDNINJA", "Invalid path for WINDNINJA_DATA: %s", pszWindNinjaData);
        return 2; 
    }

    CPLDebug( "WINDNINJA", "Setting GDAL_DATA:%s", pszGdalData );
    CPLSetConfigOption( "GDAL_DATA", pszGdalData );

    CPLDebug( "WINDNINJA", "Setting WINDNINJA_DATA:%s", pszWindNinjaData );
    CPLSetConfigOption( "WINDNINJA_DATA", pszWindNinjaData );

    globalTimeZoneDB.load_from_file(FindDataPath("date_time_zonespec.csv"));

    return 0;
}

/*
** Initialize global singletons and environments.
*/

int NinjaInitialize(const char* typeofrun) {


    GDALAllRegister();
    OGRRegisterAll();
    /*
    ** Silence warnings and errors in initialize.  Sometimes we can't dial out,
    ** but that doesn't mean we are in trouble.
    */
    CPLPushErrorHandler(CPLQuietErrorHandler);
	int rc = 0; 
    /*
    ** Setting the CURL_CA_BUNDLE variable through GDAL doesn't seem to work,
    ** but could be investigated in the future.  CURL_CA_BUNDLE can only be set in GDAL
    ** >2.1.2. Test with CPL_CURL_VERBOSE.  For #231.
    **
    ** For now, just skip the SSL verification with GDAL_HTTP_UNSAFESSL.
    */
    CPLSetConfigOption( "GDAL_HTTP_UNSAFESSL", "YES");

    if (strcmp(typeofrun, "") != 0) {

    time_t now = time(0);

        // convert now to tm struct for UTC
        tm *gmtm = gmtime(&now);
        char* dt = asctime(gmtm);
        std::string cpp_string(dt);


        std::string url = "https://ninjastorm.firelab.org/sqlitetest/?time=";
        cpp_string.erase(std::remove_if(cpp_string.begin(), cpp_string.end(), ::isspace),
        cpp_string.end());


        std::string full = url + cpp_string + "&runtype=" + typeofrun;


        const char *charStr = full.data();

        CPLHTTPResult *poResult;
        CPLSetConfigOption("GDAL_HTTP_UNSAFESSL", "YES");
        char **papszOptions = NULL;

        // Fetch the URL with custom headers
        poResult = CPLHTTPFetch(charStr, papszOptions);

        if (poResult) {
                CPLHTTPDestroyResult(poResult);

        }
    }
} 


#ifdef WIN32
    CPLDebug( "WINDNINJA", "Setting GDAL_DATA..." );
    std::string osGdalData;
    osGdalData = FindDataPath( "gdal-data/data/gdalicon.png" );
    const char *pszGdalData = CPLGetPath( osGdalData.c_str() );
    CPLDebug( "WINDNINJA", "Setting GDAL_DATA:%s", pszGdalData );
    CPLSetConfigOption( "GDAL_DATA", pszGdalData );

#if defined(FIRELAB_PACKAGE)
    char szDriverPath[MAX_PATH+1];
    rc = CPLGetExecPath( szDriverPath, MAX_PATH+1);
    const char *pszPlugins = CPLSPrintf("%s/gdalplugins", CPLGetPath(szDriverPath));
    CPLDebug("WINDNINJA", "Setting GDAL_DRIVER_PATH: %s", pszPlugins);

    CPLSetConfigOption("GDAL_DRIVER_PATH", pszPlugins);

#endif /* defined(FIRELAB_PACKAGE) */

#if defined(NINJAFOAM) && defined(FIRELAB_PACKAGE)
    char *pszExecPath;
    const char *pszFoamLibPath = "platforms/linux64mingw-w64DPOpt/lib";
    const char *pszFoamBinPath = "platforms/linux64mingw-w64DPOpt/bin";
    const char *pszTmp;

    pszExecPath = (char*)CPLMalloc( 8192 );
    rc = CPLGetExecPath( pszExecPath, 8192 );
    /*
    ** Set the WM_PROJECT_DIR.  This should point to the installation, with etc
    ** and platforms folders
    */
    rc = _putenv( CPLSPrintf("WM_PROJECT_DIR=%s", CPLGetPath( pszExecPath ) ) );

    /*
    ** Set the PATH variable to point to the lib and bin folders in the open
    ** foam installation.
    */
    pszTmp = CPLSPrintf( "PATH=%s;%s;%PATH%",
                         CPLFormFilename( CPLGetPath( pszExecPath ), pszFoamBinPath, NULL ),
                         CPLFormFilename( CPLGetPath( pszExecPath ), pszFoamLibPath, NULL ) );
    rc = _putenv( pszTmp );
    CPLFree( (void*)pszExecPath );
#endif /* defined(NINJAFOAM) && defined(FIRELAB_PACKAGE)*/


#endif




/*
** Set windninja data if it isn't set.
*/
if (!CSLTestBoolean(CPLGetConfigOption("WINDNINJA_DATA", "FALSE"))) {
    std::string osDataPath;
    osDataPath = FindDataPath("tz_world.zip");
    if (osDataPath != "") {
        CPLSetConfigOption("WINDNINJA_DATA", CPLGetPath(osDataPath.c_str()));
    }
}


    globalTimeZoneDB.load_from_file(FindDataPath("date_time_zonespec.csv"));
    CPLPopErrorHandler();
    return 0;
}
