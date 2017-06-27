#include "network.h"
#include <QDebug>

Network::Network(QObject *parent)
    : QObject(parent)
{
}

void Network::getTLE(const QString tle1) {
    QNetworkRequest req;
    req.setUrl(QUrl("http://celestrak.com/NORAD/elements/resource.txt"));
    req.setRawHeader( "User-Agent" , "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.60 Safari/537.17");
    reply = manager.get(req);
    satName = tle1;
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

QStringList Network::downloadFinished() {
    QStringList tle;
    qDebug() << "acabô";
    while(reply->canReadLine()) {
        QString line = QString(reply->readLine()).replace("\r\n", "");
        if(line.trimmed() == satName) {
            tle << QString(reply->readLine()).replace("\r\n", "");
            tle << QString(reply->readLine()).replace("\r\n", "");

        }
    }

    reply->deleteLater();
    reply = 0;
    return tle;
}
