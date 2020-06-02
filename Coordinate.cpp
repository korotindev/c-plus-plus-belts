#include "Coordinate.h"
#include <cmath>

using namespace std;

double ConvertToRadian(double gradus) {
  return gradus * PI / 180;
}

double Coordinate::GetDistance(const Coordinate& other) const {
  double latitudeRadian = ConvertToRadian(Latitude);
  double longitudeRadian = ConvertToRadian(Longitude);
  double otherLatitudeRadian = ConvertToRadian(other.Latitude);
  double otherLongitudeRadian = ConvertToRadian(other.Longitude);
  double diffLatitude = (latitudeRadian - otherLatitudeRadian) / 2;
  double diffLongitude = (longitudeRadian - otherLongitudeRadian) / 2;
  double sinusLatitudeMultiplication = sin(diffLatitude) * sin(diffLatitude);
  double sinusLongitudeMultiplication = sin(diffLongitude) * sin(diffLongitude);
  double distance = 2.0 * EARTH_RADIUS * asin(sqrt(sinusLatitudeMultiplication +
                                    cos(latitudeRadian) * cos(otherLatitudeRadian) * sinusLongitudeMultiplication));
  return distance * 1000; // to kilometers;
}
