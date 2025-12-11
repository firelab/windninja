/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for status and error reporting
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

#ifndef NINJACOM_H
#define NINJACOM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string.h>

#define NINJA_MSG_SIZE 1000

#include "callbackFunctions.h"

class ninjaComClass
{
public:
    ninjaComClass();
    ~ninjaComClass();
    double progressWeight;

    typedef enum
    {
        ninjaNone,
        ninjaDebug,
        ninjaSolverProgress,
        ninjaOuterIterProgress,
        ninjaWarning,
        ninjaFailure,
        ninjaFatal
    } msgType;

    bool printLastMsg;
    char lastMsg[NINJA_MSG_SIZE];  // storage of the last message
    int runNumber;  // run number of the simulation. Can turn this back into a pointer to the value in the WindNinjaInputs class, if the values start to differ

    bool printProgressFunc;
    ProgressFunc pfnProgress;
    void *pProgressUser;

    bool printLogFile;
    FILE*     fpLog;
    FILE*     fpErr;
    FILE* multiStream;

    int errorCount; //running error count
    int nMaxErrors; //max number of errors to report
    bool printMaxErrors;  //flag to determine whether to keep printing error messages past when errorCount exceeds nMaxErrors
    bool printSolverProgress;  //flag specifying where normal solver progress should be printed (matching will still be printed)
    bool printRunNumber;  //flag to determine if thread number should be printed at beginning of messages

    //methods

    void set_progressFunc(ProgressFunc func, void *pUser);

    void noSolverProgress();

    void ninjaCom(msgType eMsg, const char *fmt, ...);
    void ninjaComV(msgType, const char *, va_list);
    //void initializeNinjaCom(char *LastMsg, int RunNumber);

    void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};

#endif //NINJACOM_H
