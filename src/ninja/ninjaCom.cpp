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
* Constructor for ninjaComClass.
* @return
*/
ninjaComClass::ninjaComClass()
{
    fpLog = stdout;
    fpErr = stderr;
    lastMsg = NULL;
    runNumber = NULL;
    comType = NULL;
    pfnProgress = nullptr;
    pProgressUser = nullptr;
    errorCount = 0;
    nMaxErrors = 10;
    printMaxErrors = true;
    printSolverProgress = true;
    printRunNumber = true;
    progressWeight = 1.0;
}

/**
* Destructor for ninjaComClass.
* @return
*/
ninjaComClass::~ninjaComClass()
{

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
* Communication function used to pass messages from the program to different ninjaComHandler() functions.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param Message to be passed, using string formatting (like a printf() statement).
*/
void ninjaComClass::ninjaCom(msgType eMsg, const char *fmt, ...)
{
    va_list args;

    // Expand the error message
    va_start(args, fmt);
    ninjaComV(eMsg, fmt, args);
    va_end(args);
}

/**
* This is an intermediate function that parses the args...?
* @param eMsg Type of message to be passed. See msgType for available types.
* @param fmt Message to be passed, using string formatting (like a printf() statement).
* @param args Arguments list for fmt.
*/
void ninjaComClass::ninjaComV(msgType eMsg, const char *fmt, va_list args)
{
    char ninjaMsg[NINJA_MSG_SIZE] = "";
    vsnprintf(ninjaMsg, NINJA_MSG_SIZE-2, fmt, args);

    ninjaComHandler(eMsg, ninjaMsg);
}

//void ninjaComClass::initializeNinjaCom(char *LastMsg, int* RunNumber, eNinjaCom* ComType)
//{
//	lastMsg = LastMsg;
//	runNumber = RunNumber;
//	comType = ComType;
//}

/**
* Communication handler for WindNinja simulations. Takes an input message and prints/passes it.
* The place the message is printed or passed to (or if the message is ignored) depends on
* the input message type, the pre-set ninjaComHandler type, and other pre-set ninjaCom settings
*
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaComClass::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    //char* lastMsg;	//pointer to last message, points to char in WindNinjaInputs class
    //int* runNumber;	//pointer to run number, points to int in WindNinjaInputs class

    char msg[NINJA_MSG_SIZE];  // Declare a character array to store the result of sprintf, for printing

    if( *comType == eNinjaCom::ninjaGUICom )
    {
        if( runNumber == NULL )
        {
            return;
        }

        if( multiStream == NULL )
        {
            return;
        }
        if( pfnProgress == NULL || pProgressUser == NULL )
        {
            return;
        }
    }

    fpLog = stdout;
    if( *comType == eNinjaCom::ninjaLoggingCom )
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

// not applicable for eNinjaCom::WFDSSCom
// printRunNumber is always effectively set to true for eNinjaCom::ninjaGUICom
if( *comType == eNinjaCom::ninjaQuietCom || *comType == eNinjaCom::ninjaLoggingCom )
{
    printRunNumber = false;
}

if( *comType == eNinjaCom::ninjaLoggingCom || *comType == eNinjaCom::WFDSSCom || *comType == eNinjaCom::ninjaGUICom )
{
    printMaxErrors = false;
}

if( printMaxErrors == true )
{
    if( eMsg == ninjaFailure || eMsg == ninjaFatal )
    {
        errorCount++;
        if( errorCount > nMaxErrors && nMaxErrors > 0 )
        {
            if( errorCount == nMaxErrors+1 )
            {
                if( printRunNumber == true )
                {
                    fprintf(fpErr, "Run %d: More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            *runNumber, nMaxErrors);
                } else
                {
                    fprintf(fpErr, "More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            nMaxErrors);
                }

                if( *comType == eNinjaCom::ninjaGUICom )
                {
                    fprintf(multiStream, "Run %d: More than %d errors have been reported. ",
                            "No more will be reported from now on.\n",
                            *runNumber, nMaxErrors);
                    fflush(multiStream);

                    sprintf( msg, "Run %d: More than %d errors have been reported. ",
                             "No more will be reported from now on.\n",
                             *runNumber, nMaxErrors );
                    pfnProgress(msg, pProgressUser);
                }
            }
            return;
        }
    }
} // if( printMaxErrors == true )

if( *comType != eNinjaCom::ninjaQuietCom && *comType != eNinjaCom::WFDSSCom )
{
    if(eMsg == ninjaNone)                       //None
    {
        if( printRunNumber == true )
        {
            fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);
        } else
        {
            fprintf(fpLog, "%s\n", ninjaComMsg);
        }

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "Run %d: %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "Run %d: %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
    #ifdef NINJA_DEBUG
    else if(eMsg == ninjaDebug)                 //Debug
    {
        if( printRunNumber == true )
        {
            fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);
        } else
        {
            fprintf(fpLog, "%s\n", ninjaComMsg);
        }

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "Run %d: %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "Run %d: %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
    #endif //NINJA_DEBUG
    else if(eMsg == ninjaSolverProgress)        //Solver progress (%complete)
    {
        if(printSolverProgress)
        {
            if( printRunNumber == true )
            {
                if( atoi(ninjaComMsg) > 99 ) // does this even matter for anything??
                {
                fprintf(fpLog, "Run %d (solver): 99%% complete\n", *runNumber);
                } else
                {
                fprintf(fpLog, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
                }
            } else
            {
                if( atoi(ninjaComMsg) > 99 ) // does this even matter for anything??
                {
                fprintf(fpLog, "Solver: 99%% complete\n");
                } else
                {
                fprintf(fpLog, "Solver: %d%% complete\n", atoi(ninjaComMsg));
                }
            }

            if( *comType == eNinjaCom::ninjaGUICom )
            {
                if( atoi(ninjaComMsg) > 99 ) // does this even matter for anything??
                {
                fprintf(multiStream, "Run %d (solver): 99%% complete\n", *runNumber);
                } else
                {
                fprintf(multiStream, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
                }
                fflush(multiStream);

                if( atoi(ninjaComMsg) > 99 ) // does this even matter for anything??
                {
                sprintf( msg, "Run %d (solver): 99%% complete\n", atoi(ninjaComMsg) );
                } else
                {
                sprintf( msg, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg) );
                }
                pfnProgress(msg, pProgressUser);
            }
        }
    }
    else if(eMsg == ninjaOuterIterProgress)     //Solver progress for outer matching iterations (%complete)
    {
        if( printRunNumber == true )
        {
            fprintf(fpLog, "Run %d (matching): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
        } else
        {
            fprintf(fpLog, "Solver (matching): %d%% complete\n", atoi(ninjaComMsg));
        }

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "Run %d (matching): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
            fflush(multiStream);

            sprintf( msg, "Run %d (matching): %d%% complete\n", *runNumber, atoi(ninjaComMsg) );
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaWarning)               //Warnings
    {
        if( printRunNumber == true )
        {
            fprintf(fpLog, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);
        } else
        {
            fprintf(fpLog, "\nWarning: %s\n", ninjaComMsg);
        }

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
} // if( *comType != eNinjaCom::ninjaQuietCom && *comType != eNinjaCom::WFDSSCom )

if( *comType != eNinjaCom::WFDSSCom )
{
    if(eMsg == ninjaFailure)                    //Failures (ie errors)
    {
        if( printRunNumber == true )
        {
            fprintf(fpErr, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        } else
        {
            fprintf(fpErr, "\nERROR: %s\n", ninjaComMsg);
        }

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaFatal)                 //Failures (probably fatal)
    {
        if( printRunNumber == true )
        {
            fprintf(fpErr, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        } else
        {
            fprintf(fpErr, "\nERROR: %s\n", ninjaComMsg);
        }

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
} // if( *comType != eNinjaCom::WFDSSCom )

if( *comType == eNinjaCom::WFDSSCom )
{
    //If message is a Failure or Fatal type, write the string to the NinjaComString which
    //can then be read from the ninja class using lastComString
    if( eMsg == ninjaFailure || eMsg == ninjaFatal )
    {
        strcpy(lastMsg, ninjaComMsg);  //lastMsg points to string in WindNinjaInputs class (which is inherited by the ninja class)
    }
} // if( *comType == eNinjaCom::WFDSSCom )

    // technically only eNinjaCom::CLI had this, probably fine to just do it anyways
    fflush(stdout);
}

