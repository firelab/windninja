#ifndef MAPFILESCHEMEHANDLER_H
#define MAPFILESCHEMEHANDLER_H

#include <QHash>
#include <QObject>
#include <QUrl>
#include <QWebEngineUrlSchemeHandler>

class QWebEngineUrlRequestJob;

class MapFileSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    explicit MapFileSchemeHandler(QObject *parent = nullptr);

    QUrl registerFile(const QString &filePath);

    void requestStarted(QWebEngineUrlRequestJob *request) override;

private:
    QHash<QString, QString> files;
};

#endif // MAPFILESCHEMEHANDLER_H
