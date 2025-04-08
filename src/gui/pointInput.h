/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Point initialization input.
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#ifndef POINT_INPUT_H
#define POINT_INPUT_H

#include <QDockWidget>
#include <QTableWidget>

#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeView>
#include <QSizePolicy>
#include <QDateTimeEdit>

#include <vector>

#include "latLonWidget.h"
#include "wxStation.h"
#include "ninjaUnits.h"
#include "stationFetchWidget.h"
#include <set>

#include <QGridLayout>
#include <QVBoxLayout>

#include <QMouseEvent>

#include <QDebug>

class pointInput : public QWidget
{
    Q_OBJECT

 public:
  
    pointInput( QWidget *parent = 0 );
    ~pointInput();

    QString demFileName;
    QString stationFileName;
    std::vector<std::string> stationFileList;
    std::vector<std::string> fullFileList;
    std::vector<int> stationFileTypes;
    int simType;
    bool pointGo;
    int isDiurnalChecked;
    bool enableTimeseries;

    QHBoxLayout *diurnalTimeLayout;
    QDateTimeEdit *dateTimeEdit;
    QLabel *diurnalLabel;
    QLabel *oneStepTimeLabel;

    QCheckBox *writeStationFileButton;
    QCheckBox *writeStationKmlButton;
    QToolButton *widgetButton;

    QGroupBox *pointGroupBox;

    QHBoxLayout *buttonLayout;
    QVBoxLayout *pointLayout;
    QVBoxLayout *layout;
    QLabel *ninjafoamConflictLabel;

    //TreeBox Stuff
    
    QLabel *treeLabel;

    QTreeView *treeView;
    QVBoxLayout *vTreeLayout;
    QHBoxLayout *hDownloaderLayout; //Put the downloader up near the top of the page
    QWidget *newForm;
    
    //Station Fetch Directory Stuff
    
    QDir cwd;
    QDirModel *sfModel;
    //Handlers
    QHBoxLayout *ClippyToolLayout;
    QToolButton *refreshToolButton;
    QLabel *clippit; //displays vital information

    QFrame *timeLine; //Creates a fancy line to seprate things out
    QFrame *timeLine2; //Creates another fancy line...

    std::vector<std::string> vx; //For file names
    
    //endDirectoryChecking
    //Time series stuff
    QDateTimeEdit *startTime;
    QDateTimeEdit *stopTime;
    QHBoxLayout *timeBoxLayout;
    QSpinBox *numSteps;
    
    QVBoxLayout *startLayout;
    QVBoxLayout *stopLayout;
    QVBoxLayout *stepLayout;
    
    QLabel *startLabel;
    QLabel *stopLabel;
    QLabel *stepLabel;
    
    std::vector<int> startSeries;
    std::vector<int> endSeries; //Global Storage for start and stop times

    std::vector<int> diurnalTimeVec;
    
    //End Timeseries stuff    
    
    stationFetchWidget *xWidget;
    QString tzString;

  public slots:
    void updateTz(QString tz);
    void checkForModelData();
    int directStationTraffic(const char* xFileName);
    void readStationTime(std::string start_time, std::string stop_time, int xSteps);
    static void setWxStationFormat(int format); //I don't Think I need this anymore (delete later)
    void displayInformation(int dataType);
    QDateTime readNinjaNowName(const char* fileName);
    void setOneStepTimeseries();
    
  private slots:

    void generateFullFileList();
    void collectAllIndexes(const QModelIndex &parent, std::vector<QModelIndex> &allIndexes) const;

    void readStationFiles(const QItemSelection &x ,const QItemSelection &y);
    void selChanged(const QItemSelection &x ,const QItemSelection &y); //Test Function
    void setInputFile( QString file );
    void setDiurnalParam(bool diurnalCheck);
    void openMainWindow();
    void openStationFetchWidget();
    
    void pairStartTime(QDateTime xDate);
    void pairStopTime(QDateTime xDate);
    void pairTimeSeries(int curIndex);
    void updateTimeSteps();
    
    void updateSingleTime(QDateTime xDate);
    void updateStartTime(QDateTime xDate);
    void updateStopTime(QDateTime xDate);
    void watchStopTime();
    void watchStartTime();
    
 signals:
    void writeToConsole( QString message );
    void stationFileChanged();
    
friend class stationFetchWidget;
};

#endif	/* POINT_INPUT_H */
