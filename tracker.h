#ifndef TRACKER_H
#define TRACKER_H

#include <Observer.h>
#include <SGP4.h>
#include <Util.h>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <vector>
#include <list>
#include <QString>
#include <QList>
#include <QMetaType>
#include "network.h"

struct PassDetails
{
    DateTime aos;
    DateTime los;
    double max_elevation;
};

// Needed so you can wrap PassDetails in a QVariant
Q_DECLARE_METATYPE(PassDetails)
Q_DECLARE_METATYPE(QList<PassDetails>)

class Tracker
{
public:
    enum Satellite {
        Azimuth,
        Elevation,
        Range,
        Latitude,
        Longitude,
        Altitude
    };

    // it's better for these to use std::string instead of QString because they belong to the SGP4 lib
    Tracker();
    Tracker(std::vector<std::string> tle_list);
    Tracker(QList<QString> tle_list);
    Tracker(std::string tle1, std::string tle2, std::string tle3);
    double FindMaxElevation(
        const DateTime& aos,
        const DateTime& los) const;
    DateTime FindCrossingPoint(
        const DateTime& initial_time1,
        const DateTime& initial_time2,
        bool finding_aos) const;
    std::list<PassDetails> GeneratePassList(
        const DateTime& start_time = DateTime::Now(true),
        const DateTime& end_time = DateTime::Now(true).AddDays(7.0),
        const int time_step = 180);
    QList<PassDetails> GeneratePassListQt(
        const DateTime& start_time = DateTime::Now(true),
        const DateTime& end_time = DateTime::Now(true).AddDays(7.0),
        const int time_step = 180) const;
    QString nextPass() const;
    QString getTitle() const;
    QString getFullTLE() const;
    void UpdateTLE() const;
    QString getSatInfo(int info) const;
    friend QDataStream &operator <<(QDataStream &stream, const Tracker &val);
    friend QDataStream &operator >>(QDataStream &stream, Tracker &val);
private:
    CoordGeodetic user_geo;
    QString tle1;
    QString tle2;
    QString tle3;
};

Q_DECLARE_METATYPE(Tracker)

#endif // TRACKER_H
