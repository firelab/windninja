#ifndef MENUBAR_H
#define MENUBAR_H

#include <QObject>

namespace Ui {
class MainWindow;
}

class MenuBar : public QObject
{
  Q_OBJECT
public:
  explicit MenuBar(Ui::MainWindow* ui, QObject* parent = nullptr);

private slots:
  // functions for Menu actions
  // functions for QMenu fileMenu "File" actions
  void newProject();
  void openProject();
  void exportSolution();
  void closeProject();
  // functions for QMenu optionsMenu "Options" actions
  void enableConsoleOutput();
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
};

#endif // MENUBAR_H
