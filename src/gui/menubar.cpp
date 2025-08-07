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

#include "menubar.h"
#include "ui_mainwindow.h"

MenuBar::MenuBar(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent), ui(ui)
{
    // QMenu fileMenu "File" actions
    connect(ui->newProjectAction, &QAction::triggered, this, &MenuBar::newProject);
    connect(ui->openProjectAction, &QAction::triggered, this, &MenuBar::openProject);
    connect(ui->exportSolutionAction, &QAction::triggered, this, &MenuBar::exportSolution);
    connect(ui->closeProjectAction, &QAction::triggered, this, &MenuBar::closeProject);
    connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QCoreApplication::quit);  // exit the entire app

    // QMenu optionsMenu "Options" actions
    connect(ui->enableConsoleOutputAction, &QAction::triggered, this, &MenuBar::enableConsoleOutput);
    connect(ui->writeConsoleOutputAction, &QAction::triggered, this, &MenuBar::writeConsoleOutput);

    // QMenu toolsMenu "Tools" actions
    connect(ui->resampleDataAction, &QAction::triggered, this, &MenuBar::resampleData);
    connect(ui->writeBlankStationFileAction, &QAction::triggered, this, &MenuBar::writeBlankStationFile);
    connect(ui->setConfigurationOptionAction, &QAction::triggered, this, &MenuBar::setConfigurationOption);

    // QMenu helpMenu "Help" actions
    // sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
    connect(ui->displayArcGISProGuideAction, &QAction::triggered, this, &MenuBar::displayArcGISProGuide);

    // sub QMenu tutorialsMenu "Tutorials" actions
    connect(ui->displayTutorial1Action, &QAction::triggered, this, &MenuBar::displayTutorial1);
    connect(ui->displayTutorial2Action, &QAction::triggered, this, &MenuBar::displayTutorial2);
    connect(ui->displayTutorial3Action, &QAction::triggered, this, &MenuBar::displayTutorial3);
    connect(ui->displayTutorial4Action, &QAction::triggered, this, &MenuBar::displayTutorial4);

    // sub QMenu instructionsMenu "Instructions" actions
    connect(ui->displayDemDownloadInstructionsAction, &QAction::triggered, this, &MenuBar::displayDemDownloadInstructions);
    connect(ui->displayFetchDemInstructionsAction, &QAction::triggered, this, &MenuBar::displayFetchDemInstructions);
    connect(ui->displayCommandLineInterfaceInstructionsAction, &QAction::triggered, this, &MenuBar::displayCommandLineInterfaceInstructions);

    // remaining non-sub QMenu actions
    connect(ui->aboutWindNinjaAction, &QAction::triggered, this, &MenuBar::aboutWindNinja);
    connect(ui->citeWindNinjaAction, &QAction::triggered, this, &MenuBar::citeWindNinja);
    connect(ui->supportEmailAction, &QAction::triggered, this, &MenuBar::supportEmail);
    connect(ui->submitBugReportAction, &QAction::triggered, this, &MenuBar::submitBugReport);
    connect(ui->aboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);
}

void MenuBar::newProject()
{
  qDebug() << "MenuBar: newProject() triggered";
}

void MenuBar::openProject()
{
  qDebug() << "MenuBar: openProject() triggered";
}

void MenuBar::exportSolution()
{
  qDebug() << "MenuBar: exportSolution() triggered";
}

void MenuBar::closeProject()
{
  qDebug() << "MenuBar: closeProject() triggered";
}

void MenuBar::enableConsoleOutput()
{
  qDebug() << "MenuBar: enableConsoleOutput() triggered";
}

void MenuBar::writeConsoleOutput()
{
  qDebug() << "MenuBar: writeConsoleOutput() triggered";
}

void MenuBar::resampleData()
{
  qDebug() << "MenuBar: resampleData() triggered";
}

void MenuBar::writeBlankStationFile()
{
  qDebug() << "MenuBar: writeBlankStationFile() triggered";
}

void MenuBar::setConfigurationOption()
{
  qDebug() << "MenuBar: setConfigurationOption() triggered";
}

void MenuBar::displayArcGISProGuide()
{
  qDebug() << "MenuBar: displayArcGISProGuide() triggered";
}

void MenuBar::displayTutorial1()
{
  qDebug() << "MenuBar: displayTutorial1() triggered";
}

void MenuBar::displayTutorial2()
{
  qDebug() << "MenuBar: displayTutorial2() triggered";
}

void MenuBar::displayTutorial3()
{
  qDebug() << "MenuBar: displayTutorial3() triggered";
}

void MenuBar::displayTutorial4()
{
  qDebug() << "MenuBar: displayTutorial4() triggered";
}

void MenuBar::displayDemDownloadInstructions()
{
  qDebug() << "MenuBar: displayDemDownloadInstructions() triggered";
}

void MenuBar::displayFetchDemInstructions()
{
  qDebug() << "MenuBar: displayFetchDemInstructions() triggered";
}

void MenuBar::displayCommandLineInterfaceInstructions()
{
  qDebug() << "MenuBar: displayCommandLineInterfaceInstructions() triggered";
}

void MenuBar::aboutWindNinja()
{
  qDebug() << "MenuBar: aboutWindNinja() triggered";
}

void MenuBar::citeWindNinja()
{
  qDebug() << "MenuBar: citeWindNinja() triggered";
}

void MenuBar::supportEmail()
{
  qDebug() << "MenuBar: supportEmail() triggered";
}

void MenuBar::submitBugReport()
{
  qDebug() << "MenuBar: submitBugReport() triggered";
}
