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
    lastMsg = NULL;
    runNumber = NULL;
    comType = NULL;
    pfnProgress = nullptr;
    pProgressUser = nullptr;
    errorCount = 0;
    nMaxErrors = 10;
    printSolverProgress = true;
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

    /* Trouble */
    if( runNumber == NULL )
        return;

    if (eMsg==ninjaFailure || eMsg==ninjaFatal)
    {
        errorCount++;
        if (errorCount > nMaxErrors && nMaxErrors > 0)
        {
            if(errorCount == nMaxErrors+1)
            {
                //fprintf(fpLog, "Run %d: More than %d errors have been reported. "
                //        "No more will be reported from now on.\n",
                //        *runNumber, nMaxErrors);

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
            //return;
        }
    }

    if(eMsg == ninjaNone)                       //None
    {
        //fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "Run %d: %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "Run %d: %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaDebug)                 //Debug
    {
        //fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "Run %d: %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "Run %d: %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaSolverProgress)        //Solver progress (%complete)
    {
        if(printSolverProgress)
        {
            //fprintf(fpLog, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));

            if( *comType == eNinjaCom::ninjaGUICom )
            {
                fprintf(multiStream, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
                fflush(multiStream);

                sprintf( msg, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg) );
                pfnProgress(msg, pProgressUser);
            }
        }
    }
    else if(eMsg == ninjaOuterIterProgress)     //Solver progress (%complete)
    {
        //fprintf(fpLog, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
            fflush(multiStream);

            sprintf( msg, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg) );
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaWarning)               //Warnings
    {
        //fprintf(fpLog, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);

        if( *comType == eNinjaCom::ninjaGUICom )
        {
            fprintf(multiStream, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);
            fflush(multiStream);

            sprintf( msg, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg );
            pfnProgress(msg, pProgressUser);
        }
    }
    else if(eMsg == ninjaFailure)               //Failures (ie errors)
    {
        //fprintf(fpLog, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);

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
        //fprintf(fpLog, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);

        if( *comType == eNinjaCom::ninjaGUICom )
        {
        fprintf(multiStream, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        fflush(multiStream);

        sprintf( msg, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg );
        pfnProgress(msg, pProgressUser);
        } // if( *comType == eNinjaCom::ninjaGUICom )
    }
}

