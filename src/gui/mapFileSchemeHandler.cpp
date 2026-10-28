#include "mapFileSchemeHandler.h"

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QUuid>
#include <QWebEngineUrlRequestJob>

MapFileSchemeHandler::MapFileSchemeHandler(QObject *parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}

QUrl MapFileSchemeHandler::registerFile(const QString &filePath)
{
    QFileInfo info(filePath);

    if (!info.exists() || !info.isFile())
        return QUrl();

    QString extension = info.suffix().toLower();

    if (extension != "kml" && extension != "fgbz")
        return QUrl();

    QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);

    files.insert(token, info.absoluteFilePath());

    return QUrl(QStringLiteral("windninja-data://files/") + token);
}

void MapFileSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl url = request->requestUrl();

    if (url.host() != QStringLiteral("files"))
    {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    QString token = url.path().mid(1);

    if (token.isEmpty() || token.contains('/'))
    {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    QHash<QString, QString>::const_iterator it = files.constFind(token);

    if (it == files.constEnd())
    {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    QFile *file = new QFile(it.value());

    if (!file->open(QIODevice::ReadOnly))
    {
        delete file;
        request->fail(QWebEngineUrlRequestJob::RequestFailed);
        return;
    }

    QMimeDatabase mimeDatabase;
    QString mimeType = mimeDatabase.mimeTypeForFile(
                                       it.value(),
                                       QMimeDatabase::MatchExtension
                                       ).name();

    request->reply(mimeType.toUtf8(), file);
}
