#include "surfaceinput.h"

SurfaceInput::SurfaceInput()
{
}

QString SurfaceInput::fetchTimeZoneDetails(QString currentTimeZone) {
  QVector<QString> matchedRow;
  QFile file(":/date_time_zonespec.csv");

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Failed to open date_time_zonespec.csv";
    qDebug() << "No data found";
  }

  QTextStream in(&file);
  bool firstLine = true;

  while (!in.atEnd()) {
    QString line = in.readLine();

    if (firstLine) {
      firstLine = false;
      continue;  // skip header
    }

    QStringList tokens = line.split(",", Qt::KeepEmptyParts);
    QVector<QString> row;

    for (const QString& token : tokens)
      row.append(token.trimmed().remove("\""));

    QString fullZone = row.mid(0, 1).join("/");

    if (fullZone == currentTimeZone) {
      matchedRow = row;
      break;
    }
  }

  file.close();

  if (matchedRow.isEmpty()) {
    qDebug() << "No matching time zone found.";
  }

  QString standardName = matchedRow.value(2);
  QString daylightName = matchedRow.value(4);
  QString stdOffsetStr = matchedRow.value(5);
  QString dstAdjustStr = matchedRow.value(6);

  auto timeToSeconds = [](const QString& t) -> int {
    QString s = t.trimmed();
    bool negative = s.startsWith('-');
    s = s.remove(QChar('+')).remove(QChar('-'));

    QStringList parts = s.split(':');
    if (parts.size() != 3) return 0;

    int h = parts[0].toInt();
    int m = parts[1].toInt();
    int sec = parts[2].toInt();

    int total = h * 3600 + m * 60 + sec;
    return negative ? -total : total;
  };

         // Convert total seconds back to HH:MM:SS with sign
  auto secondsToTime = [](int totalSec) -> QString {
    QChar sign = totalSec < 0 ? '-' : '+';
    totalSec = std::abs(totalSec);

    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;

    return QString("%1%2:%3:%4")
        .arg(sign)
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
  };

  int stdSecs = timeToSeconds(stdOffsetStr);
  int dstSecs = timeToSeconds(dstAdjustStr);
  QString combinedOffsetStr = secondsToTime(stdSecs + dstSecs);

  return QString("Standard Name:\t\t%1\n"
                                    "Daylight Name:\t\t%2\n"
                                    "Standard Offset from UTC:\t%3\n"
                                    "Daylight Offset from UTC:\t%4")
                                .arg(standardName)
                                .arg(daylightName)
                                .arg(stdOffsetStr)
                                .arg(combinedOffsetStr);

}

QVector<QVector<QString>> SurfaceInput::fetchAllTimeZones(bool isShowAllTimeZonesSelected)
{
  QVector<QVector<QString>> fullData;
  QVector<QVector<QString>> americaData;

  QFile file(":/date_time_zonespec.csv");

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open CSV file.";
    return fullData;
  }

  QTextStream in(&file);
  bool firstLine = true;

  while (!in.atEnd()) {
    QString line = in.readLine();

    if (firstLine) {
      firstLine = false;
      continue;
    }

    QStringList tokens = line.split(",", Qt::KeepEmptyParts);
    QVector<QString> row;
    for (const QString& token : tokens)
      row.append(token.trimmed().remove('"'));

    if (!row.isEmpty())
      fullData.append(row);

    if (!row.isEmpty()) {
      QStringList parts = row[0].split("/", Qt::KeepEmptyParts);
      if (!parts.isEmpty() && parts[0] == "America" || row[0] == "Pacific/Honolulu") {
        americaData.append(row);
      }
    }
  }

  file.close();

  if (isShowAllTimeZonesSelected) {
    return fullData;
  } else {
    return americaData;
  }
}

int SurfaceInput::fetchDEMFile(QVector<double> boundingBox, std::string demFile, double resolution, std::string fetchType)
{
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;

  err = NinjaFetchDEMBBox(ninjaArmy, boundingBox.data(), demFile.c_str(), resolution, strdup(fetchType.c_str()), papszOptions);
  if (err != NINJA_SUCCESS){
    qDebug() << "NinjaFetchDEMBBox: err =" << err;
    return err;
  }
  else
  {
    return NINJA_SUCCESS;
  }
}

void SurfaceInput::computeDEMFile(QString filePath)
{
  double adfGeoTransform[6];
  GDALDataset *poInputDS;
  poInputDS = (GDALDataset*)GDALOpen(filePath.toStdString().c_str(), GA_ReadOnly);

  GDALDriverName = poInputDS->GetDriver()->GetDescription();
  GDALXSize = poInputDS->GetRasterXSize();
  GDALYSize = poInputDS->GetRasterYSize();

  if (poInputDS->GetGeoTransform(adfGeoTransform) == CE_None) {
    double c1, c2;
    c1 = adfGeoTransform[1];
    c2 = adfGeoTransform[5];
    if (abs(c1) == abs(c2)) {
      GDALCellSize = abs(c1);
    } else {
      GDALClose((GDALDatasetH)poInputDS);
    }
  }

  GDALRasterBand* band = poInputDS->GetRasterBand(1);
  int gotMin = 0, gotMax = 0;
  double minVal = band->GetMinimum(&gotMin);
  double maxVal = band->GetMaximum(&gotMax);
  if (!gotMin || !gotMax) {
    band->ComputeStatistics(false, &minVal, &maxVal, nullptr, nullptr, nullptr, nullptr);
  }

  GDALMinValue = minVal;
  GDALMaxValue = maxVal;

  GDALClose((GDALDatasetH)poInputDS);

}

double SurfaceInput::computeMeshResolution(int index, bool isMomemtumChecked)
{
  int coarse = 4000;
  int medium = 10000;
  int fine = 20000;
  double meshResolution = 200.0;

  if( GDALCellSize == 0.0 || GDALXSize == 0 || GDALYSize == 0 )
  {
    return meshResolution;
  }

#ifdef NINJAFOAM
  if (isMomemtumChecked) {
    coarse = 25000;
    medium = 50000;
    fine = 100000;
  }
#endif //NINJAFOAM

  int targetNumHorizCells = fine;
  switch (index) {
  case 0:
    targetNumHorizCells = coarse;
    break;
  case 1:
    targetNumHorizCells = medium;
    break;
  case 2:
    targetNumHorizCells = fine;
    break;
  case 3:
    return 200;
  default:
    return 200;
  }

  double XLength = GDALXSize * GDALCellSize;
  double YLength = GDALYSize * GDALCellSize;
  double nXcells = 2 * std::sqrt((double)targetNumHorizCells) * (XLength / (XLength + YLength));
  double nYcells = 2 * std::sqrt((double)targetNumHorizCells) * (YLength / (XLength + YLength));

  double XCellSize = XLength / nXcells;
  double YCellSize = YLength / nYcells;

  meshResolution = (XCellSize + YCellSize) / 2;

#ifdef NINJAFOAM
  if (isMomemtumChecked) {
    XLength = GDALXSize * GDALCellSize;
    YLength = GDALYSize * GDALCellSize;

    double dz = GDALMaxValue - GDALMinValue;
    double ZLength = std::max((0.1 * std::max(XLength, YLength)), (dz + 0.1 * dz));
    double zmin, zmax;
    zmin = GDALMaxValue + 0.05 * ZLength; //zmin (above highest point in DEM for MDM)
    zmax = GDALMaxValue + ZLength; //zmax

    double volume;
    double cellCount;
    double cellVolume;

    volume = XLength * YLength * (zmax-zmin); //volume of blockMesh
    cellCount = targetNumHorizCells * 0.5; // cell count in volume 1
    cellVolume = volume/cellCount; // volume of 1 cell in blockMesh
    double side = std::pow(cellVolume, (1.0/3.0)); // length of side of cell in blockMesh

           //determine number of rounds of refinement
    int nCellsToAdd = 0;
    int refinedCellCount = 0;
    int nCellsInLowestLayer = int(XLength/side) * int(YLength/side);
    int nRoundsRefinement = 0;
    while(refinedCellCount < (0.5 * targetNumHorizCells)){
      nCellsToAdd = nCellsInLowestLayer * 8; //each cell is divided into 8 cells
      refinedCellCount += nCellsToAdd - nCellsInLowestLayer; //subtract the parent cells
      nCellsInLowestLayer = nCellsToAdd/2; //only half of the added cells are in the lowest layer
      nRoundsRefinement += 1;
    }

    meshResolution = side/(nRoundsRefinement*2.0);
  }
#endif //NINJAFOAM

  return meshResolution;

}

void SurfaceInput::computeBoundingBox(double centerLat, double centerLon, double radius, double boundingBox[4])
{
  const double EARTH_RADIUS_MILES = 3958.7613;
  const double DEG_TO_RAD = M_PI / 180.0;

  double deltaLat = radius / (2.0 * M_PI * EARTH_RADIUS_MILES / 360.0);
  double latRadius = EARTH_RADIUS_MILES * std::cos(centerLat * DEG_TO_RAD);
  double deltaLon = radius / (2.0 * M_PI * latRadius / 360.0);

  boundingBox[0] = centerLat + deltaLat;  // North latitude
  boundingBox[1] = centerLon + deltaLon;  // East longitude
  boundingBox[2] = centerLat - deltaLat;  // South latitude
  boundingBox[3] = centerLon - deltaLon;  // West longitude

  qDebug() << "Bounding Box (N, E, S, W):"
           << boundingBox[0]
           << boundingBox[1]
           << boundingBox[2]
           << boundingBox[3];
}

void SurfaceInput::computePointRadius(double north, double east, double south, double west, double pointRadius[3])
{
  const double EARTH_RADIUS_MILES = 3958.7613;
  const double DEG_TO_RAD = M_PI / 180.0;

  double centerLat = (north + south) / 2.0;
  double centerLon = (east + west) / 2.0;

  double deltaLat = std::abs(north - south) / 2.0;
  double deltaLon = std::abs(east - west) / 2.0;

  double latMiles = (2.0 * M_PI * EARTH_RADIUS_MILES / 360.0) * deltaLat;

  double latRadius = EARTH_RADIUS_MILES * std::cos(centerLat * DEG_TO_RAD);
  double lonMiles = (2.0 * M_PI * latRadius / 360.0) * deltaLon;

  double radius = (latMiles + lonMiles) / 2.0;

  pointRadius[0] = centerLat;
  pointRadius[1] = centerLon;
  pointRadius[2] = radius;

  qDebug() << "Center (Lat, Lon):" << centerLat << centerLon;
  qDebug() << "Estimated Radius (mi):" << radius;
}
