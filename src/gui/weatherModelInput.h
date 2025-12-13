 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Hands GUI related logic for the Weather Model Page
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

#ifndef WEATHERMODELINPUT_H
#define WEATHERMODELINPUT_H

#include "ui_mainWindow.h"
#include "windninja.h"
#include "appState.h"
#include "QTimeZone"
#include "QFileSystemModel"
#include <QStandardItemModel>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include <QObject>

class WeatherModelInput : public QObject
{
    Q_OBJECT
public:
    explicit WeatherModelInput(Ui::MainWindow* ui, QObject* parent = nullptr);

signals:
    void updateState();

public slots:
    void updateTreeView();
    void updatePastcastDateTimeEdits();

private slots:
    void weatherModelDownloadButtonClicked();
    void weatherModelFileTreeViewItemSelectionChanged(const QItemSelection &selected);
    void weatherModelTimeSelectAllButtonClicked();
    void weatherModelTimeSelectNoneButtonClicked();
    void weatherModelGroupBoxToggled(bool toggled);
    void weatherModelComboBoxCurrentIndexChanged(int index);
    void weatherModelDownloadFinished();

private:
    NinjaToolsH* ninjaTools;
    NinjaErr ninjaErr;

    Ui::MainWindow *ui;
    QFileSystemModel *fileModel;
    QStandardItemModel *timeModel;
    QProgressDialog *progress;
    QFutureWatcher<int> *futureWatcher;
    const QVector<QString> modelGlossary = {
        "UCAR=University Corporation for Atmospheric Research",
        "NOMADS=NOAA Operational Model Archive and Distribution System",
        "GCP=Google Cloud Platform",
        "NDFD=National Digital Forecast Database",
        "NAM=North American Mesoscale",
        "RAP=Rapid Refresh",
        "HRRR=High-Resolution Rapid Refresh",
        "GFS=Global Forecast System",
        "HIRES=High Resolution",
        "NEST=Nested",
        "ARW=Advanced Research WRF",
        "NMM=Non-hydrostatic Mesoscale Model",
        "NBM=National Blend of Models"
    };
    static int fetchForecastWeather(NinjaToolsH* ninjaTools,
                                                      const QString& modelIdentifierStr,
                                                      const QString& demFileStr,
                                                      int hours);

    static int fetchPastcastWeather(NinjaToolsH* ninjaTools,
                                            const QString& modelIdentifierStr,
                                            const QString& demFileStr,
                                            const QString& timeZoneStr,
                                            int startYear, int startMonth, int startDay, int startHour,
                                            int endYear, int endMonth, int endDay, int endHour);
};

#endif // WEATHERMODELINPUT_H
