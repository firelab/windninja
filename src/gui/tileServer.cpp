

#include "tileServer.h"

TileServer::TileServer(QObject* parent)
    : QObject(parent)
{
}

bool TileServer::start(quint16 port)
{
    setupRoutes();

    QTcpServer* tcpServer = new QTcpServer(this);

    if(!tcpServer->listen(QHostAddress::LocalHost, port)) {
        return false;
    }

    m_port = tcpServer->serverPort();

    m_server.bind(tcpServer);

    return true;
}

quint16 TileServer::port() const
{
    return m_port;
}

QString TileServer::baseUrl() const
{
    return QString("http://127.0.0.1:%1/tiles").arg(m_port);
}

void TileServer::registerDataset(QString datasetId, QString dataPath)
{
    m_datasetMap[datasetId] = dataPath;
}

void TileServer::clearDatasets()
{
    m_datasetMap.clear();
}

// this is inefficient, even if it were a GDAL form. Opening the database multiple times once per call and closing it is not efficient.
// gdal would be even quirkier trying to do such a thing. both forms with this current method are doing: "open dataset -> read tile -> close"
// so multiple read and writes to the same file
void TileServer::setupRoutes()
{
    // GET route
    m_server.route("/tiles/<arg>/<arg>/<arg>/<arg>.pbf",
        [this](const QString& datasetId,
               const QString& zStr,
               const QString& xStr,
               const QString& yStr) {

            //qDebug() << "GET ROUTE HIT:" << datasetId << zStr << xStr << yStr;

            bool okZ, okX, okY;
            int z = zStr.toInt(&okZ);
            int x = xStr.toInt(&okX);
            int y = yStr.toInt(&okY);

            if(!okZ || !okX || !okY) {
                qDebug() << "INT PARSE FAILED";
                return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
            }

            QHttpHeaders headers;
            headers.append("Access-Control-Allow-Origin", "*");
            headers.append("Access-Control-Allow-Methods", "GET, OPTIONS");
            headers.append("Access-Control-Allow-Headers", "*");
            headers.append("Content-Encoding", "gzip");

            QString basePath = m_datasetMap.value(datasetId);

            if(basePath.isEmpty()) {
                QHttpServerResponse response(QHttpServerResponder::StatusCode::NotFound);
                response.setHeaders(headers);
                return response;
            }

            if(!basePath.endsWith(".mbtiles")) {
                QHttpServerResponse response(QHttpServerResponder::StatusCode::NotFound);
                response.setHeaders(headers);
                return response;
            }

            // SQLite handle or GDAL dataset here

            sqlite3* db = nullptr;
            if(sqlite3_open(basePath.toStdString().c_str(), &db) != SQLITE_OK) {
                QHttpServerResponse response(QHttpServerResponder::StatusCode::InternalServerError);
                response.setHeaders(headers);
                return response;
            }

            int tmsY = (1 << z) - 1 - y;

            sqlite3_stmt* stmt = nullptr;

            const char* sql =
                "SELECT tile_data FROM tiles "
                "WHERE zoom_level=? AND tile_column=? AND tile_row=?";

            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

            sqlite3_bind_int(stmt, 1, z);
            sqlite3_bind_int(stmt, 2, x);
            sqlite3_bind_int(stmt, 3, tmsY);

            QByteArray tileData;

            if(sqlite3_step(stmt) == SQLITE_ROW) {
                const void* blob = sqlite3_column_blob(stmt, 0);
                int size = sqlite3_column_bytes(stmt, 0);
                tileData = QByteArray((const char*)blob, size);
            }

            sqlite3_finalize(stmt);
            sqlite3_close(db);

            if(tileData.isEmpty()) {
                QHttpServerResponse response(QHttpServerResponder::StatusCode::NotFound);
                response.setHeaders(headers);
                return response;
            }

            QHttpServerResponse response("application/x-protobuf", tileData);
            response.setHeaders(headers);
            return response;
        }
    );
}


