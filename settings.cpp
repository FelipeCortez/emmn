#include "settings.h"
#include <QDebug>

Settings::Settings()
    : settings("INPE", "EMMN")
{
    persistentString = settings.value("persistentString", "Testando").toString();
    qRegisterMetaTypeStreamOperators<Tracker>("Tracker");
}

void Settings::setPersistentString(QString str) {
    persistentString = str;
    settings.setValue("persistentString", persistentString);
}

void Settings::saveTrackers(QList<Tracker> trackers) {
    //settings.setValue("tracker", QVariant::fromValue(trackers.first()));

    //qDebug() << "Salvando";
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

QString Settings::getPersistentString() {
    return persistentString;
}
