#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QtNetwork>

class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);
    void getTLE(const QString satellite);

private:
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QString satName;

public slots:
    void downloadFinished();
};

#endif // NETWORK_H
