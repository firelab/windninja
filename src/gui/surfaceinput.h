#ifndef SURFACEINPUT_H
#define SURFACEINPUT_H

#include "../ninja/gdal_util.h""
#include "../ninja/windninja.h"
#include <QDebug>
#include <QFile>

class SurfaceInput
{
public:
  SurfaceInput();

  QString fetchTimeZoneDetails(QString currentTimeZone);
  QVector<QVector<QString>> fetchAllTimeZones(bool isShowAllTimeZonesSelected);
  int fetchDEMFile(QVector<double> boundingBox, std::string demFile, double resolution, std::string fetchType);
  void computeDEMFile(QString filePath);
  double computeMeshResolution(int index, bool isMomemtumChecked);
  void computeBoundingBox(double centerLat, double centerLon, double radius, double boundingBox[4]);
  void computePointRadius(double north, double east, double south, double west, double pointRadius[3]);
  double* getDEMCorners();

private:
  QString GDALDriverName;
  int GDALXSize, GDALYSize;
  double GDALCellSize, GDALMaxValue, GDALMinValue;
  double DEMCorners[8];
};

#endif // SURFACEINPUT_H

