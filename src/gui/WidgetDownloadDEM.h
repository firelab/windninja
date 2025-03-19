/******************************************************************************
 *
 * $Id: WidgetDownloadDEM.h 1757 2012-08-07 18:40:40Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  DEM Downloader Window
 * Author:   Cody Posey <cody.posey85@gmail.com>
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

#ifndef WIGDET_DOWNLOAD_DEM_H_
#define WIGDET_DOWNLOAD_DEM_H_

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
#include "ui_WidgetDownloadDEM.h"
#include "GoogleMapsInterface.h"

#ifndef PI
#define PI 3.14159
#endif

class WidgetDownloadDEM : public QWidget, private Ui::WidgetDownloadDEM
{
    Q_OBJECT

public:
    WidgetDownloadDEM(QWidget *parent = 0);
    ~WidgetDownloadDEM();
    QDir settingsDir;
    void initializeGoogleMapsInterface();
    void setupGM();
    void connectInputs();
    void fillNoDataValues(const char* file);
    void writeSettings();
    void readSettings();
    int fetchBoundBox(double *boundsBox, const char *fileName, double resolution);

    bool get_wasDemFetched();
    std::string get_elevSource();
    double get_northBound();
    double get_southBound();
    double get_eastBound();
    double get_westBound();

protected:
    void closeEvent(QCloseEvent *event);
   
    private slots:
        void clearListeners();
        void saveDEM();
        void updateProgress();
        void updateDEMSource(int index);
        bool demBoundsCheck();
        void zoomToMidpoint();
        void demSelectedUpdate(bool selected);
        void closeDEM();

    signals:
         void doneDownloading(const char* file);
         void exitDEM();
        

private:
    QDialog dlg;

    bool wasDemFetched;
    std::string elevSource;

    double latitude;
    double longitude;
    double northBound;
    double southBound;
    double eastBound;
    double westBound;
    double srtm_southBound;
    double srtm_westBound;
    double srtm_northBound;
    double srtm_eastBound;
    double world_gmted_southBound;
    double world_gmted_westBound;
    double world_gmted_northBound;
    double world_gmted_eastBound;
    double lcp_southBound;
    double lcp_westBound;
    double lcp_northBound;
    double lcp_eastBound;
    double currentResolution;

    double northDEMBound;
    double southDEMBound;

    double fileSize;
    bool demSelected;

    const char *demFile;
    QDir demFileDir;
    QFutureWatcher<int> futureWatcher;
    QProgressDialog *progressBar;
    QMessageBox *boundsError;
    QMessageBox *bufferError;
    QMessageBox *latlngError;
    SurfaceFetch *fetcher;
    GoogleMapsInterface* gmInterface;
    QString currentSaveAsDesc;
    QString currentSuffix;
};

#endif /* WIGDET_DOWNLOAD_DEM_H_ */

