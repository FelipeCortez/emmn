#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QtNetwork>

//! Conexão com a Internet para atualizar diariamente dados de TLEs
class Network : public QObject
{
    Q_OBJECT

public:
    Network(QObject *parent = 0);
    void updateSatelliteCatalogue();
    void getTLEs();

private:
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QNetworkCookieJar jar;
    QString satName;

public slots:
    void tlesDownloadFinished();
    void cookiesDownloadFinished();
#ifndef QT_NO_SSL
    void sslErrors(const QList<QSslError> &errors);
#endif

signals:
    void updateTrackersUI();
};

#endif // NETWORK_H
