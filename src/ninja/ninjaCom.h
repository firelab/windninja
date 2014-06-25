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

#ifdef _OPENMP
#include "omp.h"
#endif

#ifdef NINJA_GUI
#include <QObject>
#include <QString>
#include <QColor>
#endif

#define NINJA_MSG_SIZE 1000

class ninjaComClass //virtual base class
#ifdef NINJA_GUI
  : public QObject
#endif
{
public:
    ninjaComClass();
    virtual ~ninjaComClass();

#ifdef NINJA_GUI
    int *runProgress;
    int nRuns;
    int *progressMultiplier;
#endif

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

    typedef enum
    {
        ninjaDefaultCom,
        ninjaQuietCom,
        ninjaLoggingCom,
        ninjaGUICom,
        WFDSSCom,
        ninjaCLICom
    } eNinjaCom;

    char* lastMsg;	//pointer to last message, points to char in WindNinjaInputs class
    int* runNumber;	//pointer to run number, points to int in WindNinjaInputs class
    eNinjaCom* comType;	//pointer to communication type, should point to eNinjaCom in WindNinjaInputs class

    FILE*     fpLog;

    int errorCount;	//running error count
    int	nMaxErrors;	//max number of errors to report
    bool printSolverProgress;   //flag specifying where normal solver progress should be printed (matching will still be printed)

    //methods

    void noSolverProgress();

    void ninjaCom(msgType eMsg, const char *fmt, ...);
    void ninjaComV(msgType, const char *, va_list);
    //void initializeNinjaCom(char *LastMsg, int* RunNumber, eNinjaCom* ComType);

    //pure virtual function that must be overridden in derived classes
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg) = 0;

};


class ninjaDefaultComHandler : public ninjaComClass	//concrete class
{
public:
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};


class ninjaQuietComHandler : public ninjaComClass	//concrete class
{
public:
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};


class ninjaLoggingComHandler : public ninjaComClass	//concrete class
{
public:
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};

#ifndef NINJA_GUI
class ninjaGUIComHandler : public ninjaComClass	//concrete class
{
public:
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};
#else
class ninjaGUIComHandler : public ninjaComClass //concrete class
{
  Q_OBJECT
 public:
  ninjaGUIComHandler();
  ~ninjaGUIComHandler();
  bool verbose;
  virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);

 signals:
  void sendProgress(int run, int progress);
  void sendMessage(QString message, QColor color = Qt::black);

};
#endif // NINJA_GUI

class ninjaWFDSSComHandler : public ninjaComClass	//concrete class
{
public:
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};

class ninjaCLIComHandler : public ninjaComClass	//concrete class
{
public:
    virtual void ninjaComHandler(msgType eMsg, const char *ninjaComMsg);
};
#endif //NINJACOM_H
