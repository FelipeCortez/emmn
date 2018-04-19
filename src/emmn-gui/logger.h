#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QVector>
#include <QTextStream>
#include <QFile>
#include "helpers.h"

struct LogEntry {
    AzEle mecPos; //! posição mecânica
    float azGeo; //! azimute geográfico
    float azSpeed;
    float eleSpeed;
    // controlMode
    // currentPass
    // power
};

//! Salva periodicamente arquivos com informações de azimute, elevação e horário
class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(QObject *parent = 0);
    void addLog(LogEntry log);
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
