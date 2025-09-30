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
    printLastMsg = false;
    lastMsg = NULL;
    runNumber = NULL;
    comType = NULL;
    printProgressFunc = false;
    pfnProgress = nullptr;
    pProgressUser = nullptr;
    multiStream = NULL;
    printLogFile = false;
    errorCount = 0;
    nMaxErrors = 10;
    printMaxErrors = false;
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

void ninjaComClass::set_progressFunc(ProgressFunc func, void *pUser)
{
    if( func == NULL || pUser == NULL )
    {
        fprintf(stderr, "ninjaComClass::set_ninjaComProgressFunc() error!! input ProgressFunction or ProgressFunctionUser are NULL!!!\n");
        fflush(stderr);
        return;
    }

    pfnProgress = func;
    pProgressUser = pUser;
    printProgressFunc = true;
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

//void ninjaComClass::initializeNinjaCom(char *LastMsg, int* RunNumber)
//{
//	lastMsg = LastMsg;
//	runNumber = RunNumber;
//}

/**
* Communication handler for WindNinja simulations. Takes an input message and prints/passes it.
* The place the message is printed or passed to (or if the message is ignored)
* depends on the input message type, and other pre-set ninjaCom settings
*
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaComClass::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    //char* lastMsg;	//pointer to last message, points to char in WindNinjaInputs class
    //int* runNumber;	//pointer to run number, points to int in WindNinjaInputs class

    char msg[NINJA_MSG_SIZE];  // Declare a character array to store the result of sprintf, for printing

    if( printProgressFunc == false || multiStream != NULL )
    {
        if( runNumber == NULL )
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
    // but printRunNumber is always effectively set to true for printProgressFunc and multiStream output

    // for now, always assume printMaxErrors has been already been set ahead of time, where the default value is always false, to print all errors

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
                        sprintf( msg, "Run %d: More than %d errors have been reported. ",
                                 "No more will be reported from now on.\n",
                                 *runNumber, nMaxErrors );
                    } else
                    {
                        sprintf( msg, "More than %d errors have been reported. ",
                                 "No more will be reported from now on.\n",
                                 nMaxErrors );
                    }

                    fprintf(fpErr, "%s", msg);

                    if( multiStream != NULL )
                    {
                        fprintf(multiStream, "%s", msg);
                        fflush(multiStream);
                    }

                    if( printProgressFunc == true )
                    {
                        pfnProgress(msg, pProgressUser);
                    }
                }
                return;
            }
        } // if( eMsg == ninjaFailure || eMsg == ninjaFatal )
    } // if( printMaxErrors == true )

    if(eMsg == ninjaNone)                       //None
    {
        if( printRunNumber == true )
        {
            sprintf( msg, "Run %d: %s\n", *runNumber, ninjaComMsg );
        } else
        {
            sprintf( msg, "%s\n", ninjaComMsg );
        }

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printProgressFunc == true )
        {
            pfnProgress(msg, pProgressUser);
        }
    }
    #ifdef NINJA_DEBUG
    else if(eMsg == ninjaDebug)                 //Debug
    {
        if( printRunNumber == true )
        {
            sprintf( msg, "Run %d: %s\n", *runNumber, ninjaComMsg);
        } else
        {
            sprintf( msg, "%s\n", ninjaComMsg);
        }

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printProgressFunc == true )
        {
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
                sprintf( msg, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
            } else
            {
                sprintf( msg, "Solver: %d%% complete\n", atoi(ninjaComMsg));
            }

            fprintf(fpLog, "%s", msg);

            if( multiStream != NULL )
            {
                fprintf(multiStream, "%s", msg);
                fflush(multiStream);
            }

            if( printProgressFunc == true )
            {
                pfnProgress(msg, pProgressUser);
            }
        }
    }
    else if(eMsg == ninjaOuterIterProgress)     //Solver progress for outer matching iterations (%complete)
    {
        if( printRunNumber == true )
        {
            sprintf( msg, "Run %d (matching): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
        } else
        {
            sprintf( msg, "Solver (matching): %d%% complete\n", atoi(ninjaComMsg));
        }

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printProgressFunc == true )
        {
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaWarning)               //Warnings
    {
        if( printRunNumber == true )
        {
            sprintf( msg, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);
        } else
        {
            sprintf( msg, "\nWarning: %s\n", ninjaComMsg);
        }

        fprintf(fpLog, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printProgressFunc == true )
        {
            pfnProgress(msg, pProgressUser);
        }
    } else if(eMsg == ninjaFailure)             //Failures (ie errors)
    {
        if( printRunNumber == true )
        {
            sprintf( msg, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        } else
        {
            sprintf( msg, "\nERROR: %s\n", ninjaComMsg);
        }

        fprintf(fpErr, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printProgressFunc == true )
        {
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaFatal)                 //Failures (probably fatal)
    {
        if( printRunNumber == true )
        {
            sprintf( msg, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        } else
        {
            sprintf( msg, "\nERROR: %s\n", ninjaComMsg);
        }

        fprintf(fpErr, "%s", msg);

        if( multiStream != NULL )
        {
            fprintf(multiStream, "%s", msg);
            fflush(multiStream);
        }

        if( printProgressFunc == true )
        {
            pfnProgress(msg, pProgressUser);
        }
    }

    if( printLastMsg == true )
    {
        //If message is a Failure or Fatal type, write the string to the NinjaComString which
        //can then be read from the ninja class using lastComString
        if( eMsg == ninjaFailure || eMsg == ninjaFatal )
        {
            strcpy(lastMsg, ninjaComMsg);  //lastMsg points to string in WindNinjaInputs class (which is inherited by the ninja class)
        }
    } // if( printLastMsg == true )

    fflush(stdout);
    fflush(stderr);
}

