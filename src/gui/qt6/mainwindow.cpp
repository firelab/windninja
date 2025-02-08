#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPainterPath>
#include <QRegion>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Remove window frame
  setWindowFlags(Qt::FramelessWindowHint);

  // **Enable transparency while keeping it visible**
  setAttribute(Qt::WA_TranslucentBackground);

  // Ensure a minimum size so the window is visible
  setMinimumSize(400, 300);
  resize(600, 400);

  // Center the window on the screen**
  QScreen *screen = QGuiApplication::primaryScreen();
  if (screen) {
      QRect screenGeometry = screen->geometry();
      int x = (screenGeometry.width() - width()) / 2;
      int y = (screenGeometry.height() - height()) / 2;
      move(x, y);
  }

  // **Apply a semi-transparent background with rounded corners**
  setStyleSheet("background-color: rgba(255, 255, 255, 220); border-radius: 20px;");

  // Apply initial rounded mask after setting size
  applyRoundedCorners();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    applyRoundedCorners();
}

void MainWindow::applyRoundedCorners() {
    if (width() == 0 || height() == 0) {
        return;
    }

    int radius = 20;
    QPainterPath path;
    path.addRoundedRect(rect(), radius, radius);
    setMask(QRegion(path.toFillPolygon().toPolygon()));
}
