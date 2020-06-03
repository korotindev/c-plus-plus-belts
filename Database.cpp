#include "Database.h"

using namespace std;

void StopsStorage::Add(Stop stop) {
  storage[move(stop.name)] = stop.coordinate;
}

double StopsStorage::GetDistance(const string& lhsStopName, const string& rhsStopName) const {
  pair<string, string> routePair = make_pair(max(lhsStopName, rhsStopName), min(lhsStopName, rhsStopName));
  if (auto it = distanceStorage.find(routePair); it != distanceStorage.end()) {
    return it->second;
  }
  auto lhsCoord = storage.find(lhsStopName)->second;
  auto rhsCoord = storage.find(rhsStopName)->second;
  auto distance = lhsCoord.GetDistance(rhsCoord);
  distanceStorage[routePair] = distance;
  return distance;
}

void BusStorage::Add(Bus bus) {
  storage[bus.name] = bus.stopsNames;
  for(auto &stopName : bus.stopsNames) {
    stopsToBuses[stopName].insert(bus.name);
  }
  uniqueStorage[move(bus.name)].insert(make_move_iterator(bus.stopsNames.begin()),
                                       make_move_iterator(bus.stopsNames.end()));
}

size_t BusStorage::GetUniqueStopsCount(const std::string& busName) const {
  if (auto it = uniqueStorage.find(busName); it != uniqueStorage.end()) {
    return it->second.size();
  }
  return 0;
}

bool BusStorage::ExistBus(const std::string& busName) const {
  return storage.find(busName) != storage.end();
}

const vector<string>& BusStorage::GetStops(const std::string& busName) const {
  static const vector<string> defaultStops;
  if (auto it = storage.find(busName); it != storage.end()) {
    return it->second;
  }
  return defaultStops;
}

const set<string>& BusStorage::GetBuses(const string& stopName) const {
  static const set<string> defaultBuses;
  if (auto it = stopsToBuses.find(stopName); it != stopsToBuses.end()) {
    return it->second;
  }
  return defaultBuses;
}

bool BusStorage::ExistStop(const std::string& stopName) const {
  return stopsToBuses.find(stopName) != stopsToBuses.end();
}

void Database::EntertainStop(Stop stop) {
  stopsStorage.Add(move(stop));
}

void Database::EntertainBus(Bus bus) {
  busStorage.Add(move(bus));
}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string& busName) {
  if (!busStorage.ExistBus(busName)) {
    return make_unique<ReadNoBusResponse>(busName);
  }

  const auto& stops = busStorage.GetStops(busName);
  auto response = make_unique<ReadBusMetricsResponse>(busName);

  response->stopsCount = stops.size();
  response->uniqueStopsCount = busStorage.GetUniqueStopsCount(busName);
  response->routeDistance = 0.0;
  for (size_t i = 1; i < stops.size(); i++) {
    response->routeDistance += stopsStorage.GetDistance(stops[i - 1], stops[i]);
  }
  return response;
}

Bus::Bus(std::string name_, std::vector<std::string> stopsNames_) : name(move(name_)), stopsNames(move(stopsNames_)) {}

Stop::Stop(std::string name, Coordinate coordinate) : name(move(name)), coordinate(coordinate) {}


std::unique_ptr<ReadStopResponse> Database::ReadStop(const string& stopName) {
  if (!busStorage.ExistStop(stopName)) {
    return make_unique<ReadNoStopResponse>(stopName);
  }

  auto response = make_unique<ReadStopMetricsResponse>(stopName);
  const auto& sortedBuses = busStorage.GetBuses(stopName);
  response->buses = vector<string>(sortedBuses.begin(), sortedBuses.end());
  return response;
}
