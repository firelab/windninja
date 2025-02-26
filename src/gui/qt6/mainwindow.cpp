#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>
#include <QtWebEngineWidgets/qwebengineview.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
  ui->setupUi(this);

  //Load HTML file with Leaflet
  webView = new QWebEngineView(this);
  QString filePath = QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/data/map.html").toString();
  webView->setUrl(QUrl(filePath));

  // Set up layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(webView);

  // Apply
  ui->mapPanelWidget->setLayout(layout);


}

MainWindow::~MainWindow() { delete ui; }
