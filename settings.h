#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include "tracker.h"

class Settings
{
public:
    Settings();
    QString getPersistentString();
    void setPersistentString(QString str);
    QList<Tracker> getTrackers();
    void saveTrackers(QList<Tracker> trackers);
private:
    QSettings settings;
    QString persistentString;
};

#endif // SETTINGS_H
