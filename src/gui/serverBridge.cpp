 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Connects the Qt GUI with the ninjastorm server
 * Author:   Mason Willman <mason.willman@usda.gov>
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

#include "serverBridge.h"

ServerBridge::ServerBridge() {}

/*
** Check for version updates, or messages from the server.
*/
void ServerBridge::checkMessages(void)
{
    QMessageBox mbox;
    char *papszMsg = NinjaQueryServerMessages(true);
    if (papszMsg != NULL) {
        if (strcmp(papszMsg, "TRUE\n") == 0)
        {
            mbox.setText("There is a fatal flaw in Windninja, it must close.");
            mbox.exec();
            delete[] papszMsg;
            abort();
        }
        else
        {
            char *papszMsg = NinjaQueryServerMessages(false);
            if (papszMsg != NULL)
            {
                mbox.setTextFormat(Qt::RichText);
                mbox.setText(papszMsg);
                mbox.setTextInteractionFlags(Qt::TextBrowserInteraction);
                mbox.setStandardButtons(QMessageBox::Ok);
                mbox.setWindowModality(Qt::ApplicationModal);
                mbox.exec();
                delete[] papszMsg;
            }
        }
    }
}

/*
** Query the ninjastorm.firelab.org/sqlitetest/messages.txt and ask for the most up to date version.
** There are three current values:
**
** VERSION -> a semantic version string, comparable with strcmp()
** MESSAGE -> One or more messages to display to the user
** ABORT   -> There is a fundamental problem with windninja, and it should call
**            abort() after displaying a message.
*/
bool ServerBridge::NinjaCheckVersions(char * mostrecentversion, char * localversion)
{
    char comparemostrecentversion[256];
    char comparelocalversion[256];
    int mostrecentversionIndex = 0;
    int localversionIndex = 0;
    while (*mostrecentversion) {
        if (*mostrecentversion != '.') {
            comparemostrecentversion[mostrecentversionIndex++] = *mostrecentversion;
        }
        mostrecentversion++;
    }
    comparemostrecentversion[mostrecentversionIndex] = '\0';

    while (*localversion) {
        if (*localversion != '.') {
            comparelocalversion[localversionIndex++] = *localversion;
        }
        localversion++;
    }

    comparelocalversion[localversionIndex] = '\0';
    return strcmp(comparemostrecentversion, comparelocalversion) == 0;

}

char * ServerBridge::NinjaQueryServerMessages(bool checkAbort)
{
    CPLSetConfigOption("GDAL_HTTP_UNSAFESSL", "YES");
    CPLSetConfigOption("GDAL_HTTP_TIMEOUT", "5");
    const char* url = "https://ninjastorm.firelab.org/sqlitetest/messages.txt";
    CPLHTTPResult *poResult = CPLHTTPFetch(url, NULL);
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", NULL );
    if( !poResult || poResult->nStatus != 0 || poResult->nDataLen == 0 )
    {
        CPLDebug( "NINJA", "Failed to reach the ninjastorm server." );
        return NULL;
    }

    const char* pszTextContent = reinterpret_cast<const char*>(poResult->pabyData);
    std::vector<std::string> messages;
    std::istringstream iss(pszTextContent);
    std::string message;

    // Read all lines into the vector
    while (std::getline(iss, message))
    {
        messages.push_back(message);
    }

    // Process all lines except the last two
    std::ostringstream oss;
    if (checkAbort)
    {
        for (size_t i = 0; i < messages.size(); ++i)
        {
            if (i == messages.size()-1)
            {
                oss << messages[i] << "\n";
            }
        }
    }
    else
    {
        bool versionisuptodate = NinjaCheckVersions(const_cast<char*>(messages[1].c_str()), const_cast<char*>(NINJA_VERSION_STRING));
        if (!versionisuptodate)
        {
            oss << "You are using an outdated WindNinja version, please update to version: " << messages[1] << "<br><br>";

            oss << "Windows: Download the new " << messages[1]
                << " installer <a href=\"https://research.fs.usda.gov/firelab/products/dataandtools/windninja\">here</a><br><br>";

            oss << "Linux: See the <a href=\"https://github.com/firelab/windninja/tree/" << messages[1] << "\">" << messages[1] << "</a> tag in our GitHub repo<br>";

        }
        if (messages[4].empty() == false)
        {
            for (size_t i = 3; i < messages.size() - 2; ++i)
            {
                if (!messages[i].empty())
                {
                    oss << messages[i] << "\n";
                }
            }
        }
        if (messages[4].empty() && versionisuptodate) {
            return NULL;
        }
    }

    std::string resultingmessage = oss.str();
    char* returnString = new char[resultingmessage.length() + 1];
    std::strcpy(returnString, resultingmessage.c_str());
    CPLHTTPDestroyResult(poResult);
    return returnString;

    return NULL;
}
