/*
 * Copyright 2013 Daniel Warner <contact@danrw.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRACKER_H
#define TRACKER_H

#include <Observer.h>
#include <SGP4.h>
#include <Util.h>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <vector>
#include <list>

struct PassDetails
{
    DateTime aos;
    DateTime los;
    double max_elevation;
};

class Tracker
{
public:
    Tracker();
    Tracker(std::vector<std::string> tle_list);
    double FindMaxElevation(
        const DateTime& aos,
        const DateTime& los);
    DateTime FindCrossingPoint(
        const DateTime& initial_time1,
        const DateTime& initial_time2,
        bool finding_aos);
    std::list<struct PassDetails> GeneratePassList(
        const DateTime& start_time = DateTime::Now(true),
        const DateTime& end_time = DateTime::Now(true).AddDays(7.0),
        const int time_step = 180);
private:
    CoordGeodetic user_geo;
    SGP4 sgp4;
};

#endif // TRACKER_H
