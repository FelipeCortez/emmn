#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include "tracker.h"

class Settings
{
public:
    Settings();
    QList<Tracker> loadTrackers();
    void saveTrackers(QList<Tracker> trackers);
    bool getUseLocalTime();
    void setUseLocalTime(bool useLocalTime);

private:
    QSettings settings;
    bool useLocalTime;
};

#endif // SETTINGS_H
