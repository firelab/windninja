#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDir>
#include <QDebug>
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QtWebEngineWidgets/qwebengineview.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
  ui->setupUi(this);

  //Load HTML file with Leaflet
  webView = new QWebEngineView(this);
  QString filePath = QUrl::fromLocalFile("home/vboxuser/Documents/windninja/data/map.html").toString();
  qDebug() << QDir::currentPath();
  qDebug() << filePath;
  webView->setUrl(QUrl(filePath));

  // Set up layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(webView);

  // Apply
  ui->mapPanelWidget->setLayout(layout);


}

MainWindow::~MainWindow() { delete ui; }
