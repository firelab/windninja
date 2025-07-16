#ifndef BRIDGE_H
#define BRIDGE_H

#include <QObject>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

class MapBridge : public QObject {
    Q_OBJECT
public:
    explicit MapBridge(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void boundingBoxReceived(double north, double south, double east, double west);


public slots:
    void receiveBoundingBox(const QString &jsonCoords);

};

#endif // BRIDGE_H
