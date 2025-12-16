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

#include "outputs.h"
#include "surfaceInput.h"
#include "menuBar.h"
#include "domainAverageInput.h"
#include "pointInitializationInput.h"
#include "mapBridge.h"
#include "serverBridge.h"
#include "weatherModelInput.h"
#include "ui_mainWindow.h"
#include "appState.h"
#include "windninja.h"
#include <QWebChannel>
#include <QFuture>
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

struct OutputMeshResolution {
    double resolution;
    QByteArray units;
};

struct OutputPDFSize {
    double PDFHeight;
    double PDFWidth;
    double PDFDpi;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void toggleExpandCollapse(const QModelIndex &index);
    void loadMapKMZ(const std::vector<std::string>& input);

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void updateDirunalState();
    void updateStabilityState();
    void updateMetholodyState();
    void updateProgressValueSignal(int run, int progress);
    void updateProgressMessageSignal(const QString &msg);
    void writeToConsoleSignal(const QString &msg, QColor color = Qt::white);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void massSolverCheckBoxClicked();
    void momentumSolverCheckBoxClicked();
    void diurnalCheckBoxClicked();
    void stabilityCheckBoxClicked();
    void treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void solveButtonClicked();
    void outputDirectoryButtonClicked();
    void writeToConsole(QString message, QColor color = Qt::white);
    void updateProgressValue(int run, int progress);
    void updateProgressMessage(const QString message);
    void cancelSolve();

private:
    Ui::MainWindow *ui;
    QWebEngineView *webEngineView;
    QWebChannel *webChannel;
    MapBridge *mapBridge;
    SurfaceInput *surfaceInput;
    MenuBar *menuBar;
    ServerBridge *serverBridge;
    DomainAverageInput *domainAverageInput;
    PointInitializationInput *pointInitializationInput;
    WeatherModelInput *weatherModelInput;
    Outputs *outputs;

    NinjaArmyH *ninjaArmy;
    NinjaErr ninjaErr;

    std::vector<int> runProgress;
    int totalProgress;
    int maxProgress;

    QProgressDialog *progressDialog;
    QFutureWatcher<int> *futureWatcher;

    int startSolve(int numProcessors);
    void finishedSolve();

    QString currentDEMFilePath;

    void connectSignals();
    void treeItemClicked(QTreeWidgetItem *item, int column);
    bool prepareArmy(NinjaArmyH *ninjaArmy, int numNinjas, const char* initializationMethod);
    bool setOutputFlags(NinjaArmyH* ninjaArmy,
                        int i,
                        int numNinjas,
                        OutputMeshResolution googleEarth,
                        OutputMeshResolution fireBehavior,
                        OutputMeshResolution shapeFiles,
                        OutputMeshResolution geospatialPDFs,
                        OutputPDFSize PDFSize);
    OutputMeshResolution getMeshResolution(bool useOutputMeshResolution,
                                           QDoubleSpinBox* outputMeshResolutionSpinBox,
                                           QComboBox* outputMeshResolutionComboBox);

    int lineNumber;

    void writeSettings();
    void readSettings();

};
#endif // MAINWINDOW_H
