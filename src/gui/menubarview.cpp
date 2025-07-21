#include "menubarview.h"
#include "ui_mainwindow.h"

MenuBarView::MenuBarView(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent), ui(ui)
{
    // QMenu fileMenu "File" actions
    connect(ui->newProjectAction, &QAction::triggered, this, &MenuBarView::newProject);
    connect(ui->openProjectAction, &QAction::triggered, this, &MenuBarView::openProject);
    connect(ui->exportSolutionAction, &QAction::triggered, this, &MenuBarView::exportSolution);
    connect(ui->closeProjectAction, &QAction::triggered, this, &MenuBarView::closeProject);
    //connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QCoreApplication::quit);  // exit the entire app
    // QMenu optionsMenu "Options" actions
    connect(ui->enableConsoleOutputAction, &QAction::triggered, this, &MenuBarView::enableConsoleOutput);
    connect(ui->writeConsoleOutputAction, &QAction::triggered, this, &MenuBarView::writeConsoleOutput);

    // QMenu toolsMenu "Tools" actions
    connect(ui->resampleDataAction, &QAction::triggered, this, &MenuBarView::resampleData);
    connect(ui->writeBlankStationFileAction, &QAction::triggered, this, &MenuBarView::writeBlankStationFile);
    connect(ui->setConfigurationOptionAction, &QAction::triggered, this, &MenuBarView::setConfigurationOption);

    // QMenu helpMenu "Help" actions
    // sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
    connect(ui->displayArcGISProGuideAction, &QAction::triggered, this, &MenuBarView::displayArcGISProGuide);

    // sub QMenu tutorialsMenu "Tutorials" actions
    connect(ui->displayTutorial1Action, &QAction::triggered, this, &MenuBarView::displayTutorial1);
    connect(ui->displayTutorial2Action, &QAction::triggered, this, &MenuBarView::displayTutorial2);
    connect(ui->displayTutorial3Action, &QAction::triggered, this, &MenuBarView::displayTutorial3);
    connect(ui->displayTutorial4Action, &QAction::triggered, this, &MenuBarView::displayTutorial4);

    // sub QMenu instructionsMenu "Instructions" actions
    connect(ui->displayDemDownloadInstructionsAction, &QAction::triggered, this, &MenuBarView::displayDemDownloadInstructions);
    connect(ui->displayFetchDemInstructionsAction, &QAction::triggered, this, &MenuBarView::displayFetchDemInstructions);
    connect(ui->displayCommandLineInterfaceInstructionsAction, &QAction::triggered, this, &MenuBarView::displayCommandLineInterfaceInstructions);

    // remaining non-sub QMenu actions
    connect(ui->aboutWindNinjaAction, &QAction::triggered, this, &MenuBarView::aboutWindNinja);
    connect(ui->citeWindNinjaAction, &QAction::triggered, this, &MenuBarView::citeWindNinja);
    connect(ui->supportEmailAction, &QAction::triggered, this, &MenuBarView::supportEmail);
    connect(ui->submitBugReportAction, &QAction::triggered, this, &MenuBarView::submitBugReport);
    connect(ui->aboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);
}

void MenuBarView::newProject()
{
  qDebug() << "MenuBarView: newProject() triggered";
}

void MenuBarView::openProject()
{
  qDebug() << "MenuBarView: openProject() triggered";
}

void MenuBarView::exportSolution()
{
  qDebug() << "MenuBarView: exportSolution() triggered";
}

void MenuBarView::closeProject()
{
  qDebug() << "MenuBarView: closeProject() triggered";
}

void MenuBarView::enableConsoleOutput()
{
  qDebug() << "MenuBarView: enableConsoleOutput() triggered";
}

void MenuBarView::writeConsoleOutput()
{
  qDebug() << "MenuBarView: writeConsoleOutput() triggered";
}

void MenuBarView::resampleData()
{
  qDebug() << "MenuBarView: resampleData() triggered";
}

void MenuBarView::writeBlankStationFile()
{
  qDebug() << "MenuBarView: writeBlankStationFile() triggered";
}

void MenuBarView::setConfigurationOption()
{
  qDebug() << "MenuBarView: setConfigurationOption() triggered";
}

void MenuBarView::displayArcGISProGuide()
{
  qDebug() << "MenuBarView: displayArcGISProGuide() triggered";
}

void MenuBarView::displayTutorial1()
{
  qDebug() << "MenuBarView: displayTutorial1() triggered";
}

void MenuBarView::displayTutorial2()
{
  qDebug() << "MenuBarView: displayTutorial2() triggered";
}

void MenuBarView::displayTutorial3()
{
  qDebug() << "MenuBarView: displayTutorial3() triggered";
}

void MenuBarView::displayTutorial4()
{
  qDebug() << "MenuBarView: displayTutorial4() triggered";
}

void MenuBarView::displayDemDownloadInstructions()
{
  qDebug() << "MenuBarView: displayDemDownloadInstructions() triggered";
}

void MenuBarView::displayFetchDemInstructions()
{
  qDebug() << "MenuBarView: displayFetchDemInstructions() triggered";
}

void MenuBarView::displayCommandLineInterfaceInstructions()
{
  qDebug() << "MenuBarView: displayCommandLineInterfaceInstructions() triggered";
}

void MenuBarView::aboutWindNinja()
{
  qDebug() << "MenuBarView: aboutWindNinja() triggered";
}

void MenuBarView::citeWindNinja()
{
  qDebug() << "MenuBarView: citeWindNinja() triggered";
}

void MenuBarView::supportEmail()
{
  qDebug() << "MenuBarView: supportEmail() triggered";
}

void MenuBarView::submitBugReport()
{
  qDebug() << "MenuBarView: submitBugReport() triggered";
}
