#include "mainwindow.h"
#include "../../ninja/windninja.h"
#include <QApplication>
#include "modeldata.h"
#include "provider.h"
#include "vector"
#include "iostream"


int main(int argc, char *argv[]) {

  std::vector<double> windSpeeds = {5.0, 10.0, 15.0};
  std::vector<double> windDirections = {0.0, 90.0, 180.0};

  DomainAverageWind windModel("/home/vboxuser/Documents/windninja/data/big_butte.tif", 100, "domain_average", "coarse", "grass", 20, 0, 10.0, "m" , 0, 2, windSpeeds, "mps", windDirections);

  NinjaSim sim(windModel);

  /*QApplication a(argc, argv);
  MainWwindow w;
  w.show();
  return a.exec();*/

  return 0;
}
