#ifndef SURFACEINPUT_H
#define SURFACEINPUT_H

#include <qfile.h>
#include "gdal_utils.h"
#include "gdal_priv.h"
#include "../ninja/windninja.h"

class SurfaceInput
{
public:
  SurfaceInput();

  QString fetchTimeZoneDetails(QString currentTimeZone);
  QVector<QVector<QString>> fetchAllTimeZones(bool isShowAllTimeZonesSelected);
  int fetchDEMFile(double boundingBox[], std::string demFile, double resolution, std::string fetchType);
  int computeDEMFile(QString filePath);
  double computeMeshResolution(int index, bool isMomemtumChecked);
  void computeBoundingBox(double centerLat, double centerLon, double radius, double boundingBox[4]);
  void computePointRadius(double north, double east, double south, double west, double pointRadius[3]);

private:
  QString GDALDriverName;
  int GDALXSize, GDALYSize;
  double GDALCellSize, GDALMaxValue, GDALMinValue;
};

#endif // SURFACEINPUT_H

