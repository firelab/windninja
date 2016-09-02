/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Client to download weather data
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

#ifndef WEATHER_MODEL_H
#define WEATHER_MODEL_H

#include <QCoreApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QToolButton>
#include <QLabel>
#include <QString>
#include <QDirModel>
#include <QTreeView>
#include <QHeaderView>
#include <QDateTime>
#include <QProgressDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QThread>
#include <QLineEdit>

#include <QDebug>

#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "ninjaException.h"
#include "wxModelInitializationFactory.h"
#include "nomads_wx_init.h"

#include "gdal_util.h"
#include "netcdf.h"
#include "ninja_conv.h"

static QProgressDialog *pGlobProg;
static void *pCancel;

static const char *apszWxModelGlossary[] =
{
    "UCAR=University Corporation for Atomosperhic Research",
    "NOMADS=NOAA Operational Model Archive and Distribution System",
    "NDFD=National Digital Forecast Database",
    "NAM=North American Mesoscale",
    "RAP=Rapid Refresh",
    "GFS=Global Forecast System",
    "HIRES=High Resolution",
    "NEST=Nested",
    "ARW=Advanced Research WRF",
    "NMM=Non-hydrostatic Mesoscale model",
    NULL
};

class weatherModel : public QWidget
{
    Q_OBJECT

 public:
    weatherModel(QWidget *parent = 0);
    ~weatherModel();

    QString wxModelFileName;

    QGroupBox *weatherGroupBox;
    QGroupBox *downloadGroupBox;
    QComboBox *modelComboBox;
    QSpinBox *daySpinBox;
    QSpinBox *hourSpinBox;
    QToolButton *downloadToolButton;

    QLabel *ninjafoamConflictLabel;
    QLabel *statusLabel;
    QToolButton *refreshToolButton;

    QLabel *forecastListLabel;

    QTreeView *treeView;
    QDirModel *model;

    QDir cwd;
    QString inputFile;

    QProgressDialog *progressDialog;
    QLabel *progressLabel;

    QHBoxLayout *downloadLayout;
    QHBoxLayout *treeLayout;
    QHBoxLayout *listLayout;
    QHBoxLayout *loadLayout;
    QVBoxLayout *weatherLayout;
    QVBoxLayout *layout;

    QString tzString;
 private:
    void loadModelComboBox();

    const char * ExpandDescription( const char *pszReadable );

    ncepNamSurfInitialization nam;
    ncepNdfdInitialization ndfd;
    ncepRapSurfInitialization rap;
    ncepDgexSurfInitialization dgex;
    ncepNamAlaskaSurfInitialization namAk;
    ncepGfsSurfInitialization gfs;

#ifdef WITH_NOMADS_SUPPORT
    int nNomadsCount;
    NomadsWxModel **papoNomads;
#endif

 private slots:
    void getData();
    void displayForecastTime(const QModelIndex &index);
    void setInputFile(QString newFile);
    void unselectForecast( bool checked );
    void setTimeLimits( int index );
    void setComboToolTip( int index );
 public slots:
    void checkForModelData();
    void updateTz( QString tz );

};

#endif /* WEATHER_MODEL_H */
