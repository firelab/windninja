/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Main Window that handles GUI scraping and state changes
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "surfaceinput.h"
#include "surfaceinputview.h"
#include "menubarview.h"
#include "domainaverageview.h"
#include "mapbridge.h"
#include "serverbridge.h"
#include "ui_mainwindow.h"
#include "appstate.h"
#include <QWebChannel>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QProgressDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QTreeWidget>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QListView>
#include <vector>
#include <string>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void populateForecastDownloads();
    void toggleExpandCollapse(const QModelIndex &index);
    void loadMapKMZ(const std::vector<std::string>& input);

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void openElevationFile();

private slots:
    void massSolverCheckBoxClicked();
    void momentumSolverCheckBoxClicked();
    void diurnalCheckBoxClicked();
    void stabilityCheckBoxClicked();
    void treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void pointInitializationCheckBoxClicked();
    void useWeatherModelInitClicked();
    void solveButtonClicked();
    void outputDirectoryButtonClicked();
    void numberOfProcessorsSolveButtonClicked();
    void timeZoneAllZonesCheckBoxClicked();
    void timeZoneDetailsCheckBoxClicked();
    void timeZoneComboBoxCurrentIndexChanged(int index);
    void refreshUI();


private:
    void connectSignals();
    void treeItemClicked(QTreeWidgetItem *item, int column);

    Ui::MainWindow *ui;
    QWebEngineView *webView;
    QWebChannel *channel;
    MapBridge *mapBridge;
    SurfaceInput *surfaceInput;
    SurfaceInputView *surfaceInputView;
    MenuBarView *menuBarView;
    ServerBridge *serverBridge;
    DomainAverageView *domainAverageView;
};
#endif // MAINWINDOW_H
