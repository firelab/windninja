#include "mainwindow.h"
#include "../../ninja/windninja.h"
#include <QApplication>
#include <QTimer>
#include <QSplashScreen>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QMessageBox>
#include "controller.h"
#include "splashscreen.h"

int main(int argc, char *argv[]) {

  QApplication a(argc, argv);
  QIcon icon(":/wn-icon.png");
  QString ver = NINJA_VERSION_STRING;
  a.setWindowIcon(icon);
  a.setApplicationName(QString("WindNinja"));
  a.setApplicationVersion(ver);

  MainWindow* w = nullptr;
  try {
    w = new MainWindow;
  } catch (...) {
    QMessageBox::critical(nullptr, "Initialization Error",
                          "WindNinja failed to initialize. Most likely cause is failure to find data "
                          "dependencies. Try setting the environment variable WINDNINJA_DATA");
    return 1;
  }

  Controller controller(w);

  // Immediately pull timezone data
  QTimer::singleShot(0, w, &MainWindow::timeZoneDataRequest);

  QPixmap bigSplashPixmap(":wn-splash.png");
  QSize splashSize(1200, 320);
  QPixmap smallSplashPixmap;
  smallSplashPixmap = bigSplashPixmap.scaled(splashSize,
                                             Qt::KeepAspectRatioByExpanding);
  QStringList list;
  list << "Loading WindNinja " + ver + "...";
  list << "Loading mesh generator...";
  list << "Loading conjugate gradient solver...";
  list << "Loading preconditioner...";
  list << "WindNinja " + ver + " loaded.";

  SplashScreen *splash = new SplashScreen(smallSplashPixmap, list, 1000);
  splash->display();
  QObject::connect(splash, SIGNAL(done()), w, SLOT(show()));

  return a.exec();
}
