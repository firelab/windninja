
#ifndef TILESERVER_H
#define TILESERVER_H

#include <sqlite3.h>

#include <QObject>
#include <QString>
#include <QFile>
#include <QMap>
#include <QHttpServer>
#include <QHttpHeaders>
#include <QHttpServerResponse>
#include <QHttpServerResponder>
#include <QTcpServer>
#include <QHostAddress>

class TileServer : public QObject
{
    Q_OBJECT

public:
    TileServer(QObject* parent = nullptr);

    bool start(quint16 port = 0); // 0 = auto-pick port
    quint16 port() const;  // no longer needed, baseUrl() returns with this already in it, shared across each dataset

    QString baseUrl() const;  // comes with the port() already put into it

    void registerDataset(QString datasetId, QString dataPath);
    void clearDatasets();

private:

    void setupRoutes();

    QHttpServer m_server;
    quint16 m_port;
    QMap<QString, QString> m_datasetMap;
};

#endif // TILESERVER_H
