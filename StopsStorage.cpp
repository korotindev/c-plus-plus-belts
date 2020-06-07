#include "StopsStorage.h"

using namespace std;

Stop::Stop(std::string name, Coordinate coordinate, std::vector<StopDistance> knownDistances)
  : name(move(name)),
    coordinate(coordinate),
    knownDistances(move(knownDistances)) {}


bool operator==(const StopDistance& lhs, const StopDistance& rhs) {
  return make_pair(lhs.name, lhs.distance) == make_pair(rhs.name, rhs.distance);
}

std::ostream& operator<<(std::ostream& output, const StopDistance& data) {
  return output << "StopDistance{" << data.name << ", " << data.distance << "}";
}

void StopsStorage::Add(Stop stop) {
  for(auto &[targetStopName, distance] : stop.knownDistances) {
    distanceStorage[make_pair(stop.name, targetStopName)] = distance;
  }
  storage[move(stop.name)].coordinate = stop.coordinate;
}

double StopsStorage::GetDistanceV2(const string& lhsStopName, const string& rhsStopName) const {
  pair<string, string> routePair = make_pair(lhsStopName, rhsStopName);
  if (auto it = distanceStorage.find(routePair); it != distanceStorage.end()) {
    return it->second;
  }

  pair<string, string> reverseRoutePair = make_pair(rhsStopName, lhsStopName);
  if (auto it = distanceStorage.find(reverseRoutePair); it != distanceStorage.end()) {
    return it->second;
  }

  return 0.0;
}

double StopsStorage::GetDistance(const string& lhsStopName, const string& rhsStopName) const {
  auto lhsCoord = storage.find(lhsStopName)->second.coordinate;
  auto rhsCoord = storage.find(rhsStopName)->second.coordinate;
  auto distance = lhsCoord.GetDistance(rhsCoord);
  return distance;
}


const set<string>& StopsStorage::GetBuses(const string& stopName) const {
  static const set<string> defaultBuses;
  if (auto it = storage.find(stopName); it != storage.end()) {
    return it->second.buses;
  }
  return defaultBuses;
}

bool StopsStorage::Exist(const std::string& stopName) const {
  return storage.find(stopName) != storage.end();
}

void StopsStorage::AddBusToStop(const std::string& stopName, const std::string& busName) {
  storage[stopName].buses.insert(busName);
}