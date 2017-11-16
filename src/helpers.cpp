#include "helpers.h"
#include <QRegularExpression>

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

    bool saveTLEList(QStringList tleList) {
        //https://stackoverflow.com/a/32535544
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if(path.isEmpty()) {
            qFatal("Cannot determine settings storage location");
        }

        QDir d{path};

        if(d.mkpath(d.absolutePath()) && QDir::setCurrent(d.absolutePath())) {
            qDebug() << "settings in" << QDir::currentPath();
            QFile f("tleList.txt");
            if(f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&f);
                stream.setCodec("UTF-8");
                stream << "contents";
                for(auto& str : tleList) {
                    stream << str << endl;
                }
            }
        }

        return true;
    }

    class TLEListModel : QAbstractListModel {
        Q_OBJECT

    public:
        TLEModel(QObject *parent = 0);
        inline int rowCount(const QModelIndex &parent = QModelIndex()) const override {
            return 1;
        }

        inline QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            return QVariant("Hello");
        }

    private:
        QList<QStringList> tleList;
    };

    QStringListModel* readTLEList() {
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if(path.isEmpty()) {
            qFatal("Cannot determine settings storage location");
        }

        QDir d{path};

        QRegularExpression re(" DEB");

        QStringListModel* listModel = new QStringListModel();
        QStringList titles;

        if(QDir::setCurrent(d.absolutePath())) {
            QFile f("tleList.txt");
            if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                while(!f.atEnd()) {
                    QString line1 = f.readLine().trimmed().replace("\n", "");
                    if(line1.at(0) == '0' && !re.match(line1).hasMatch()) {
                        line1 = line1.remove(0, 2).replace("\n", "");
                        QString line2 = f.readLine().trimmed().replace("\n", "");
                        QString line3 = f.readLine().trimmed().replace("\n", "");
                        line1 += " [" + (line2.simplified().split(' ').at(1)) + "]";

                        titles << line1;
                    }
                }
            }
        }

        listModel->setStringList(titles);
        return listModel;
    }

    QStringList findInTLEList(QString catalogNumber) {
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if(path.isEmpty()) {
            qFatal("Cannot determine settings storage location");
        }

        QDir d{path};

        QRegularExpression re(" DEB");

        QStringList tle;

        if(QDir::setCurrent(d.absolutePath())) {
            QFile f("tleList.txt");
            if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                while(!f.atEnd()) {
                    QString line1 = f.readLine().trimmed().replace("\n", "");
                    if(line1.at(0) == '0' && !re.match(line1).hasMatch()) {
                        line1 = line1.remove(0, 2).replace("\n", "");
                        QString line2 = f.readLine().trimmed().replace("\n", "");
                        QString line3 = f.readLine().trimmed().replace("\n", "");
                        QString satCat = line2.simplified().split(' ').at(1);
                        if(line2 == satCat) {
                            tle << line1 << line2 << line3;
                            qDebug() << line2;
                            return tle;
                        }
                    }
                }
            }
        }
    }

    QStringList getSpaceTrackCredentials() {
        QFile file(":/txt/credentials.txt");

        if(!file.open(QFile::ReadOnly)) {
            qFatal("Couldn't find credentials file");
            return QStringList();
        } else {
            QStringList credentials;
            credentials << file.readLine().trimmed();
            credentials << file.readLine().trimmed();
            return credentials;
        }
    }
}
