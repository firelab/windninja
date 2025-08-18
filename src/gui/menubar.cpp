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
    connect(ui->newProjectAction, &QAction::triggered, this, &MenuBar::newProjectActionTriggered);
    connect(ui->openProjectAction, &QAction::triggered, this, &MenuBar::openProjectActionTriggered);
    connect(ui->exportSolutionAction, &QAction::triggered, this, &MenuBar::exportSolutionActionTriggered);
    connect(ui->closeProjectAction, &QAction::triggered, this, &MenuBar::closeProjectActionTriggered);
    connect(ui->openElevationInputFileAction, &QAction::triggered, this, &MenuBar::openElevationInputFileActionTriggered);
    connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QCoreApplication::quit);  // exit the entire app

    // QMenu optionsMenu "Options" actions
    connect(ui->enableConsoleOutputAction, &QAction::toggled, ui->consoleDockWidget, &QDockWidget::setVisible);
    connect(ui->consoleDockWidget, &QDockWidget::visibilityChanged, ui->enableConsoleOutputAction, &QAction::setChecked);  // if closed from the QDockWidget itself, unchecks the menuAction
    connect(ui->writeConsoleOutputAction, &QAction::triggered, this, &MenuBar::writeConsoleOutputActionTriggered);

    // QMenu toolsMenu "Tools" actions
    connect(ui->resampleDataAction, &QAction::triggered, this, &MenuBar::resampleDataActionTriggered);
    connect(ui->writeBlankStationFileAction, &QAction::triggered, this, &MenuBar::writeBlankStationFileActionTriggered);
    connect(ui->setConfigurationOptionAction, &QAction::triggered, this, &MenuBar::setConfigurationOptionActionTriggered);

    // QMenu helpMenu "Help" actions
    // sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
    connect(ui->displayArcGISProGuideAction, &QAction::triggered, this, &MenuBar::displayArcGISProGuideActionTriggered);

    // sub QMenu tutorialsMenu "Tutorials" actions
    connect(ui->displayTutorial1Action, &QAction::triggered, this, &MenuBar::displayTutorial1ActionTriggered);
    connect(ui->displayTutorial2Action, &QAction::triggered, this, &MenuBar::displayTutorial2ActionTriggered);
    connect(ui->displayTutorial3Action, &QAction::triggered, this, &MenuBar::displayTutorial3ActionTriggered);
    connect(ui->displayTutorial4Action, &QAction::triggered, this, &MenuBar::displayTutorial4ActionTriggered);

    // sub QMenu instructionsMenu "Instructions" actions
    connect(ui->displayDemDownloadInstructionsAction, &QAction::triggered, this, &MenuBar::displayDemDownloadInstructionsActionTriggered);
    connect(ui->displayFetchDemInstructionsAction, &QAction::triggered, this, &MenuBar::displayFetchDemInstructionsActionTriggered);
    connect(ui->displayCommandLineInterfaceInstructionsAction, &QAction::triggered, this, &MenuBar::displayCommandLineInterfaceInstructionsActionTriggered);

    // remaining non-sub QMenu actions
    connect(ui->aboutWindNinjaAction, &QAction::triggered, this, &MenuBar::aboutWindNinjaActionTriggered);
    connect(ui->citeWindNinjaAction, &QAction::triggered, this, &MenuBar::citeWindNinjaActionTriggered);
    connect(ui->supportEmailAction, &QAction::triggered, this, &MenuBar::supportEmailActionTriggered);
    connect(ui->submitBugReportAction, &QAction::triggered, this, &MenuBar::submitBugReportActionTriggered);
    connect(ui->aboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);
}

void MenuBar::newProjectActionTriggered()
{
    writeToConsole("MenuBar: newProject() triggered");
}

void MenuBar::openProjectActionTriggered()
{
    writeToConsole("MenuBar: openProject() triggered");
}

void MenuBar::exportSolutionActionTriggered()
{
    writeToConsole("MenuBar: exportSolution() triggered");
}

void MenuBar::closeProjectActionTriggered()
{
    writeToConsole("MenuBar: closeProject() triggered");
}

void MenuBar::writeConsoleOutputActionTriggered()
{
    QString fileName = QFileDialog::getSaveFileName(ui->centralwidget,
                tr("Save Console Output"),
                "console-output.txt",
                tr("Text Files (*.txt)"));

    if(!fileName.isEmpty())
    {
        QDateTime currentTime(QDateTime::currentDateTime());
        writeToConsole("writing console output to " + fileName, Qt::darkGreen);
        writeToConsole("current time is " + currentTime.toString("MM/dd/yyyy hh:mm:ss t"), Qt::darkGreen);

        std::ofstream fout(fileName.toStdString().c_str(), std::ios::out);
        if(!fout)
        {
            writeToConsole("Cannot open " + fileName + " for writing.", Qt::red);
            return;
        }

        QTextDocument *doc = ui->consoleTextEdit->document();
        QTextBlock block = doc->begin();
        while( block.isValid() )
        {
            fout << block.text().toStdString() << "\n";
            block = block.next();
        }
        fout.close();
    }
}

void MenuBar::resampleDataActionTriggered()
{
    writeToConsole("MenuBar: resampleData() triggered");
}

void MenuBar::writeBlankStationFileActionTriggered()
{
    QString fileName = QFileDialog::getSaveFileName(ui->centralwidget,
                tr("Save Blank Station File"),
                "stations.csv",
                tr("Text Files (*.csv)"));

    if(!fileName.isEmpty())
    {
        writeToConsole("writing blank station file to " + fileName, Qt::darkGreen);

        char** papszOptions = nullptr;
        int err = NinjaWriteBlankWxStationFile( fileName.toStdString().c_str(), papszOptions );
        if( err != NINJA_SUCCESS )
        {
            writeToConsole("failed to write blank station file!", Qt::red);
        }
    }
}

void MenuBar::setConfigurationOptionActionTriggered()
{
    setConfigurationOptionDialog configDialog;

    int rc = configDialog.exec();
    if( rc == QDialog::Rejected )
    {
        return;
    }

    const char *pszKey, *pszVal;
    QString key = configDialog.GetKey();
    QString val = configDialog.GetValue();
    if( key == "" )
    {
        return;
    }
    if( val == "" )
    {
        pszVal = NULL;
    }
    else
    {
        pszVal = CPLSPrintf( "%s", (char*)val.toLocal8Bit().data() );
    }

    qDebug() << "Setting configuration option " << key << "to" << val;
    writeToConsole("Setting configuration option " + key + " to " + val);

    pszKey = CPLSPrintf( "%s", (char*)key.toLocal8Bit().data() );
    CPLSetConfigOption( pszKey, pszVal );
}

void MenuBar::displayArcGISProGuideActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/displaying_wind_vectors_in_ArcGIS_Pro.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial1ActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial1.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial2ActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial2.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial3ActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial3.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayTutorial4ActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/tutorials/WindNinja_tutorial4.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayDemDownloadInstructionsActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/download_elevation_file.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayFetchDemInstructionsActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/fetch_dem_instructions.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::displayCommandLineInterfaceInstructionsActionTriggered()
{
    QString displayFile = dataPath.absoluteFilePath("../doc/CLI_instructions.pdf");
    displayFile = QDir().cleanPath(displayFile);  // cleanup the file path, make it a truly absolute path
    writeToConsole("Opening " + displayFile);

    if(!QDesktopServices::openUrl(QUrl(displayFile)))
    {
        QMessageBox::warning(ui->centralwidget, tr("Broken Link."),
                tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                QMessageBox::Ok);
    }
}

void MenuBar::aboutWindNinjaActionTriggered()
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

void MenuBar::citeWindNinjaActionTriggered()
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

void MenuBar::supportEmailActionTriggered()
{
    QDesktopServices::openUrl(QUrl("mailto:wind.ninja.support@gmail.com?subject=[windninja-support]"));
}

void MenuBar::submitBugReportActionTriggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/firelab/windninja/issues/new"));
}
