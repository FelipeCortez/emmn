#include "tracker.h"
#include <QDebug>

Tracker::Tracker()
    : user_geo(-5.7793, -35.201, 0.014)
    , tle1("")
    , tle2("")
    , tle3("")
{
}

Tracker::Tracker(std::vector<std::string> tle_list)
    : user_geo(-5.7793, -35.201, 0.014)
    , tle1(QString::fromStdString(tle_list[0]))
    , tle2(QString::fromStdString(tle_list[1]))
    , tle3(QString::fromStdString(tle_list[2]))
{
}

Tracker::Tracker(QList<QString> tle_list)
    : user_geo(-5.7793, -35.201, 0.014)
    , tle1(tle_list[0])
    , tle2(tle_list[1])
    , tle3(tle_list[2])
{
}

Tracker::Tracker(std::string tle1, std::string tle2, std::string tle3)
    : user_geo(-5.7793, -35.201, 0.014)
    , tle1(QString::fromStdString(tle1))
    , tle2(QString::fromStdString(tle2))
    , tle3(QString::fromStdString(tle3))
{
}

void Tracker::UpdateTLE() const {
//
}

QString Tracker::getSatInfo(int info) const {
    SGP4 sgp4(Tle(tle1.toStdString(), tle2.toStdString(), tle3.toStdString()));
    Eci eci = sgp4.FindPosition(DateTime::Now());
    Observer obs(user_geo);
    switch(info) {
        case Satellite::Elevation:
            return QString::number(obs.GetLookAngle(eci).elevation);
        case Satellite::Azimuth:
            return QString::number(obs.GetLookAngle(eci).azimuth);
        case Satellite::Range:
            return QString::number(obs.GetLookAngle(eci).range);
    }
}

double Tracker::FindMaxElevation(
        const DateTime& aos,
        const DateTime& los) const
{
    Observer obs(user_geo);
    SGP4 sgp4(Tle(tle1.toStdString(), tle2.toStdString(), tle3.toStdString()));

    bool running;

    double time_step = (los - aos).TotalSeconds() / 9.0;
    DateTime current_time(aos); //! current time
    DateTime time1(aos); //! start time of search period
    DateTime time2(los); //! end time of search period
    double max_elevation; //! max elevation

    running = true;

    do {
        running = true;
        max_elevation = -99999999999999.0;
        while (running && current_time < time2)
        {
            /*
             * find position
             */
            Eci eci = sgp4.FindPosition(current_time);
            CoordTopocentric topo = obs.GetLookAngle(eci);

            if (topo.elevation > max_elevation)
            {
                /*
                 * still going up
                 */
                max_elevation = topo.elevation;
                /*
                 * move time along
                 */
                current_time = current_time.AddSeconds(time_step);
                if (current_time > time2)
                {
                    /*
                     * dont go past end time
                     */
                    current_time = time2;
                }
            }
            else
            {
                /*
                 * stop
                 */
                running = false;
            }
        }

        /*
         * make start time to 2 time steps back
         */
        time1 = current_time.AddSeconds(-2.0 * time_step);
        /*
         * make end time to current time
         */
        time2 = current_time;
        /*
         * current time to start time
         */
        current_time = time1;
        /*
         * recalculate time step
         */
        time_step = (time2 - time1).TotalSeconds() / 9.0;
    } while (time_step > 1.0);

    return max_elevation;
}

DateTime Tracker::FindCrossingPoint(
        const DateTime& initial_time1,
        const DateTime& initial_time2,
        bool finding_aos) const
{
    Observer obs(user_geo);
    SGP4 sgp4(Tle(tle1.toStdString(), tle2.toStdString(), tle3.toStdString()));

    bool running;
    int cnt;

    DateTime time1(initial_time1);
    DateTime time2(initial_time2);
    DateTime middle_time;

    running = true;
    cnt = 0;
    while (running && cnt++ < 16)
    {
        middle_time = time1.AddSeconds((time2 - time1).TotalSeconds() / 2.0);
        /*
         * calculate satellite position
         */
        Eci eci = sgp4.FindPosition(middle_time);
        CoordTopocentric topo = obs.GetLookAngle(eci);

        if (topo.elevation > 0.0)
        {
            /*
             * satellite above horizon
             */
            if (finding_aos)
            {
                time2 = middle_time;
            }
            else
            {
                time1 = middle_time;
            }
        }
        else
        {
            if (finding_aos)
            {
                time1 = middle_time;
            }
            else
            {
                time2 = middle_time;
            }
        }

        if ((time2 - time1).TotalSeconds() < 1.0)
        {
            /*
             * two times are within a second, stop
             */
            running = false;
            /*
             * remove microseconds
             */
            int us = middle_time.Microsecond();
            middle_time = middle_time.AddMicroseconds(-us);
            /*
             * step back into the pass by 1 second
             */
            middle_time = middle_time.AddSeconds(finding_aos ? 1 : -1);
        }
    }

    /*
     * go back/forward 1second until below the horizon
     */
    running = true;
    cnt = 0;
    while (running && cnt++ < 6)
    {
        Eci eci = sgp4.FindPosition(middle_time);
        CoordTopocentric topo = obs.GetLookAngle(eci);
        if (topo.elevation > 0)
        {
            middle_time = middle_time.AddSeconds(finding_aos ? -1 : 1);
        }
        else
        {
            running = false;
        }
    }

    return middle_time;
}

std::list<PassDetails> Tracker::GeneratePassList(
        const DateTime& start_time,
        const DateTime& end_time,
        const int time_step)
{
    std::list<PassDetails> pass_list;

    Observer obs(user_geo);
    SGP4 sgp4(Tle(tle1.toStdString(), tle2.toStdString(), tle3.toStdString()));

    DateTime aos_time;
    DateTime los_time;

    bool found_aos = false;

    DateTime previous_time(start_time);
    DateTime current_time(start_time);

    while (current_time < end_time)
    {
        bool end_of_pass = false;

        /*
         * calculate satellite position
         */
        Eci eci = sgp4.FindPosition(current_time);
        CoordTopocentric topo = obs.GetLookAngle(eci);

        if (!found_aos && topo.elevation > 0.0)
        {
            /*
             * aos hasnt occured yet, but the satellite is now above horizon
             * this must have occured within the last time_step
             */
            if (start_time == current_time)
            {
                /*
                 * satellite was already above the horizon at the start,
                 * so use the start time
                 */
                aos_time = start_time;
            }
            else
            {
                /*
                 * find the point at which the satellite crossed the horizon
                 */
                aos_time = FindCrossingPoint(
                        previous_time,
                        current_time,
                        true);
            }
            found_aos = true;
        }
        else if (found_aos && topo.elevation < 0.0)
        {
            found_aos = false;
            /*
             * end of pass, so move along more than time_step
             */
            end_of_pass = true;
            /*
             * already have the aos, but now the satellite is below the horizon,
             * so find the los
             */
            los_time = FindCrossingPoint(
                    previous_time,
                    current_time,
                    false);

            PassDetails pd;
            pd.aos = aos_time;
            pd.los = los_time;
            pd.max_elevation = FindMaxElevation(
                    aos_time,
                    los_time);

            pass_list.push_back(pd);
        }

        /*
         * save current time
         */
        previous_time = current_time;

        if (end_of_pass)
        {
            /*
             * at the end of the pass move the time along by 30mins
             */
            current_time = current_time + TimeSpan(0, 30, 0);
        }
        else
        {
            /*
             * move the time along by the time step value
             */
            current_time = current_time + TimeSpan(0, 0, time_step);
        }

        if (current_time > end_time)
        {
            /*
             * dont go past end time
             */
            current_time = end_time;
        }
    };

    if (found_aos)
    {
        /*
         * satellite still above horizon at end of search period, so use end
         * time as los
         */
        PassDetails pd;
        pd.aos = aos_time;
        pd.los = end_time;
        pd.max_elevation = FindMaxElevation(aos_time, end_time);

        pass_list.push_back(pd);
    }

    return pass_list;
}

QList<PassDetails> Tracker::GeneratePassListQt(
        const DateTime& start_time,
        const DateTime& end_time,
        const int time_step) const
{
    QList<PassDetails> pass_list;

    Observer obs(user_geo);
    SGP4 sgp4(Tle(tle1.toStdString(), tle2.toStdString(), tle3.toStdString()));

    DateTime aos_time;
    DateTime los_time;

    bool found_aos = false;

    DateTime previous_time(start_time);
    DateTime current_time(start_time);

    while (current_time < end_time)
    {
        bool end_of_pass = false;

        /*
         * calculate satellite position
         */
        Eci eci = sgp4.FindPosition(current_time);
        CoordTopocentric topo = obs.GetLookAngle(eci);

        if (!found_aos && topo.elevation > 0.0)
        {
            /*
             * aos hasnt occured yet, but the satellite is now above horizon
             * this must have occured within the last time_step
             */
            if (start_time == current_time)
            {
                /*
                 * satellite was already above the horizon at the start,
                 * so use the start time
                 */
                aos_time = start_time;
            }
            else
            {
                /*
                 * find the point at which the satellite crossed the horizon
                 */
                aos_time = FindCrossingPoint(
                        previous_time,
                        current_time,
                        true);
            }
            found_aos = true;
        }
        else if (found_aos && topo.elevation < 0.0)
        {
            found_aos = false;
            /*
             * end of pass, so move along more than time_step
             */
            end_of_pass = true;
            /*
             * already have the aos, but now the satellite is below the horizon,
             * so find the los
             */
            los_time = FindCrossingPoint(
                    previous_time,
                    current_time,
                    false);

            PassDetails pd;
            pd.aos = aos_time;
            pd.los = los_time;
            pd.max_elevation = FindMaxElevation(
                    aos_time,
                    los_time);

            pass_list.push_back(pd);
        }

        /*
         * save current time
         */
        previous_time = current_time;

        if (end_of_pass)
        {
            /*
             * at the end of the pass move the time along by 30mins
             */
            current_time = current_time + TimeSpan(0, 30, 0);
        }
        else
        {
            /*
             * move the time along by the time step value
             */
            current_time = current_time + TimeSpan(0, 0, time_step);
        }

        if (current_time > end_time)
        {
            /*
             * dont go past end time
             */
            current_time = end_time;
        }
    };

    if (found_aos)
    {
        /*
         * satellite still above horizon at end of search period, so use end
         * time as los
         */
        PassDetails pd;
        pd.aos = aos_time;
        pd.los = end_time;
        pd.max_elevation = FindMaxElevation(aos_time, end_time);

        pass_list.push_back(pd);
    }

    return pass_list;
}

QString Tracker::nextPass() const {
    return QString::fromStdString((GeneratePassListQt()[0].aos - DateTime::Now()).ToString());
}

QString Tracker::getTitle() const {
    return tle1.trimmed();
}

QDataStream &operator <<(QDataStream &stream, const Tracker &val) {
    //CoordGeodetic user_geo;
    stream << val.tle1;
    stream << val.tle2;
    stream << val.tle3;
    return stream;
}

QDataStream &operator >>(QDataStream &stream, Tracker &val) {
    stream >> val.tle1;
    stream >> val.tle2;
    stream >> val.tle3;
    return stream;
}
