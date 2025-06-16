#include "mainwindow.h"
#include "../../ninja/windninja.h"
#include <QApplication>
#include <QTimer>
#include <QSplashScreen>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
// #include "modeldata.h"
// #include "provider.h"
#include "controller.h"


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

  // Load and scale splash image
  QPixmap bigPixmap(":/wn-splash.png");
  QSize splashSize(1200, 320);
  QPixmap splashPixmap = bigPixmap.scaled(splashSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

  // Create splash screen
  QSplashScreen* splash = new QSplashScreen(splashPixmap);
  splash->setFont(QFont("Sans Serif", 10));
  QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
  QPoint center = screenGeometry.center() - splash->rect().center();
  splash->move(center);

  // Apply opacity effect
  QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(splash);
  splash->setGraphicsEffect(effect);
  effect->setOpacity(0.0); // Initial opacity 0

  // Show splash screen
  splash->show();
  a.processEvents(); // Ensure splash is shown before animating

  // Fade-in animation
  QPropertyAnimation* fadeIn = new QPropertyAnimation(effect, "opacity");
  fadeIn->setDuration(1500);
  fadeIn->setStartValue(0.0);
  fadeIn->setEndValue(1.0);

  // Fade-out animation
  QPropertyAnimation* fadeOut = new QPropertyAnimation(effect, "opacity");
  fadeOut->setDuration(1500);
  fadeOut->setStartValue(1.0);
  fadeOut->setEndValue(0.0);

  // Sequence: After fade-in completes, wait a bit, then start fade-out
  QObject::connect(fadeIn, &QPropertyAnimation::finished, [=]() {
    QTimer::singleShot(1500, [=]() {
      fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
  });

  // After fade-out completes, show main window and clean up
  QObject::connect(fadeOut, &QPropertyAnimation::finished, [=]() {
    splash->finish(w);
    w->show();
    splash->deleteLater();
  });

  // Start fade-in
  fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

  return a.exec();

}
