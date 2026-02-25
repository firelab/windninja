 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles GUI related logic for Point Initialization Page
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

#ifndef POINTINITIALIZATIONINPUT_H
#define POINTINITIALIZATIONINPUT_H

#include "appState.h"
#include "windninja.h"
#include "gdal_util.h"
#include "ui_mainWindow.h"
#include <QObject>
#include <QTimeZone>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include <QFileSystemModel>

class PointInitializationInput : public QObject
{
    Q_OBJECT
public:
    PointInitializationInput(Ui::MainWindow* ui, QObject* parent = nullptr);
    QVector<QString> getStationFiles();

signals:
    void updateState();
    void updateProgressMessageSignal(const QString &msg);
    void writeToConsoleSignal(const QString &msg, QColor color=Qt::black);

public slots:
    void updateTreeView();

private slots:
    void pointInitializationGroupBoxToggled(bool toggled);
    void pointInitializationDownloadDataButtonClicked();
    void weatherStationDataDownloadCancelButtonClicked();
    void weatherStationDataSourceComboBoxCurrentIndexChanged(int index);
    void weatherStationDataTimeComboBoxCurrentIndexChanged(int index);
    void weatherStationDataDownloadButtonClicked();
    void pointInitializationTreeViewItemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void pointInitializationSelectAllButtonClicked();
    void pointInitializationSelectNoneButtonClicked();
    void folderExpanded(const QModelIndex &index);
    void folderCollapsed(const QModelIndex &index);
    void weatherStationDataTimestepsSpinBoxValueChanged(int value);
    void updateProgressMessage(const QString message);
    void weatherStationDataStartDateTimeEditChanged();
    void weatherStationDataEndDateTimeEditChanged();
    void updateDateTime();

private:
    Ui::MainWindow *ui;

    NinjaErr ninjaErr;

    QProgressDialog *progress;
    QFutureWatcher<int> *futureWatcher;
    QFileSystemModel *stationFileSystemModel;
    QDateTime maxStationLocalDateTime, minStationLocalDateTime;
    QVector<QString> stationFiles;
    QVector<int> stationFileTypes;
    QVector<QString> openStationFolders;

    int fetchStationFromBbox(NinjaToolsH* ninjaTools,
                             QVector<int> year,
                             QVector<int> month,
                             QVector<int> day,
                             QVector<int> hour,
                             QVector<int> minute,
                             QString elevationFile,
                             double buffer,
                             QString units,
                             QString osTimeZone,
                             bool fetchLatestFlag,
                             QString outputPath);
    int fetchStationByName(NinjaToolsH* ninjaTools,
                           QVector<int> year,
                           QVector<int> month,
                           QVector<int> day,
                           QVector<int> hour,
                           QVector<int> minute,
                           QString elevationFile,
                           QString stationList,
                           QString osTimeZone,
                           bool fetchLatestFlag,
                           QString outputPath);
    void fetchStationDataFinished();
    bool readTimeSeries(QModelIndex row);
    void readStationTime(QString startDateTimeStr, QString stopDateTimeStr);
    void updateTimeSteps();
};

#endif // POINTINITIALIZATIONINPUT_H
