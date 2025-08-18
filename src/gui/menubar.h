 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Hands GUI related logic for the Menu Bar
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

#ifndef MENUBAR_H
#define MENUBAR_H

#include "setconfigurationoptiondialog.h"

#include "windninja.h"
#include "../ninja/gdal_util.h"
#include "ninja_version.h"
#include <QObject>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

namespace Ui {
class MainWindow;
}

class MenuBar : public QObject
{
    Q_OBJECT

public:
    MenuBar(Ui::MainWindow* ui, QObject* parent = nullptr);

signals:
    void writeToConsole(QString message, QColor color = Qt::white);
    void openElevationInputFileMenuActionTriggered();

private slots:
    // functions for Menu actions
    // functions for QMenu fileMenu "File" actions
    void newProject();
    void openProject();
    void exportSolution();
    void closeProject();
    // functions for QMenu optionsMenu "Options" actions
    void writeConsoleOutput();
    // functions for QMenu toolsMenu "Tools" actions
    void resampleData();
    void writeBlankStationFile();
    void setConfigurationOption();
    // functions for QMenu helpMenu "Help" actions
    // functions for sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
    void displayArcGISProGuide();
    // functions for sub QMenu tutorialsMenu "Tutorials" actions
    void displayTutorial1();
    void displayTutorial2();
    void displayTutorial3();
    void displayTutorial4();
    // functions for sub QMenu instructionsMenu "Instructions" actions
    void displayDemDownloadInstructions();
    void displayFetchDemInstructions();
    void displayCommandLineInterfaceInstructions();
    // functions for remaining non-sub QMenu actions
    void aboutWindNinja();
    void citeWindNinja();
    void supportEmail();
    void submitBugReport();

private:
    Ui::MainWindow* ui;
    QDir dataPath;
};

#endif // MENUBAR_H
