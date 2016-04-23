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
    errorCount = 0;
    nMaxErrors = 10;
    printSolverProgress = true;

#ifdef NINJA_GUI
    progressMultiplier = 0;
    nRuns = 0;
    runProgress = 0;
#endif


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


//**********************************************************************
//                       ninjaDefaultComHandler()
//**********************************************************************
/**
* Communication handler for "Default" WindNinja simulations.  Prints everything to stdout.  Normally used for command line type runs.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaDefaultComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    fpLog = stdout;		//print to standard out
    bool printRunNum = true;			//flag to determine if thread number should be printed at beginning of message

    if(printRunNum == true)	//if run number should be printed at beginning of message
    {
        if (eMsg==ninjaFailure || eMsg==ninjaFatal)
        {
            errorCount++;
            if (errorCount > nMaxErrors && nMaxErrors > 0)
            {
                if(errorCount == nMaxErrors+1)
                {
                    fprintf(fpLog, "Run %d: More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            *runNumber, nMaxErrors);
                }
                return;
            }
        }

        if(eMsg == ninjaNone)					//None
            fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);
#ifdef NINJA_DEBUG
        else if(eMsg == ninjaDebug)				//Debug
            fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);
#endif //NINJA_DEBUG
        else if(eMsg == ninjaSolverProgress)	//Solver progress (%complete)
        {    if(printSolverProgress){
                if(atoi(ninjaComMsg) > 99){
                    fprintf(fpLog, "Run %d (solver): 99%% complete\n", *runNumber);
                }
                else
                    fprintf(fpLog, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
              }
        }
        else if(eMsg == ninjaOuterIterProgress)  //Solver progress for outer matching iterations (%complete)
            fprintf(fpLog, "Run %d (matching): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
        else if(eMsg == ninjaWarning)			//Warnings
            fprintf(fpLog, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);
        else if(eMsg == ninjaFailure)			//Failures (ie errors)
            fprintf(fpLog, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        else if(eMsg == ninjaFatal)				//Failures (probably fatal)
            fprintf(fpLog, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);

    }else{	//if run number should NOT be printed at beginning of message

        if (eMsg==ninjaFailure || eMsg==ninjaFatal)
        {
            errorCount++;
            if (errorCount > nMaxErrors && nMaxErrors > 0)
            {
                if(errorCount == nMaxErrors+1)
                {
                    fprintf(fpLog, "More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            nMaxErrors);
                }
                return;
            }
        }

        if(eMsg == ninjaNone)					//None
            fprintf(fpLog, "%s\n", ninjaComMsg);
#ifdef NINJA_DEBUG
        else if(eMsg == ninjaDebug)				//Debug
            fprintf(fpLog, "%s\n", ninjaComMsg);
#endif //NINJA_DEBUG
        else if(eMsg == ninjaSolverProgress)	//Solver progress (%complete)
        {
            if(printSolverProgress)
                fprintf(fpLog, "Solver: %d%% complete\n", atoi(ninjaComMsg));
        }
        else if(eMsg == ninjaOuterIterProgress)  //Solver progress for outer matching iterations (%complete)
            fprintf(fpLog, "Solver (matching): %d%% complete\n", atoi(ninjaComMsg));
        else if(eMsg == ninjaWarning)			//Warnings
            fprintf(fpLog, "\nWarning: %s\n", ninjaComMsg);
        else if(eMsg == ninjaFailure)			//Failures (ie errors)
            fprintf(fpLog, "\nERROR: %s\n", ninjaComMsg);
        else if(eMsg == ninjaFatal)				//Failures (probably fatal)
            fprintf(fpLog, "\nERROR: %s\n", ninjaComMsg);
    }
}


//**********************************************************************
//                        ninjaQuietComHandler()
//**********************************************************************
/**
* Communication handler for "Quiet" WindNinja simulations.  Only prints ninjaFailure and ninjaFatal messages.  Prints everything to stdout.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaQuietComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    if(eMsg==ninjaFailure || eMsg==ninjaFatal)
    {
        fpLog = stdout;		//print to standard out

        if (eMsg==ninjaFailure || eMsg==ninjaFatal)
        {
            errorCount++;
            if (errorCount > nMaxErrors && nMaxErrors > 0)
            {
                if(errorCount == nMaxErrors+1)
                {
                    fprintf(fpLog, "More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            nMaxErrors);
                }
                return;
            }
        }

        if(eMsg == ninjaFailure)			//Failures (ie errors)
            fprintf(fpLog, "\nERROR: %s\n", ninjaComMsg);
        else if(eMsg == ninjaFatal)				//Failures (probably fatal)
            fprintf(fpLog, "\nERROR: %s\n", ninjaComMsg);
    }
}


//**********************************************************************
//                       ninjaLoggingComHandler()
//**********************************************************************
/**
* Communication handler that prints everything out to a file called ninja.log.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaLoggingComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{

    fpLog = fopen("ninja.log", "w+");
    if(fpLog==NULL)
        return;

    if(eMsg == ninjaNone)					//None
        fprintf(fpLog, "%s\n", ninjaComMsg);
#ifdef NINJA_DEBUG
    else if(eMsg == ninjaDebug)				//Debug
        fprintf(fpLog, "%s\n", ninjaComMsg);
#endif //NINJA_DEBUG
    else if(eMsg == ninjaSolverProgress)	//Solver progress (%complete)
    {
        if(printSolverProgress)
            fprintf(fpLog, "Solver: %d%% complete\n", atoi(ninjaComMsg));
    }
    else if(eMsg == ninjaOuterIterProgress)  //Solver progress for outer matching iterations (%complete)
        fprintf(fpLog, "Solver (matching): %d%% complete\n", atoi(ninjaComMsg));
    else if(eMsg == ninjaWarning)			//Warnings
        fprintf(fpLog, "\nWarning: %s\n", ninjaComMsg);
    else if(eMsg == ninjaFailure)			//Failures (ie errors)
        fprintf(fpLog, "\nERROR: %s\n", ninjaComMsg);
    else if(eMsg == ninjaFatal)				//Failures (probably fatal)
        fprintf(fpLog, "\nERROR: %s\n", ninjaComMsg);
}

#ifdef NINJA_GUI
//**********************************************************************
//                        ninjaGUIComHandler()
//**********************************************************************
/**
* Constructor for ninjaGUIComHandler.
* @return
*/
ninjaGUIComHandler::ninjaGUIComHandler() : ninjaComClass()
{
    verbose = true;
}

/**
* Destructor for ninjaGUIComHandler.
* @return
*/
ninjaGUIComHandler::~ninjaGUIComHandler()
{
    if(progressMultiplier)
    {
        delete[] progressMultiplier;
        progressMultiplier = 0;
    }
}

/**
* Communication handler for "GUI" WindNinja simulations.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaGUIComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    QString s;
    //char* lastMsg;	//pointer to last message, points to char in WindNinjaInputs class
    //int* runNumber;	//pointer to run number, points to int in WindNinjaInputs class
    int nThreads = 1;
    /* Trouble */
    if( runNumber == NULL )
        return;
#ifdef _OPENMP
    nThreads = omp_get_num_threads();
#endif
    QCoreApplication::processEvents();

    if(progressMultiplier == 0)
    {
        progressMultiplier = new int[nRuns];
        int nFullChunks = nRuns / nThreads;

        for(int i = 0;i < nFullChunks;i++)
        {
            for(int j = 1;j <= nThreads;j++)
                progressMultiplier[i * j] = nThreads;
        }

        int nDone = nFullChunks * nThreads;
        int nLeft = nRuns - nDone;

        for(int i = 0;i < nLeft;i++)
            progressMultiplier[nDone + i] = nLeft;
    }

    if(*runNumber % nThreads == 0 || nRuns == 1)
    {
        if(eMsg == ninjaSolverProgress)
        {
            if(printSolverProgress)
                emit sendProgress(*runNumber, atoi(ninjaComMsg) * progressMultiplier[*runNumber]);
        }
        if(eMsg == ninjaOuterIterProgress)
        {
            emit sendProgress(*runNumber, atoi(ninjaComMsg) * progressMultiplier[*runNumber]);
        }
    }
    if (eMsg==ninjaFailure || eMsg==ninjaFatal)
    {
        errorCount++;
        if (errorCount > nMaxErrors && nMaxErrors > 0)
        {
            if(errorCount == nMaxErrors+1)
            {
                fprintf(fpLog, "Run %d: More than %d errors have been reported. "
                        "No more will be reported from now on.\n",
                        *runNumber, nMaxErrors);
            }
            //return;
        }
    }

    if(eMsg == ninjaNone)				//None
    {
        fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);
        s = "Run " + QString::number(*runNumber) + ": " + ninjaComMsg;
        //QMetaObject::invokeMethod((QObject*)this, "sendMessage",
        // 			      Qt::QueuedConnection,
        // 			      Q_ARG(QString*, &s),
        // 			      Q_ARG(QColor, Qt::black));
        emit sendMessage(s);
    }
    else if(eMsg == ninjaDebug)
    {				//Debug
        fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);
        s = "Run " + QString::number(*runNumber) + ": " + ninjaComMsg;
        emit sendMessage(s);
    }
    else if(eMsg == ninjaSolverProgress)	//Solver progress (%complete)
    {
        if(printSolverProgress)
        {
            fprintf(fpLog, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
            s = "Run " + QString::number(*runNumber) + " (solver): " + ninjaComMsg + "% done.";
            emit sendMessage(s);
        }
    }
    else if(eMsg == ninjaOuterIterProgress)    //Solver progress (%complete)
    {
        fprintf(fpLog, "Run %d (solver): %d%% complete\n", *runNumber, atoi(ninjaComMsg));
        s = "Run " + QString::number(*runNumber) + " (solver): " + ninjaComMsg + "% done.";
        emit sendMessage(s);
    }
    else if(eMsg == ninjaWarning)			//Warnings
    {
        fprintf(fpLog, "\nRun %d (warning): %s\n", *runNumber, ninjaComMsg);
        s = "Run " + QString::number(*runNumber) + "(warning): " + ninjaComMsg;
        emit sendMessage(s);
    }
    else if(eMsg == ninjaFailure)
    {			//Failures (ie errors)
        fprintf(fpLog, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        s = "Run " + QString::number(*runNumber) + "(ERROR): " + ninjaComMsg;
        emit sendMessage(s);
    }
    else if(eMsg == ninjaFatal)
    {				//Failures (probably fatal)
        fprintf(fpLog, "\nRun %d (ERROR): %s\n", *runNumber, ninjaComMsg);
        s = "Run " + QString::number(*runNumber) + "ERROR): " + ninjaComMsg;
        emit sendMessage(s);
    }
}
#else
//**********************************************************************
//                       ninjaGUIComHandler() for jason
//**********************************************************************
void ninjaGUIComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{

}
#endif // NINJA_GUI

//**********************************************************************
//                        ninjaWFDSSComHandler()
//**********************************************************************
/**
* Communication handler for a "WFDSS" WindNinja simulation.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaWFDSSComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    //If message is a Failure or Fatal type, write the string to the NinjaComString which
    //can then be read from the ninja class using lastComString
    if(eMsg==ninjaFailure || eMsg==ninjaFatal)
        strcpy(lastMsg,ninjaComMsg);	//lastMsg points to string in WindNinjaInputs class (which is inherited by the ninja class)
}


//**********************************************************************
//                       ninjaCLIComHandler()
//**********************************************************************
/**
* Communication handler for a "CLI" WindNinja simulation.
* @param eMsg Type of message to be passed. See msgType for available types.
* @param ninjaComMsg Message to be printed.  Comes from ninjaComV and ninjaCom.
*/
void ninjaCLIComHandler::ninjaComHandler(msgType eMsg, const char *ninjaComMsg)
{
    fpLog = stdout;
    bool printRunNum = true;
    if(printRunNum == true) {
        if (eMsg==ninjaFailure || eMsg==ninjaFatal) {
            errorCount++;
            if (errorCount > nMaxErrors && nMaxErrors > 0) {
                if(errorCount == nMaxErrors+1) {
                    fprintf(stderr, "Run %d: More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            *runNumber, nMaxErrors);
                }
                return;
            }
        }

        if(eMsg == ninjaNone)
            fprintf(fpLog, "Run %d: %s\n", *runNumber, ninjaComMsg);

        else if(eMsg == ninjaSolverProgress)
        {
            if(printSolverProgress)
                fprintf(fpLog, "Run %d (solver): %d%% complete\n",
                    *runNumber, atoi(ninjaComMsg));
        }
        else if(eMsg == ninjaOuterIterProgress)  //Solver progress for outer matching iterations (%complete)
            fprintf(fpLog, "Run %d (matching): %d%% complete\n",
                    *runNumber, atoi(ninjaComMsg));
        else if(eMsg == ninjaWarning)
            fprintf(fpLog, "\nRun %d (warning): %s\n",
                    *runNumber, ninjaComMsg);
        else if(eMsg == ninjaFailure)
            fprintf(stderr, "\nRun %d (ERROR): %s\n",
                    *runNumber, ninjaComMsg);
        else if(eMsg == ninjaFatal)
            fprintf(stderr, "\nRun %d (ERROR): %s\n",
                    *runNumber, ninjaComMsg);

    }
    else {
        if (eMsg==ninjaFailure || eMsg==ninjaFatal) {
            errorCount++;
            if (errorCount > nMaxErrors && nMaxErrors > 0) {
                if(errorCount == nMaxErrors+1) {
                    fprintf(stderr, "More than %d errors have been reported. "
                            "No more will be reported from now on.\n",
                            nMaxErrors);
                }
                return;
            }
        }

        if(eMsg == ninjaNone)
            fprintf(fpLog, "%s\n", ninjaComMsg);
        else if(eMsg == ninjaSolverProgress)
        {
            if(printSolverProgress)
                fprintf(fpLog, "Solver: %d%% complete\n", atoi(ninjaComMsg));
        }
        else if(eMsg == ninjaOuterIterProgress)  //Solver progress for outer matching iterations (%complete)
            fprintf(fpLog, "Solver (matching): %d%% complete\n", atoi(ninjaComMsg));
        else if(eMsg == ninjaWarning)
            fprintf(fpLog, "\nWarning: %s\n", ninjaComMsg);
        else if(eMsg == ninjaFailure)
            fprintf(stderr, "\nERROR: %s\n", ninjaComMsg);
        else if(eMsg == ninjaFatal)
            fprintf(stderr, "\nERROR: %s\n", ninjaComMsg);
        fflush(stdout);
    }
}
