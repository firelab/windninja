#ifndef SURFACEINPUT_H
#define SURFACEINPUT_H

#include <qfile.h>
#include "../ninja/windninja.h"

class SurfaceInput
{
public:
  SurfaceInput();

  QString fetchTimeZoneDetails(QString currentTimeZone);
  QVector<QVector<QString>> fetchAllTimeZones(bool isShowAllTimeZonesSelected);
  int fetchDEMFile(double boundingBox[], std::string demFile, double resolution, std::string fetchType);

private:

};

#endif // SURFACEINPUT_H

