#include "network.h"
#include "helpers.h"
#include <QDebug>

Network::Network(QObject *parent)
    : QObject(parent)
    , reply(nullptr)
{
}

void Network::updateSatelliteCatalogue() {
    qDebug() << "hello from thread" << QThread::currentThread();

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
        if (!errorString.isEmpty()) {
            errorString += '\n';
        }
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

void Network::getTLEs() {
    QNetworkRequest req;
    req.setUrl(QUrl("https://www.space-track.org/basicspacedata/query/class/tle_latest/ORDINAL/1/EPOCH/%3Enow-30/MEAN_MOTION/%3E11.25/format/3le"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    reply = manager.get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(tlesDownloadFinished()));
}

void Network::tlesDownloadFinished() {
    QStringList tleList;

    while (reply->canReadLine()) {
        tleList << QString(reply->readLine()).replace("\r\n", "");
        qDebug() << tleList.last();
    }

    qDebug() << tleList.size();
    if (tleList.size() > 0) {
        Helpers::saveTLEList(tleList);
    }

    emit updateTrackersUI();

    reply->deleteLater();
}
