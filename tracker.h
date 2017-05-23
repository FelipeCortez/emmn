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
    // it's better for these to use std::string instead of QString because they belong to the SGP4 lib
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
    QString getTitle() const;
    /*
    friend QDataStream & operator << (QDataStream &arch, const Tracker & object) {
        arch << object.testString;
        arch << object.testInt;
        return arch;
    }

    friend QDataStream & operator >> (QDataStream &arch, Tracker & object) {
         arch >> object.testString;
         arch >> object.testInt;
         return arch;
    }
    */
private:
    CoordGeodetic user_geo;
    QString tle1;
    QString tle2;
    QString tle3;
};

#endif // TRACKER_H
