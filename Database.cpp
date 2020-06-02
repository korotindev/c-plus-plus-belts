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
  uniqueStorage[move(bus.name)].insert(make_move_iterator(bus.stopsNames.begin()),
                                       make_move_iterator(bus.stopsNames.end()));
}

size_t BusStorage::GetUniqueStopsCount(const std::string& busName) const {
  if (auto it = uniqueStorage.find(busName); it != uniqueStorage.end()) {
    return it->second.size();
  }
  return 0;
}

const std::vector<std::string>& BusStorage::GetStops(const std::string& busName) const {
  static std::vector<std::string> defaultResult;
  if (auto it = storage.find(busName); it != storage.end()) {
    return it->second;
  }
  return defaultResult;
}

void Database::EntertainStop(Stop stop) {
  stopsStorage.Add(move(stop));
}

void Database::EntertainBus(Bus bus) {
  busStorage.Add(move(bus));
}

ReadBusResponse::ReadBusResponse(string busName_) : busName(busName_) {}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string& busName) {
  const auto& stops = busStorage.GetStops(busName);

  if (stops.empty()) {
    return make_unique<ReadNoBusResponse>(busName);
  }

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

void ReadNoBusResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": not found";
}

void ReadBusMetricsResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": "
         << stopsCount << " stops on route" << ", "
         << uniqueStopsCount << " unique stops" << ", "
         << routeDistance << " route length";
}

