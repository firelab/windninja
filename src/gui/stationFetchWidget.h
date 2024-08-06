/******************************************************************************
 *
 * $Id: stationFetchWdiget.h 1757 2012-08-07 18:40:40Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  stationFetchWidget
 * Author:   tfinney@fs.fed.us
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

#ifndef STATIONFETCHWIDGET_H_
#define STATIONFETCHWIDGET_H_

#include <QtCore>
#include <QUrl>
#include <QDesktopServices>
#include <QtGui>
#include <QtWebKit>
#include <QMessageBox>
#include <QCloseEvent>
#include <math.h>
#include <QFileDialog>
#include "fetch_factory.h"
#include "gdal_util.h"
#include "ninja_conv.h"
#include <qnetworkconfigmanager.h>
#include <qnetworksession.h>
#include "ui_stationFetchWidget.h"
#include "GoogleMapsInterface.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "pointInitialization.h"

#ifndef PI
#define PI 3.14159
#endif

class stationFetchWidget : public QWidget, private Ui::stationFetchWidget
{
    Q_OBJECT

public:
    QString demFileName;
    QDir demFileDir;
    stationFetchWidget(QWidget *parent = 0);
    ~stationFetchWidget();
    QDir settingsDir;
    void connectInputs();
    QString tzString;    
    void updatetz(QString tz);
    void fixTime();
    std::string removeWhiteSpace(std::string str); 
    void setActuallyPressed(bool pressed); 
    bool getActuallyPressed(); 
    std::string getStationIDS(); 
    int getBuffer();
    std::string getBufferUnits();
    bool getTimeseries(); 
    std::string getType(); 

protected:
    void closeEvent(QCloseEvent *event);
   
    private slots:
        void closeDEM();
        void getMetadata();
        void setInputFile( QString file );
        int fetchStation();
        std::string demButcher();
        void watchStartTime();
        void watchStopTime();

        //Progress Bar Slots
        void updateFetchProgress();
        void executeFetchStation(); //Wrapper for fetch station, necessary for progress bar
        
    signals:
        void writeToConsole(QString message);
        void exitWidget();
        
private:
        //Progress Bar Stuff
        QProgressDialog *stationFetchProgress;
        QFutureWatcher<int> stationFutureWatcher;
        bool pressedexecute;
friend class pointInput;
};

#endif /* STATIONFETCHWIDGET_H_ */

