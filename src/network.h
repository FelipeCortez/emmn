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

private:
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QString satName;
    //QString

public slots:
    QStringList downloadFinished();
};

#endif // NETWORK_H
