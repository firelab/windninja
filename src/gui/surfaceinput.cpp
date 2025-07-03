#include "surfaceinput.h"
#include <QDebug>

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
  QString stdOffsetStr = matchedRow.value(5);  // Already in HH:MM:SS
  QString dstAdjustStr = matchedRow.value(6);  // Already in HH:MM:SS

         // Function to convert signed HH:MM:SS to total seconds
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

int SurfaceInput::fetchDEMFile(double boundingBox[], std::string demFile, double resolution, std::string fetchType)
{
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;

  err = NinjaFetchDEMBBox(ninjaArmy, boundingBox, demFile.c_str(), resolution, strdup(fetchType.c_str()), papszOptions);
  if (err != NINJA_SUCCESS){
    qDebug() << "NinjaFetchDEMBBox: err =" << err;
    return err;
  }
  else
  {
    return NINJA_SUCCESS;
  }
}
