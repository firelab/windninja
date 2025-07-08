#ifndef BRIDGE_H
#define BRIDGE_H

#include <QObject>
#include <QDebug>

class MapBridge : public QObject {
  Q_OBJECT
public:
  explicit MapBridge(QObject *parent = nullptr) : QObject(parent) {}

public slots:
  void receiveMessage(const QString &msg) {
    qDebug() << "Received from JS:" << msg;
  }
};

#endif // BRIDGE_H
