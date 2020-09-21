#include "StopsStorage.h"

#include <utility>

using namespace std;

Stop::Stop(std::string name, Coordinate coordinate, std::vector<StopDistance> knownDistances)
    : name(move(name)), coordinate(coordinate), knownDistances(move(knownDistances)) {}

bool operator==(const StopDistance &lhs, const StopDistance &rhs) {
  return make_pair(lhs.name, lhs.distance) == make_pair(rhs.name, rhs.distance);
}

std::ostream &operator<<(std::ostream &output, const StopDistance &data) {
  return output << "StopDistance{" << data.name << ", " << data.distance << "}";
}

void StopsStorage::Add(Stop stop) {
  for (auto &[targetStopName, distance] : stop.knownDistances) {
    distanceStorage.emplace(Road(stop.name, targetStopName), RoadData(distance));
  }

  storage[stop.name].coordinate = stop.coordinate;
}

double StopsStorage::GetDistanceV2(const string &lhsStopName, const string &rhsStopName) const {
  if (auto it = distanceStorage.find(Road(lhsStopName, rhsStopName)); it != distanceStorage.end()) {
    return it->second.distance;
  }

  if (auto it = distanceStorage.find(Road(rhsStopName, lhsStopName)); it != distanceStorage.end()) {
    return it->second.distance;
  }

  return 0.0;
}

double StopsStorage::GetDistance(const string &lhsStopName, const string &rhsStopName) const {
  auto lhsCoord = storage.find(lhsStopName)->second.coordinate;
  auto rhsCoord = storage.find(rhsStopName)->second.coordinate;
  auto distance = lhsCoord.GetDistance(rhsCoord);
  return distance;
}

const set<string> &StopsStorage::GetBuses(const string &stopName) const {
  static const set<string> defaultBuses;
  if (auto it = storage.find(stopName); it != storage.end()) {
    return it->second.buses;
  }
  return defaultBuses;
}

bool StopsStorage::Exist(const std::string &stopName) const { return storage.find(stopName) != storage.end(); }

void StopsStorage::AddBusToStop(const std::string &stopName, const std::string &busName) {
  storage[stopName].buses.insert(busName);
}

bool operator==(const Road &lhs, const Road &rhs) { return make_pair(lhs.from, lhs.to) == make_pair(rhs.from, rhs.to); }

Road::Road(std::string from_, std::string to_) {
  from = std::move(from_);
  to = std::move(to_);
}

RoadData::RoadData(double distance_) { distance = distance_; }
