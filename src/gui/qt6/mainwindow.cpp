#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QWebEngineProfile>
#include <QWebEngineSettings>


// Menu filtering class
class DirectoryFilterModel : public QSortFilterProxyModel {
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
    QFileSystemModel *fsModel = qobject_cast<QFileSystemModel *>(sourceModel());
    if (!fsModel) return false;

    QModelIndex index = fsModel->index(source_row, 0, source_parent);
    if (!index.isValid()) return false;

    // Define the download path
    QFileInfo fileInfo = fsModel->fileInfo(index);
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    // Keep the Downloads root directory
    if (fileInfo.absoluteFilePath() == downloadsPath) {
      return true;
    }

    // Ensure filtering applies only inside Downloads
    if (!fileInfo.absoluteFilePath().startsWith(downloadsPath)) {
      return false;
    }

    // Allow `WXSTATIONS-*` directories
    if (fileInfo.isDir() && fileInfo.fileName().toLower().startsWith("wxstations")) {
      return true;
    }

    // Allow files **inside** `WXSTATIONS-*`
    QModelIndex parentIndex = index.parent();
    if (parentIndex.isValid()) {
      QFileInfo parentInfo = fsModel->fileInfo(parentIndex);
      if (parentInfo.isDir() && parentInfo.fileName().toLower().startsWith("wxstations")) {
        return true;
      }
    }

    return false;
  }
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Set default window size
  resize(1200, 700);

  /*
   *
   * Create file handler window for point init screen
   *
   */

  // Get the correct Downloads folder path
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

  // Enable QFileSystemModel to process directories and files
  QFileSystemModel *model = new QFileSystemModel(this);
  model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::AllEntries);  // Ensure files appear
  model->setRootPath(downloadsPath);

  // Enable file watching so contents refresh properly
  model->setReadOnly(false);
  model->setResolveSymlinks(true);

  // Create a filtering model
  DirectoryFilterModel *filterModel = new DirectoryFilterModel();
  filterModel->setSourceModel(model);

  // Set the correct root index inside Downloads
  QModelIndex rootIndex = model->index(downloadsPath);
  ui->treeFileExplorer->setModel(filterModel);
  ui->treeFileExplorer->setRootIndex(filterModel->mapFromSource(rootIndex));

  // Ensure folders expand and collapse correctly
  ui->treeFileExplorer->setExpandsOnDoubleClick(true);
  ui->treeFileExplorer->setAnimated(true);
  ui->treeFileExplorer->setIndentation(15);
  ui->treeFileExplorer->setSortingEnabled(true);
  ui->treeFileExplorer->setItemsExpandable(true);
  ui->treeFileExplorer->setUniformRowHeights(true);

  // Show only "Name" and "Date Modified" columns
  ui->treeFileExplorer->hideColumn(1);  // Hide Size column
  ui->treeFileExplorer->hideColumn(2);  // Hide Type column

  // Optional: Set column headers
  QHeaderView *header = ui->treeFileExplorer->header();
  header->setSectionResizeMode(0, QHeaderView::Interactive);  // Name fits content
  header->setSectionResizeMode(3, QHeaderView::Stretch);           // Date Modified stretches
  model->setHeaderData(0, Qt::Horizontal, "Name");
  model->setHeaderData(3, Qt::Horizontal, "Date Modified");

  // Force model to reload children
  ui->treeFileExplorer->expandAll();  // Force expand all to check visibility

  // Get project root directory
  QDir projectRoot(QCoreApplication::applicationDirPath());
  while (!projectRoot.exists("data")) {
    if (!projectRoot.cdUp()) break;
  }

  /*
   *
   * Functionality for the map widget
   *
   */

  // Resolve the map file path
  QString filePath = projectRoot.filePath("data/map.html");

  // Enable remote content
  QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
  QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

  //Load HTML file with Leaflet
  webView = new QWebEngineView(this);
  QUrl url = QUrl::fromLocalFile(filePath);
  webView->setUrl(url);

  // Set up layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(webView);

  // Apply
  ui->mapPanelWidget->setLayout(layout);

  /*
   *
   * Connect tree items to stacked tab window
   *
   */

  // Top-level items
  ui->stackedInputPage->setCurrentIndex(0);
  ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, 1);  // Solver Methodology (Page 0)
  ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, 4);  // Inputs (Page 4)

  // Sub-items for Solver Methodology
  ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, 2);  // Conservation of Mass (Page 1)
  ui->treeWidget->topLevelItem(0)->child(1)->setData(0, Qt::UserRole, 3);  // Conservation of Mass and Momentum (Page 2)

  // Sub-items for Inputs
  ui->treeWidget->topLevelItem(1)->child(0)->setData(0, Qt::UserRole, 5);  // Surface Input (Page 5)
  ui->treeWidget->topLevelItem(1)->child(1)->setData(0, Qt::UserRole, 6);  // Dirunal Input (Page 6)
  ui->treeWidget->topLevelItem(1)->child(2)->setData(0, Qt::UserRole, 7);  // Stability Input (Page 7)
  ui->treeWidget->topLevelItem(1)->child(3)->setData(0, Qt::UserRole, 8);  // Wind Input (Page 8)

  // Sub-sub-items for Wind Input
  QTreeWidgetItem *windInputItem = ui->treeWidget->topLevelItem(1)->child(3);
  windInputItem->child(0)->setData(0, Qt::UserRole, 9);  // Domain Average Wind (Page 9)
  windInputItem->child(1)->setData(0, Qt::UserRole, 10); // Point Init (Page 10)
  windInputItem->child(2)->setData(0, Qt::UserRole, 11); // Weather Model (Page 11)

  connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
}

MainWindow::~MainWindow() { delete ui; }

/*
 *
 * Click tree item helper function
 *
 */

void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column) {
  int pageIndex = item->data(column, Qt::UserRole).toInt();
  if (pageIndex >= 0) {
    ui->stackedInputPage->setCurrentIndex(pageIndex);
  }
}
