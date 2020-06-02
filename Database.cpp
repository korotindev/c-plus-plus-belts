//
// Created by Дмитрий Коротин on 02.06.2020.
//

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
  uniqueStorage[move(bus.name)].insert(move(bus.name));
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

ReadBusResponse Database::ReadBus(const std::string& busName) {
  const auto& stops = busStorage.GetStops(busName);
  ReadBusResponse response;
  response.stopsCount = stops.size();
  response.uniqueStopsCount = busStorage.GetUniqueStopsCount(busName);
  for(size_t i = 1; i < stops.size(); i++) {
    response.routeDistance += stopsStorage.GetDistance(stops[i-1], stops[i]);
  }
  return response;
}

Bus::Bus(std::string name_, std::vector<std::string> stopsNames_) : name(move(name_)), stopsNames(move(stopsNames_)) {}

Stop::Stop(std::string name, Coordinate coordinate) : name(move(name)), coordinate(coordinate) {}

std::ostream& operator<<(std::ostream& output, const ReadBusResponse& response) {
  output << "Bus " << response.busName << ": "
         << response.stopsCount << " stops on route" << ", "
         << response.uniqueStopsCount << " unique stops" << ", "
         << response.routeDistance << " route length";
  return output;
}