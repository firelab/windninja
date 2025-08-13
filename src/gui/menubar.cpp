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
    QString dataFolder = QString::fromUtf8(CPLGetConfigOption("WINDNINJA_DATA", ""));
    dataPath = QDir(dataFolder);

    // QMenu fileMenu "File" actions
    connect(ui->newProjectAction, &QAction::triggered, this, &MenuBar::newProject);
    connect(ui->openProjectAction, &QAction::triggered, this, &MenuBar::openProject);
    connect(ui->exportSolutionAction, &QAction::triggered, this, &MenuBar::exportSolution);
    connect(ui->closeProjectAction, &QAction::triggered, this, &MenuBar::closeProject);
    connect(ui->openElevationInputFileMenuAction, &QAction::triggered, this, &MenuBar::openElevationInputFileMenuActionTriggered);
    connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QCoreApplication::quit);  // exit the entire app

    // QMenu optionsMenu "Options" actions
    connect(ui->enableConsoleOutputAction, &QAction::toggled, ui->consoleDockWidget, &QDockWidget::setVisible);
    connect(ui->consoleDockWidget, &QDockWidget::visibilityChanged, ui->enableConsoleOutputAction, &QAction::setChecked);  // if closed from the QDockWidget itself, unchecks the menuAction
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
    ui->consoleTextEdit->append("MenuBar: newProject() triggered");
}

void MenuBar::openProject()
{
    qDebug() << "MenuBar: openProject() triggered";
    ui->consoleTextEdit->append("MenuBar: openProject() triggered");
}

void MenuBar::exportSolution()
{
    qDebug() << "MenuBar: exportSolution() triggered";
    ui->consoleTextEdit->append("MenuBar: exportSolution() triggered");
}

void MenuBar::closeProject()
{
    qDebug() << "MenuBar: closeProject() triggered";
    ui->consoleTextEdit->append("MenuBar: closeProject() triggered");
}

void MenuBar::writeConsoleOutput()
{
    QString fileName = QFileDialog::getSaveFileName(ui->centralwidget,
                tr("Save Console Output"),
                "console-output.txt",
                tr("Text Files (*.txt)"));

    if(!fileName.isEmpty())
    {
        QDateTime currentTime(QDateTime::currentDateTime());
        ui->consoleTextEdit->append("writing WindNinja console output to " + fileName);
        ui->consoleTextEdit->append("current time is " + currentTime.toString("MM/dd/yyyy hh:mm:ss t"));

        std::ofstream fout(fileName.toStdString().c_str(), std::ios::out);
        QString text = ui->consoleTextEdit->toPlainText();
        fout << text.toStdString();
        fout.close();
    }
}

void MenuBar::resampleData()
{
    qDebug() << "MenuBar: resampleData() triggered";
    ui->consoleTextEdit->append("MenuBar: resampleData() triggered");
}

void MenuBar::writeBlankStationFile()
{
    qDebug() << "MenuBar: writeBlankStationFile() triggered";
    ui->consoleTextEdit->append("MenuBar: writeBlankStationFile() triggered");
}

void MenuBar::setConfigurationOption()
{
    qDebug() << "MenuBar: setConfigurationOption() triggered";
    ui->consoleTextEdit->append("MenuBar: setConfigurationOption() triggered");
}

void MenuBar::displayArcGISProGuide()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/displaying_wind_vectors_in_ArcGIS_Pro.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial1()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial1.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial2()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial2.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial3()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial3.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial4()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial4.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayDemDownloadInstructions()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/download_elevation_file.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayFetchDemInstructions()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/fetch_dem_instructions.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayCommandLineInterfaceInstructions()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/CLI_instructions.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    qDebug() << "Opening" << displayFile;
    ui->consoleTextEdit->append("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::aboutWindNinja()
{
    QString aboutText = "<h2>WindNinja</h2>\n";

    aboutText.append("<p><h4>Version:</h4>" + QString(NINJA_VERSION_STRING) + "</p>");

    aboutText.append("<p><h4>Git Commit:</h4>" + QString(NINJA_SCM_VERSION) + "</p>");

    aboutText.append("<p><h4>Release Date:</h4>" + QString(NINJA_RELEASE_DATE) + "</p>");

    aboutText.append("<p><h4>Developed by:</h4><p>Jason Forthofer<br/> " \
                                                 "Natalie Wagenbrenner<br/>" \
                                                 "Kyle Shannon<br/>" \
                                                 "Loren Atwood<br/>" \
                                                 "Mason Willman"); \

    aboutText.append("<p>Missoula Fire Sciences Laboratory<br/>");
    aboutText.append("Rocky Mountain Research Station<br/>");
    aboutText.append("USDA Forest Service<br/>");
    aboutText.append("5775 Highway 10 W.<br/>");
    aboutText.append("Missoula, MT 59808</p>");

    aboutText.append("<p><a href=\"https://github.com/firelab/windninja/blob/master/CONTRIBUTORS\">Contributors</a></p>");
    aboutText.append("<h4>Sponsored By:</h4>");
    aboutText.append("USDA Forest Service<br/>");
    aboutText.append("Center for Environmental Management of Military Lands at Colorado State University<br/>");
    aboutText.append("Joint Fire Sciences Program<br/>");
    aboutText.append("Washington State University</p>");
    aboutText.append("<p><a href=\"https://github.com/firelab/windninja/blob/master/CREDITS.md\">Special Thanks</a></p>");
    aboutText.append("<br/>");

    QMessageBox::about(ui->centralwidget, tr("About WindNinja"),
                aboutText);
}

void MenuBar::citeWindNinja()
{
    QString citeText = "<h4>To cite WindNinja in a publication use:</h4>";

    citeText.append("Forthofer, J.M., Butler, B.W., Wagenbrenner, N.S., 2014. A comparison ");
    citeText.append("of three approaches for simulating fine-scale surface winds in ");
    citeText.append("support of wildland fire management. Part I. Model formulation and ");
    citeText.append("comparison against measurements. International Journal of Wildland ");
    citeText.append("Fire, 23:969-931. doi: 10.1071/WF12089.");

    citeText.append("<h4>For additional WindNinja publications visit:</h4>");

    citeText.append("<p><a href=\"https://ninjastorm.firelab.org/windninja/publications/\">https://ninjastorm.firelab.org/windninja/publications</a></p>");

    QMessageBox::about(ui->centralwidget, tr("Cite WindNinja"),
            citeText);
}

void MenuBar::supportEmail()
{
    QDesktopServices::openUrl(QUrl("mailto:wind.ninja.support@gmail.com?subject=[windninja-support]"));
}

void MenuBar::submitBugReport()
{
    QDesktopServices::openUrl(QUrl("https://github.com/firelab/windninja/issues/new"));
}
