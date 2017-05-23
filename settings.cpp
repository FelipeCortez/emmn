#include "settings.h"

Q_DECLARE_METATYPE(QList<Tracker>)

Settings::Settings()
    : settings("INPE", "EMMN")
{
    persistentString = settings.value("persistentString", "Testando").toString();
}

void Settings::setPersistentString(QString str) {
    persistentString = str;
    settings.setValue("persistentString", persistentString);
}

void Settings::saveTrackers(QList<Tracker> trackers) {
    settings.setValue("trackers", QVariant::fromValue(trackers));
}

QList<Tracker> Settings::getTrackers() {
    return settings.value("trackers").value<QList<Tracker>>();
}

QString Settings::getPersistentString() {
    return persistentString;
}
