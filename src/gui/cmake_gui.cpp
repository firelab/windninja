/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  main() function to initiate Qt GUI
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

#include "cli.h"

#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include <QString>
#include <QErrorMessage>
#include <QMessageBox>
#include "mainWindow.h"
#include "splash.h"

#include "ninjaArmy.h"
#include "ninjaException.h"
#include "ninja_conv.h"
#include "ninja_init.h"

#include "gdal.h"
#include "ogr_api.h"

#ifdef _OPENMP
omp_lock_t netCDF_lock;
#endif
int main(int argc, char *argv[])
{
    int result;
#ifdef _OPENMP
    omp_init_lock (&netCDF_lock);
#endif

    if(argc > 1)
    {
        CPLSetConfigOption( "NINJA_DISABLE_CALL_HOME", "ON" );
        result = windNinjaCLI(argc, argv);
#ifdef _OPENMP
        omp_destroy_lock (&netCDF_lock);
#endif
        return result;
    }

    NinjaInitialize();

    QApplication app(argc, argv);

    //set application name/version in User-Agent header
    QString ver = NINJA_VERSION_STRING;
    app.setApplicationName(QString("WindNinja"));
    app.setApplicationVersion(ver);

    app.setWindowIcon(QIcon(":wn-icon.png"));

    QPixmap bigSplashPixmap(":wn-splash.png");
    //resampled one
    QSize splashSize(1200, 320);
    QPixmap smallSplashPixmap;
    smallSplashPixmap = bigSplashPixmap.scaled(splashSize,
                         Qt::KeepAspectRatioByExpanding);
    QStringList list;
    list << "Loading WindNinja " + ver + "...";
    list << "Loading mesh generator...";
    list << "Loading conjugate gradient solver...";
    list << "Loading preconditioner...";
    list << "WindNinja " + ver + " loaded.";

    mainWindow *mw;
    QMessageBox mbox;
    try
    {
        mw = new mainWindow;
    }
    catch(...)
    {
        mbox.setText("WindNinja failed to initialize.  Most "
                     "likely cause is failure to find data "
                     "dependencies.  Try setting the environment "
                     "variable WINDNINJA_DATA");
        mbox.exec();
        return 1;
    }
    char **papszMsg = NinjaCheckVersion();
    if (papszMsg != NULL) {
      const char *pszVers =
          CSLFetchNameValueDef(papszMsg, "VERSION", NINJA_VERSION_STRING);
      if (strcmp(pszVers, NINJA_VERSION_STRING) > 0) {
        mbox.setText("A new version of WindNinja is available: " +
                     QString(pszVers));
        mbox.exec();
      }
      char **papszUserMsg = CSLFetchNameValueMultiple(papszMsg, "MESSAGE");
      for (int i = 0; i < CSLCount(papszUserMsg); i++) {
        mbox.setText(QString(papszUserMsg[i]));
        mbox.exec();
      }
      CSLDestroy(papszUserMsg);
      if (CSLFetchNameValue(papszMsg, "ABORT") != NULL) {
        mbox.setText("There is a fatal flaw in Windninja, it must close.");
        mbox.exec();
        abort();
      }
    }
    CSLDestroy(papszMsg);

    splashScreen *splash = new splashScreen(smallSplashPixmap, list, 1000);
    splash->display();
    QObject::connect(splash, SIGNAL(done()), mw, SLOT(show()));
    result = app.exec();

#ifdef _OPENMP
    omp_destroy_lock (&netCDF_lock);
#endif

    return result;
}
