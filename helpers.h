#ifndef HELPERS_H
#define HELPERS_H

#include <QSettings>
#include <QList>
#include "tracker.h"
#include "DateTime.h"

namespace Settings {
    QSettings* getSettings();
    void saveTrackers(QList<Tracker> trackers);
    QList<Tracker> loadTrackers();
    bool getUseLocalTime();
    void setUseLocalTime(bool useLocalTime);
}

namespace Helpers {
    QString betterDate(DateTime date);
    QString betterTime(DateTime time);
    double radToDeg(double rad);
    double clip(double val, double max);
}

#endif // HELPERS_H
