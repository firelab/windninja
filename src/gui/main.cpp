#include "mainwindow.h"
#include "../ninja/windninja.h"
#include <QApplication>
#include <QTimer>
// #include "modeldata.h"
// #include "provider.h"
#include "controller.h"


#ifdef _OPENMP
omp_lock_t netCDF_lock;
#endif

int main(int argc, char *argv[]) {

  int result;
  #ifdef _OPENMP
  omp_init_lock (&netCDF_lock);
  #endif

  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions);
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
    #ifdef _OPENMP
    omp_destroy_lock (&netCDF_lock);
    #endif
  }

  QApplication a(argc, argv);
  MainWindow w;

  Controller controller(&w);

  // Immediately pull timezone data
  QTimer::singleShot(0, &w, &MainWindow::timeZoneDataRequest);

  w.show();
  result = a.exec();

  #ifdef _OPENMP
  omp_destroy_lock (&netCDF_lock);
  #endif

  return result;
}
