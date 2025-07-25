 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  main() function to initiate Qt GUI
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

#include "mainwindow.h"
#include "../ninja/windninja.h"
#include <QApplication>
#include <QTimer>
#include <QSplashScreen>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QMessageBox>
#include "splashscreen.h"


int main(int argc, char *argv[])
{
    setbuf(stdout, nullptr);

    int result;

    char ** papszOptions = NULL;
    NinjaErr err = 0;
    err = NinjaInit(papszOptions);   //TODO: NEED TO ADD NINJA INITIALIZE FOR GUI THROUGH API
    if(err != NINJA_SUCCESS)
    {
    printf("NinjaInit: err = %d\n", err);
    }

    QApplication a(argc, argv);
    QIcon icon(":/wn-icon.png");
    QString ver = NINJA_VERSION_STRING;
    a.setWindowIcon(icon);
    a.setApplicationName(QString("WindNinja"));
    a.setApplicationVersion(ver);

    MainWindow* w = nullptr;
    try {
    w = new MainWindow;
    } catch (...) {
    QMessageBox::critical(nullptr, "Initialization Error",
                          "WindNinja failed to initialize. Most likely cause is failure to find data "
                          "dependencies. Try setting the environment variable WINDNINJA_DATA");
    return 1;
    }

    QPixmap bigSplashPixmap(":wn-splash.png");
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

    SplashScreen *splash = new SplashScreen(smallSplashPixmap, list, 1000);
    splash->display();
    QObject::connect(splash, SIGNAL(done()), w, SLOT(show()));

    result = a.exec();
    return result;
}
