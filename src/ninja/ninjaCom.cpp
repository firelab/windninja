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

#include "ninjaCom.h"

/**
* @brief Default constructor.
*
*/
ninjaComClass::ninjaComClass()
{
    printRunNumber = true;
    runNumber = -9999;

    printMaxErrors = false;
    errorCount = 0;
    nMaxErrors = 10;

    progressWeight = 1.0;

    printSolverProgress = true;

    printLastMsg = false;
    lastMsg[0] = '\0';

    printToMsgHandler = false;
    pMsgHandler = nullptr;
    pMsgUser = nullptr;

    printLogFile = false;
    fpLog = stdout;
    fpErr = stderr;
    multiStream = NULL;
}

/**
* @brief Destructor.
*
*/
ninjaComClass::~ninjaComClass()
{

}

/**
* @brief Copy constructor.
*
* @param a ninjaComClass object to be copied.
*/
ninjaComClass::ninjaComClass(const ninjaComClass& A)
{
    printRunNumber = A.printRunNumber;
    runNumber = A.runNumber;

    printMaxErrors = A.printMaxErrors;
    errorCount = A.errorCount;
    nMaxErrors = A.nMaxErrors;

    progressWeight = A.progressWeight;

    printSolverProgress = A.printSolverProgress;

    printLastMsg = A.printLastMsg;
    strcpy( lastMsg, A.lastMsg );

    printToMsgHandler = A.printToMsgHandler;
    pMsgHandler = A.pMsgHandler;
    pMsgUser = A.pMsgUser;

    printLogFile = A.printLogFile;
    fpLog = A.fpLog;
    fpErr = A.fpErr;
    multiStream = A.multiStream;
}

/**
* @brief Equals operator.
*
* @param a ninjaComClass object to set equal to.
* @return A reference to a ninjaComClass object equal to the one on the right-hand side.
*/
ninjaComClass& ninjaComClass::operator=(const ninjaComClass &A)
{
    if(&A != this)
    {
        printRunNumber = A.printRunNumber;
        runNumber = A.runNumber;

        printMaxErrors = A.printMaxErrors;
        errorCount = A.errorCount;
        nMaxErrors = A.nMaxErrors;

        progressWeight = A.progressWeight;

        printSolverProgress = A.printSolverProgress;

        printLastMsg = A.printLastMsg;
        strcpy( lastMsg, A.lastMsg );

        printToMsgHandler = A.printToMsgHandler;
        pMsgHandler = A.pMsgHandler;
        pMsgUser = A.pMsgUser;

        printLogFile = A.printLogFile;
        fpLog = A.fpLog;
        fpErr = A.fpErr;
        multiStream = A.multiStream;
    }
    return *this;
}

void ninjaComClass::set_messageHandler(ninjaComMessageHandler pMessageHandler, void *pUser)
{
    if( pMessageHandler == NULL || pUser == NULL )
    {
        fprintf(stderr, "CRITICAL: ninjaComClass::set_messageHandler(), input ninjaComMessageHandler pMsgHandler and pMsgUser are NULL\n");
        fflush(stderr);
        return;
    }

    printToMsgHandler = true;
    pMsgHandler = pMessageHandler;
    pMsgUser = pUser;
}

/**
 * Function to turn off solver progress messages.
 * Note that "matching" progress messages will still
 * be sent if it is a point initialization run.
 */
void ninjaComClass::noSolverProgress()
{
    printSolverProgress = false;
}

/**
* Communication function used to pass messages from the program to the ninjaComDispatchMessage() function,
* parsing the printf() %f, %d, %s, etc style syntax of the message into a fully built string.
* @param eMsgType Type of message to be passed. See msgType for available types.
* @param fmt Message to be passed, using string formatting (like a printf() statement).
*/
void ninjaComClass::ninjaCom(msgType eMsgType, const char *fmt, ...)
{
    va_list args;

    // Expand the error message
    va_start(args, fmt);
    ninjaComV(eMsgType, fmt, args);
    va_end(args);
}

/**
* This is an intermediate function that parses the args...?
* @param eMsgType Type of message to be passed. See msgType for available types.
* @param fmt Message to be passed, using string formatting (like a printf() statement).
* @param args Arguments list for fmt.
*/
void ninjaComClass::ninjaComV(msgType eMsgType, const char *fmt, va_list args)
{
    char ninjaMsg[NINJA_MSG_SIZE] = "";
    vsnprintf(ninjaMsg, NINJA_MSG_SIZE-2, fmt, args);

    ninjaComDispatchMessage(eMsgType, ninjaMsg);
}

/**
* Communication dispatcher for WindNinja simulations. Takes an input message and prints/passes it.
* The place the message is printed or passed to (or if the message is ignored) depends on the input message type,
* whether a ninjaComMessageHandler and/or a multi-stream FILE and/or various FILE streams are set,
* and other pre-set ninjaCom settings
*
* @param eMsgType Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed. Comes from ninjaComV and ninjaCom.
*/
void ninjaComClass::ninjaComDispatchMessage(msgType eMsgType, const char *ninjaComMsg)
{
    char msg[NINJA_MSG_SIZE];  // Declare a character array to store the result of sprintf, for printing
    char runPartMsg[10];  // this SHOULD be enough for the "Run %d:" part of the msg

    if( printToMsgHandler == false || multiStream != NULL )
    {
        if( runNumber == -9999 )
        {
            return;
        }
    }

    if( printLogFile == true )
    {
        fpLog = fopen("ninja.log", "w+");
        if( fpLog == NULL )
        {
            return;
        }
    }

    // for now, just send fpErr/stderr to stdout
    //fpErr = stderr;
    fpErr = stdout;

    // for now, always assume printRunNumber has been already been set ahead of time, where the default value is always true

    // for now, always assume printMaxErrors has been already been set ahead of time, where the default value is always false, to print all errors

    runPartMsg[0] = '\0';
    if( printRunNumber == true )
    {
        sprintf( runPartMsg, "Run %d: ", runNumber );
    }

    if( printMaxErrors == true )
    {
        if( eMsgType == ninjaFailure || eMsgType == ninjaFatal )
        {
            errorCount++;
            if( errorCount > nMaxErrors && nMaxErrors > 0 )
            {
                if( errorCount == nMaxErrors+1 )
                {
                    sprintf( msg, "%sMore than %d errors have been reported. "
                                  "No more will be reported from now on.\n",
                                  runPartMsg, nMaxErrors );

                    fprintf(fpErr, "%s", msg);

                    if( multiStream != NULL )
                    {
                        fprintf(multiStream, "%s", msg);
                        fflush(multiStream);
                    }

                    if( printToMsgHandler == true )
                    {
                        pMsgHandler(msg, pMsgUser);
                    }
                }
                return;
            }
        } // if( eMsgType == ninjaFailure || eMsgType == ninjaFatal )
    } // if( printMaxErrors == true )

    if(eMsgType == ninjaNone)                       //None
    {
        sprintf( msg, "%s%s\n", runPartMsg, ninjaComMsg );

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printToMsgHandler == true )
        {
            pMsgHandler(msg, pMsgUser);
        }
    }
    #ifdef NINJA_DEBUG
    else if(eMsgType == ninjaDebug)                 //Debug
    {
        sprintf( msg, "%s%s\n", runPartMsg, ninjaComMsg);

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printToMsgHandler == true )
        {
            pMsgHandler(msg, pMsgUser);
        }
    }
    #endif //NINJA_DEBUG
    else if(eMsgType == ninjaSolverProgress)        //Solver progress (%complete)
    {
        if(printSolverProgress)
        {
            sprintf( msg, "%sSolver: %d%% complete\n", runPartMsg, atoi(ninjaComMsg));

            fprintf(fpLog, "%s", msg);

            if( multiStream != NULL )
            {
                fprintf(multiStream, "%s", msg);
                fflush(multiStream);
            }

            if( printToMsgHandler == true )
            {
                pMsgHandler(msg, pMsgUser);
            }
        }
    }
    else if(eMsgType == ninjaOuterIterProgress)     //Solver progress for outer matching iterations (%complete)
    {
        sprintf( msg, "%sSolver (matching): %d%% complete\n", runPartMsg, atoi(ninjaComMsg));

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printToMsgHandler == true )
        {
            pMsgHandler(msg, pMsgUser);
        }
    }
    else if(eMsgType == ninjaWarning)               //Warnings
    {
        sprintf( msg, "%sWarning: %s\n", runPartMsg, ninjaComMsg);

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printToMsgHandler == true )
        {
            pMsgHandler(msg, pMsgUser);
        }
    } else if(eMsgType == ninjaFailure)             //Failures (ie errors)
    {
        sprintf( msg, "%sERROR: %s\n", runPartMsg, ninjaComMsg);

        fprintf(fpErr, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printToMsgHandler == true )
        {
            pMsgHandler(msg, pMsgUser);
        }
    }
    else if(eMsgType == ninjaFatal)                 //Failures (probably fatal)
    {
        sprintf( msg, "%sERROR: %s\n", runPartMsg, ninjaComMsg);

        fprintf(fpErr, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printToMsgHandler == true )
        {
            pMsgHandler(msg, pMsgUser);
        }
    }

    if( printLastMsg == true )
    {
        // If message is a Failure or Fatal type, write the string to the lastMsg storage
        // which can then be read from ninjaCom using ninja::get_lastComString()
        if( eMsgType == ninjaFailure || eMsgType == ninjaFatal )
        {
            strcpy(lastMsg, ninjaComMsg);  // stores the raw message in lastMsg, without any ninjaCom message processing
        }
    } // if( printLastMsg == true )

    fflush(stdout);
    fflush(stderr);
}

