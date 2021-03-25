#include "locator.h"

using namespace std;

double DegToRad(double degrees) {
    return degrees/180.0*PI;
}

double FindMinDistanceTwoPoints(const Point& first, const Point& second) {
    double a1 = std::cos(DegToRad(first.latitude)) *
                std::sin(DegToRad(fabs(first.longitude - second.longitude)) / 2.0);
    double a2 = std::cos(DegToRad(second.latitude)) *
                std::sin(DegToRad(fabs(first.longitude - second.longitude)) / 2.0);

    double b = std::sin(DegToRad(fabs(first.latitude - second.latitude)) / 2.0);

    return EARTH_RADIUS * 2.0 * std::asin(std::sqrt(a1 * a2 + b * b));
}

//double FindMinDistanceTwoPoints(const Point& lhs, const Point& rhs) {
//    return std::acos(std::sin(DegToRad(lhs.latitude)) * std::sin(DegToRad(rhs.latitude)) +
//                std::cos(DegToRad(lhs.latitude)) * std::cos(DegToRad(rhs.latitude)) *
//                std::cos(fabs(DegToRad(lhs.longitude - rhs.longitude)))) * 6'371'000.0;
//}




double Distance::operator +=(Point other) {
    distance += FindMinDistanceTwoPoints(last_point, other);
    last_point = other;
    return distance;
}

double Distance::operator +(Point other) {
    return *this+=other;
}