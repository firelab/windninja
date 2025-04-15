#include "mainwindow.h"
#include "../../ninja/windninja.h"
#include <QApplication>
#include "modeldata.h"
#include "provider.h"
#include "vector"
#include "iostream"


int main(int argc, char *argv[]) {

  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();

}
