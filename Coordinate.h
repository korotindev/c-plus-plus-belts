//
// Created by Дмитрий Коротин on 01.06.2020.
//

#ifndef C_PLUS_PLUS_BELTS_COORDINATE_H
#define C_PLUS_PLUS_BELTS_COORDINATE_H

static const double PI = 3.1415926535;
static const double EARTH_RADIUS = 6371.0; // kilometers

struct Coordinate {
   double Latitude;
   double Longitude;
   double GetDistance(const Coordinate &other) const;
};


#endif //C_PLUS_PLUS_BELTS_COORDINATE_H
