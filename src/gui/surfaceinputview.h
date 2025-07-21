 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Hands GUI related logic for the Surface Input Page
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

#ifndef SURFACEINPUTVIEW_H
#define SURFACEINPUTVIEW_H

#include "surfaceinput.h"
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QObject>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDebug>
#include <QProgressDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class MainWindow;
}

class SurfaceInputView : public QObject {
  Q_OBJECT
public:
    explicit SurfaceInputView(Ui::MainWindow* ui,
                                QWebEngineView* webView,
                                SurfaceInput* surfaceInput,
                                QObject* parent = nullptr);

signals:
    void requestRefresh();

public slots:
    void boundingBoxReceived(double north, double south, double east, double west);
    void elevationInputFileOpenButtonClicked();

private slots:
    void surfaceInputDownloadCancelButtonClicked();
    void surfaceInputDownloadButtonClicked();
    void meshResolutionUnitsComboBoxCurrentIndexChanged(int index);
    void elevationInputTypePushButtonClicked();
    void boundingBoxLineEditsTextChanged();
    void pointRadiusLineEditsTextChanged();
    void elevationInputFileDownloadButtonClicked();
    void elevationInputFileLineEditTextChanged(const QString &arg1);
    void meshResolutionComboBoxCurrentIndexChanged(int index);
    void fetchDEMFinished();

private:
    void startFetchDEM(QVector<double> boundingBox, std::string demFile, double resolution, std::string fetchType);
    Ui::MainWindow *ui;
    QWebEngineView *webView;
    SurfaceInput *surfaceInput;

    QProgressDialog *progress;
    QFutureWatcher<int> *futureWatcher;
    QString currentDEMFilePath;
};

#endif // SURFACEINPUTVIEW_H
