#ifndef UTIL_H
#define UTIL_H

#include <QSettings>
#include <QList>
#include "tracker.h"

namespace Util {
    QSettings getSettings();
}

namespace Settings {
    QList<Tracker> loadTrackers();
    void saveTrackers(QList<Tracker> trackers);
    bool getUseLocalTime();
    void setUseLocalTime(bool useLocalTime);
}

QSettings Util::getSettings() {
    return QSettings("EMMN", "INPE");
}

void Settings::saveTrackers(QList<Tracker> trackers) {
    QSettings settings = Util::getSettings();

    settings.beginWriteArray("trackers", trackers.length());
    int i;
    for(i = 0; i < trackers.length(); ++i) {
        //qDebug() << trackers.at(i).getTitle();
        settings.setArrayIndex(i);
        settings.setValue("tracker", QVariant::fromValue(trackers[i]));
    }
    settings.endArray();
}

QList<Tracker> Settings::loadTrackers() {
    settings = Util::getSettings();
    QList<Tracker> trackers;
    int count = settings.beginReadArray("trackers");
    int i;
    for(i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        trackers.push_back(settings.value("tracker").value<Tracker>());
    }

    settings.endArray();
    return trackers;
}

bool Settings::getUseLocalTime() {
    return settings.value("useLocalTime", false).toBool();
}

void Settings::setUseLocalTime(bool useLocalTime) {
    settings.setValue("useLocalTime", useLocalTime);
}

#endif // UTIL_H
