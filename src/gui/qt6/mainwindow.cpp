#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

}

MainWindow::~MainWindow() { delete ui; }
