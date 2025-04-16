#include "mainwindow.h"
//#include "../../ninja/windninja.h"
#include <QApplication>
#include "modeldata.h"
#include "provider.h"
#include "datafetcher.h"
#include "vector"
#include "iostream"


int main(int argc, char *argv[]) {

  double bbox[4] = {40.07, -104.0, 40.0, -104.07};
  fetchDEMBoundingBox("/home/vboxuser/Documents/windninja/data/output.tif", "srtm", 30, bbox);

  fetchForecast("NOMADS-HRRR-CONUS-3-KM", 2, "/home/vboxuser/Documents/windninja/data/big_butte.tif");

  //fetchStationData(2023, 1, 1, 1, 1, "", "data/missoula_valley.tif", "UTC", 1);

  double cord[2] = {104.0, 40.0};
  double dim[2] = {330, 30};
  fetchFromDEMPoint(cord, dim, "mi", 30.0, "data/dem_point_output.tif", "gmted");

  std::vector<double> windSpeeds = {5.0, 10.0, 15.0};
  std::vector<double> windDirections = {0.0, 90.0, 180.0};

  //DomainAverageWind windModel("/home/vboxuser/Documents/windninja/data/big_butte.tif", 100, "domain_average", "coarse", "grass", 20, 0, 10.0, "m" , 0, 2, "/home/vboxuser/Documents/windninja/data", windSpeeds, "mps", windDirections);

  //NinjaSim sim(windModel);

  /*QApplication a(argc, argv);
  MainWwindow w;
  w.show();
  return a.exec();*/

  return 0;
}
