#include "helpers.h"

namespace Settings {
    QSettings* getSettings() {
        return new QSettings("INPE", "EMMN");
    }

    void saveTrackers(QList<Tracker> trackers) {
        QSettings* settings = Settings::getSettings();
        settings->beginWriteArray("trackers", trackers.length());
        int i;
        for(i = 0; i < trackers.length(); ++i) {
            settings->setArrayIndex(i);
            settings->setValue("tracker", QVariant::fromValue(trackers[i]));
        }
        settings->endArray();
        delete settings;
    }

    QList<Tracker> loadTrackers() {
        QSettings* settings = Settings::getSettings();
        QList<Tracker> trackers;
        int count = settings->beginReadArray("trackers");
        int i;
        for(i = 0; i < count; ++i) {
            settings->setArrayIndex(i);
            trackers.push_back(settings->value("tracker").value<Tracker>());
        }

        settings->endArray();
        delete settings;
        return trackers;
    }

    bool getUseLocalTime() {
        QSettings* settings = Settings::getSettings();
        bool useLocalTime = settings->value("useLocalTime", false).toBool();
        delete settings;
        return useLocalTime;
    }

    void setUseLocalTime(bool useLocalTime) {
        QSettings* settings = Settings::getSettings();
        settings->setValue("useLocalTime", useLocalTime);
        delete settings;
    }

    QString getSerialPort() {
        QSettings* settings = Settings::getSettings();
        QString serialPort = settings->value("serialPort").toString();
        delete settings;
        return serialPort;
    }

    void setSerialPort(QString serialPort) {
        QSettings* settings = Settings::getSettings();
        settings->setValue("serialPort", serialPort);
        delete settings;
    }
}

namespace Helpers {

    QString betterDate(DateTime date) {
        QString zone;

        if(Settings::getUseLocalTime()) {
            date = date.AddHours(-3);
            zone = "(GMT-3)";
        } else {
            zone = "UTC";
        }

        return QString("%1/%2/%3 %4:%5:%6 %7").arg(date.Year())
                                              .arg(date.Month(), 2, 10, QChar('0'))
                                              .arg(date.Day(), 2, 10, QChar('0'))
                                              .arg(date.Hour(), 2, 10, QChar('0'))
                                              .arg(date.Minute(), 2, 10, QChar('0'))
                                              .arg(date.Second(), 2, 10, QChar('0'))
                                              .arg(zone);
    }

    QString betterTime(DateTime time) {
        QString zone;

        if(Settings::getUseLocalTime()) {
            time = time.AddHours(-3);
            zone = "(GMT-3)";
        } else {
            zone = "UTC";
        }

        return QString("%4:%5").arg(time.Hour(), 2, 10, QChar('0'))
                               .arg(time.Minute(), 2, 10, QChar('0'));
                                  //.arg(zone);
    }

    double radToDeg(double rad) {
        const double PI = 3.141592653589793238463;
        return rad * 180 / PI;
    }

    double clip(double val, double max) {
        if(fabs(val) > max) {
            return max * (fabs(val) / val);
        } else {
            return val;
        }
    }

    QList<QSerialPortInfo> getSerialPortsAvailable() {
        return QSerialPortInfo::availablePorts();
    }
}
