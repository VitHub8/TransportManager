#pragma once

#include <cmath>

struct Point {
    double latitude = 0.0;
    double longitude = 0.0;
};

static const double EARTH_RADIUS = 6'371'000.0;

constexpr static const double PI = 3.1415926535;

struct Distance {
    Point last_point;
    double distance = 0;

    double operator +=(Point other);

    double operator +(Point other);
};

double DegToRad(double degrees);
double FindMinDistanceTwoPoints(const Point& lhs, const Point& rhs) ;
