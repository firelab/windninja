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

#include "pointDataModel.h"
#include "pointInputDelegate.h"
#include "latLonWidget.h"
#include "wxStation.h"
#include "ninjaUnits.h"
#include "stationFetchWidget.h"
#include <set>

#include <QGridLayout>
#include <QVBoxLayout>

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
    std::vector<int> stationFileTypes;
    int simType;
    bool pointGo;

    QLineEdit *stationFileLineEdit;

    QDateTimeEdit *dateTimeEdit;

    QToolButton *addStationButton;
    QToolButton *removeStationButton;
    QToolButton *readStationFileButton;
    QCheckBox *writeStationFileButton;
    QCheckBox *writeStationKmlButton;
    QToolButton *doTest;

    QGroupBox *pointGroupBox;

    QHBoxLayout *fileLayout;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *pointLayout;
    QVBoxLayout *layout;
    QLabel *ninjafoamConflictLabel;

    pointDataModel pointData;

    pointInputDelegate *pointDelegate;

    //TreeBox Stuff
    QTreeView *stationTreeView;
    QTableView *stationTableView;
    
    QLabel *treeLabel;

    QTreeView *treeView;
    QHBoxLayout *treeLayout;
    QVBoxLayout *vTreeLayout;
    
    QStackedWidget *initPages;
    QComboBox *initOpt;
    QHBoxLayout *optLayout;
    QWidget *oldForm;
    QWidget *newForm;
    
    //Station Fetch Directory Stuff
    
    QDir cwd;
    QDirModel *sfModel;
    //Handlers
    QHBoxLayout *ClippyToolLayout;
    QToolButton *refreshToolButton;
    QLabel *clippit; //displays vital information

    QStringList filters;
    QString tXtest;
    std::vector<std::string> vy; //clean this up
    std::vector<std::string> vx; //For file names
    
    
    //endDirectoryChecking
    //Time series stuff
    QDateTimeEdit *startTime;
    QDateTimeEdit *stopTime;
    QCheckBox *enableTimeseries;
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
    
    //End Timeseries stuff
    QLineEdit *ska;
    QLineEdit *jazz;
    

    void updateTable();
    
    stationFetchWidget *xWidget;
    QString tzString;
    

  public slots:
    void updateTz(QString tz);
    void checkForModelData();
    int directStationTraffic(const char* xFileName);
    static void setWxStationFormat(int format); //I don't Think I need this anymore (delete later)
    void displayInformation(int dataType);
    
    
  private slots:
    void readStationFile();
    void readMultipleStaitonFiles(const QModelIndex &index);
    void selChanged(); //Test Function
    void writeStationFile();
    void writeStationKml();
    void setInputFile( QString file );
    void openMainWindow();
    void openStationFetchWidget();
    int checkNumStations(string comparator, std::vector<std::string> stationVec);
    
    void pairFetchTime(QDateTime xDate); //This is a test function that needs to be deleted!
    void pairStartTime(QDateTime xDate);
    void pairStopTime(QDateTime xDate);
    void pairTimeSeries(int curIndex);
    
    void updateStartTime(QDateTime xDate);
    void updateStopTime(QDateTime xDate);
    
    void toggleUI();
    void toggleTimeseries();

 signals:
    void writeToConsole( QString message );
    void stationFileChanged();
    
friend class stationFetchWidget;
};

#endif	/* POINT_INPUT_H */
