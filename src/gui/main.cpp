#include "mainwindow.h"
#include "../ninja/windninja.h"
#include <QApplication>
#include <QTimer>
// #include "modeldata.h"
// #include "provider.h"
#include "controller.h"


int main(int argc, char *argv[]) {

  QApplication a(argc, argv);
  MainWindow w;

  Controller controller(&w);

  // Immediately pull timezone data
  QTimer::singleShot(0, &w, &MainWindow::timeZoneDataRequest);

  w.show();
  return a.exec();
}
