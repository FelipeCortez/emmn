#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QVector>
#include "helpers.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);
    void addLog(AzEle pos);
    void openLogDirectory();

private:
    QString path;
    QVector<AzEle> positions;
    QFile* file;
    QTextStream* stream;
signals:

public slots:
};

#endif // LOGGER_H
