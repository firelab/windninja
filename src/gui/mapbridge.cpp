#include "mapbridge.h"

void MapBridge::receiveBoundingBox(const QString &jsonCoords) {
  QJsonDocument doc = QJsonDocument::fromJson(jsonCoords.toUtf8());
  if (!doc.isObject()) {
    qWarning() << "Invalid bounding box JSON";
    return;
  }

  QJsonObject obj = doc.object();

  double north = obj["north"].toDouble();
  double south = obj["south"].toDouble();
  double east = obj["east"].toDouble();
  double west = obj["west"].toDouble();

  qDebug() << "Bounding box received:";
  qDebug() << "North:" << north << "South:" << south;
  qDebug() << "East:" << east << "West:" << west;

  emit boundingBoxReceived(north, south, east, west);
}
