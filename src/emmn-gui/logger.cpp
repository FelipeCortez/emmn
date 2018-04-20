#include "logger.h"
#include <QDesktopServices>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QDebug>

// https://stackoverflow.com/a/26991243
bool fileExists(QString path) {
    QFileInfo check_file(path);
    return check_file.exists() && check_file.isFile();
}

Logger::Logger(QObject *parent)
    : QObject(parent)
    , file(nullptr)
    , stream(nullptr)
{
    path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if (path.isEmpty()) {
        qFatal("Cannot determine settings storage location");
    }

    QDir dir(path);

    auto now = QDateTime::currentDateTime();

    if (dir.mkpath(dir.absolutePath()) && QDir::setCurrent(dir.absolutePath())) {
        QString filename("log-" + now.date().toString(Qt::DateFormat::ISODate) + ".csv");

        bool newFile = !fileExists(filename);

        file = new QFile(filename);
        file->open(QIODevice::Append | QIODevice::Text);
        stream = new QTextStream(file);
        stream->setCodec("UTF-8");

        if (newFile) {
            *stream << "sep=," << endl;
            *stream << QString::fromUtf8("hora, azimute_geo, azimute_mec, elevacao") << endl;
        }
    }
}

void Logger::addLog(LogEntry log) {
    qDebug() << log.azGeo;
    // if (stream != nullptr) {
    //     auto now = QDateTime::currentDateTime();
    //     *stream << now.time().toString(Qt::DateFormat::ISODate)
    //             << ", "
    //             << QString::number(pos.azimuth, 'f', 3)
    //             << ", "
    //             << QString::number(pos.elevation, 'f', 3)
    //             << endl;
    // }
}

void Logger::openLogDirectory() {
    QDesktopServices::openUrl(QUrl(QDir(path).absolutePath()));
}
