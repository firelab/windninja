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

#include "setConfigurationDialogOption.h"
#include "windninja.h"
#include "gdal_util.h"
#include "ninja_version.h"
#include "ui_mainWindow.h"
#include <QObject>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextDocument>
#include <QTextBlock>
#include <QDesktopServices>

class MenuBar : public QObject
{
    Q_OBJECT

public:
    MenuBar(Ui::MainWindow* ui, QObject* parent = nullptr);

signals:
    void writeToConsoleSignal(QString message, QColor color=Qt::black);

private slots:
    // functions for Menu actions
    // functions for QMenu fileMenu "File" actions
    void newProjectActionTriggered();
    void openProjectActionTriggered();
    void exportSolutionActionTriggered();
    void closeProjectActionTriggered();
    // functions for QMenu optionsMenu "Options" actions
    void writeConsoleOutputActionTriggered();
    // functions for QMenu toolsMenu "Tools" actions
    //void resampleDataActionTriggered();
    void writeBlankStationFileActionTriggered();
    void setConfigurationOptionActionTriggered();
    // functions for QMenu helpMenu "Help" actions
    // functions for sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
    void displayArcGISProGuideActionTriggered();
    // functions for sub QMenu tutorialsMenu "Tutorials" actions
    void displayTutorial1ActionTriggered();
    void displayTutorial2ActionTriggered();
    void displayTutorial3ActionTriggered();
    void displayTutorial4ActionTriggered();
    // functions for sub QMenu instructionsMenu "Instructions" actions
    void displayDemDownloadInstructionsActionTriggered();
    void displayFetchDemInstructionsActionTriggered();
    void displayCommandLineInterfaceInstructionsActionTriggered();
    // functions for remaining non-sub QMenu actions
    void aboutWindNinjaActionTriggered();
    void citeWindNinjaActionTriggered();
    void supportEmailActionTriggered();
    void submitBugReportActionTriggered();
    void enableConsoleOutputActionToggled(bool toggled);

private:
    Ui::MainWindow* ui;
    QDir dataPath;
};

#endif // MENUBAR_H
