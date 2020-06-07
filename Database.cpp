#include "Database.h"

using namespace std;

void Database::EntertainStop(Stop stop) {
  stopsStorage.Add(move(stop));
}

void Database::EntertainBus(Bus bus) {
  for (auto& stopName : bus.stopsNames) {
    stopsStorage.AddBusToStop(stopName, bus.name);
  }
  busStorage.Add(move(bus));
}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string& busName, const size_t requestId) {
  if (!busStorage.Exist(busName)) {
    return make_unique<ReadNoBusResponse>(busName, requestId);
  }

  const auto& stops = busStorage.GetStops(busName);
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

std::unique_ptr<ReadStopResponse> Database::ReadStop(const string& stopName, const size_t requestId) {
  if (!stopsStorage.Exist(stopName)) {
    return make_unique<ReadNoStopResponse>(stopName, requestId);
  }

  auto response = make_unique<ReadStopMetricsResponse>(stopName, requestId);
  const auto& sortedBuses = stopsStorage.GetBuses(stopName);
  response->buses = vector<string>(sortedBuses.begin(), sortedBuses.end());
  return response;
}