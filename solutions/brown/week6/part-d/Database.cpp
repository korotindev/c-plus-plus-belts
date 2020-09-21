#include "Database.h"

using namespace std;

void StopsStorage::Add(Stop stop) {
  for (auto &[targetStopName, distance] : stop.knownDistances) {
    distanceStorage[make_pair(stop.name, targetStopName)] = distance;
  }
  storage[move(stop.name)].coordinate = stop.coordinate;
}

double StopsStorage::GetDistanceV2(const string &lhsStopName, const string &rhsStopName) const {
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

double StopsStorage::GetDistance(const string &lhsStopName, const string &rhsStopName) const {
  auto lhsCoord = storage.find(lhsStopName)->second.coordinate;
  auto rhsCoord = storage.find(rhsStopName)->second.coordinate;
  auto distance = lhsCoord.GetDistance(rhsCoord);
  return distance;
}

void BusStorage::Add(Bus bus) {
  storage[bus.name] = bus.stopsNames;
  uniqueStorage[move(bus.name)].insert(make_move_iterator(bus.stopsNames.begin()),
                                       make_move_iterator(bus.stopsNames.end()));
}

size_t BusStorage::GetUniqueStopsCount(const std::string &busName) const {
  if (auto it = uniqueStorage.find(busName); it != uniqueStorage.end()) {
    return it->second.size();
  }
  return 0;
}

bool BusStorage::Exist(const std::string &busName) const { return storage.find(busName) != storage.end(); }

const vector<string> &BusStorage::GetStops(const std::string &busName) const {
  static const vector<string> defaultStops;
  if (auto it = storage.find(busName); it != storage.end()) {
    return it->second;
  }
  return defaultStops;
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

void Database::EntertainStop(Stop stop) { stopsStorage.Add(move(stop)); }

void Database::EntertainBus(Bus bus) {
  for (auto &stopName : bus.stopsNames) {
    stopsStorage.AddBusToStop(stopName, bus.name);
  }
  busStorage.Add(move(bus));
}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string &busName, const size_t requestId) {
  if (!busStorage.Exist(busName)) {
    return make_unique<ReadNoBusResponse>(busName, requestId);
  }

  const auto &stops = busStorage.GetStops(busName);
  auto response = make_unique<ReadBusMetricsResponse>(busName, requestId);

  response->stopsCount = stops.size();
  response->uniqueStopsCount = busStorage.GetUniqueStopsCount(busName);
  response->routeDistance = 0.0;
  response->routeDistanceV2 = 0.0;
  for (size_t i = 1; i < stops.size(); i++) {
    response->routeDistance += stopsStorage.GetDistance(stops[i - 1], stops[i]);
    response->routeDistanceV2 += stopsStorage.GetDistanceV2(stops[i - 1], stops[i]);
  }
  return response;
}

Bus::Bus(std::string name_, std::vector<std::string> stopsNames_) : name(move(name_)), stopsNames(move(stopsNames_)) {}

Stop::Stop(std::string name, Coordinate coordinate, std::vector<StopDistance> knownDistances)
    : name(move(name)), coordinate(coordinate), knownDistances(move(knownDistances)) {}

std::unique_ptr<ReadStopResponse> Database::ReadStop(const string &stopName, const size_t requestId) {
  if (!stopsStorage.Exist(stopName)) {
    return make_unique<ReadNoStopResponse>(stopName, requestId);
  }

  auto response = make_unique<ReadStopMetricsResponse>(stopName, requestId);
  const auto &sortedBuses = stopsStorage.GetBuses(stopName);
  response->buses = vector<string>(sortedBuses.begin(), sortedBuses.end());
  return response;
}

bool operator==(const StopDistance &lhs, const StopDistance &rhs) {
  return make_pair(lhs.name, lhs.distance) == make_pair(rhs.name, rhs.distance);
}

std::ostream &operator<<(std::ostream &output, const StopDistance &data) {
  return output << "StopDistance{" << data.name << ", " << data.distance << "}";
}