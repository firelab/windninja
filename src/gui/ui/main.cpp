
#include <cstdio>
#include <exception>

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char **argv) {
  int rc = 0;
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  rc = app.exec();
  return rc;
}
