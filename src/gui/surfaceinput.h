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
  double computeMeshResolution(int index);

private:
  QString GDALDriverName, GDALDriverLongName;
  int GDALXSize, GDALYSize;
  double GDALCellSize, GDALNoData;
  double GDALMaxValue, GDALMinValue;


};

#endif // SURFACEINPUT_H

