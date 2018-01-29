#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QtNetwork>

class Network : public QObject
{
    Q_OBJECT
public:
    Network(QObject *parent = 0);
    void getTLE(const QString tle1);
    void getTLEs();
    void updateSatelliteCatalogue();

private:
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QNetworkCookieJar jar;
    QString satName;
    bool cookiesAcquired;


public slots:
    QStringList tleDownloadFinished();
    void tlesDownloadFinished();
    void cookiesDownloadFinished();
#ifndef QT_NO_SSL
    void sslErrors(const QList<QSslError> &errors);
#endif
};

#endif // NETWORK_H
