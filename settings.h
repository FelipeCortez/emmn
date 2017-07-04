#ifndef Settings_H
#define Settings_H

#include <QSettings>
#include <QList>
#include "tracker.h"

namespace Settings {
    inline QSettings* getSettings() {
        return new QSettings("INPE", "EMMN");
    }

    inline void saveTrackers(QList<Tracker> trackers) {
        QSettings* settings = Settings::getSettings();
        settings->beginWriteArray("trackers", trackers.length());
        int i;
        for(i = 0; i < trackers.length(); ++i) {
            //qDebug() << trackers.at(i).getTitle();
            settings->setArrayIndex(i);
            settings->setValue("tracker", QVariant::fromValue(trackers[i]));
        }
        settings->endArray();
        delete settings;
    }

    inline QList<Tracker> loadTrackers() {
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

    inline bool getUseLocalTime() {
        QSettings* settings = Settings::getSettings();
        bool useLocalTime = settings->value("useLocalTime", false).toBool();
        delete settings;
        return useLocalTime;
    }

    inline void setUseLocalTime(bool useLocalTime) {
        QSettings* settings = Settings::getSettings();
        settings->setValue("useLocalTime", useLocalTime);
        delete settings;
    }
}

#endif // Settings_H
