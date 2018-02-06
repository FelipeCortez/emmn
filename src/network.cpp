#include "network.h"
#include "helpers.h"
#include <QDebug>

Network::Network(QObject *parent)
    : QObject(parent)
    , reply(nullptr)
    , cookiesAcquired(false)
{
}

void Network::getTLE(const QString tle1) {
    QNetworkRequest req;
    req.setUrl(QUrl("http://celestrak.com/NORAD/elements/resource.txt"));
    req.setRawHeader("User-Agent" ,
                     "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.60 Safari/537.17");
    reply = manager.get(req);
    satName = tle1;
    connect(reply, SIGNAL(finished()), this, SLOT(tleDownloadFinished()));
}

QStringList Network::tleDownloadFinished() {
    QStringList tle;
    while (reply->canReadLine()) {
        QString line = QString(reply->readLine()).replace("\r\n", "");

        if (line.trimmed() == satName) {
            tle << QString(reply->readLine()).replace("\r\n", "");
            tle << QString(reply->readLine()).replace("\r\n", "");
        }
    }

    return tle;
}

// -----------------------------------------

void Network::updateSatelliteCatalogue() {
    QNetworkRequest req;
    req.setUrl(QUrl("https://www.space-track.org/ajaxauth/login"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,  "application/x-www-form-urlencoded");

    QUrlQuery postData;
    QStringList credentials = Helpers::getSpaceTrackCredentials();
    postData.addQueryItem("identity", credentials[0]);
    postData.addQueryItem("password", credentials[1]);

    //qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();
    reply = manager.post(req, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(cookiesDownloadFinished()));

    #ifndef QT_NO_SSL
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
    #endif
}

#ifndef QT_NO_SSL
void Network::sslErrors(const QList<QSslError> &errors)
{
    QString errorString;
    foreach (const QSslError &error, errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }

    //reply->ignoreSslErrors();
}
#endif

void Network::cookiesDownloadFinished() {
    QByteArray bytes = reply->readAll(); // bytes
    QString replyString(bytes); // string
    QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);
    if (cookieVar.isValid()) {
        QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie>>();
        for (QNetworkCookie cookie : cookies) {
            jar.insertCookie(cookie);
        }
    }

    reply->deleteLater();

    getTLEs();
}

// -----------------------------------------

void Network::getTLEs() {
    QNetworkRequest req;
    req.setUrl(QUrl("https://www.space-track.org/basicspacedata/query/class/tle_latest/ORDINAL/1/EPOCH/%3Enow-30/MEAN_MOTION/%3E11.25/format/3le"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,  "application/x-www-form-urlencoded");
    reply = manager.get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(tlesDownloadFinished()));
}

void Network::tlesDownloadFinished() {
    QStringList tleList;

    while (reply->canReadLine()) {
        tleList << QString(reply->readLine()).replace("\r\n", "");
        qDebug() << tleList.last();
    }

    Helpers::saveTLEList(tleList);

    reply->deleteLater();
}
