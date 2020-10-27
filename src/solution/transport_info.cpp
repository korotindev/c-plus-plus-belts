#include "transport_info.h"

using namespace std;

const TransportInfo::Stop* TransportInfo::GetStop(const string& name) const { return GetValuePointer(stops_, name); }
const TransportInfo::Bus* TransportInfo::GetBus(const string& name) const { return GetValuePointer(buses_, name); }

TransportInfo::Stop* TransportInfo::GetStop(const string& name) { return GetValuePointer(stops_, name); }
TransportInfo::Bus* TransportInfo::GetBus(const string& name) { return GetValuePointer(buses_, name); }

void TransportInfo::AddStop(const Descriptions::Stop& stop) {
  stops_[stop.name] =
      TransportInfo::Stop{.name = stop.name, .position = stop.position, .distances = stop.distances, .bus_names = {}};
}

void TransportInfo::AddBus(const Descriptions::Bus& bus) {
  buses_[bus.name] = Bus{.name = bus.name,
                         .stops = bus.stops,
                         .endpoints = bus.endpoints,
                         .unique_stop_count = ComputeUniqueItemsCount(AsRange(bus.stops)),
                         .road_route_length = ComputeRoadRouteLength(bus.stops),
                         .geo_route_length = ComputeGeoRouteDistance(bus.stops)};

  for (const string& stop_name : bus.stops) {
    GetStop(stop_name)->bus_names.insert(bus.name);
  }
}

int TransportInfo::ComputeStopsDistance(const Stop* lhs, const Stop* rhs) const {
  if (auto it = lhs->distances.find(rhs->name); it != lhs->distances.end()) {
    return it->second;
  } else {
    return rhs->distances.at(lhs->name);
  }
}

int TransportInfo::ComputeStopsDistance(const string& lhs, const string& rhs) const {
  return ComputeStopsDistance(GetStop(lhs), GetStop(rhs));
}

int TransportInfo::ComputeRoadRouteLength(const vector<string>& stops) {
  int result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += ComputeStopsDistance(GetStop(stops[i - 1]), GetStop(stops[i]));
  }
  return result;
}

double TransportInfo::ComputeGeoRouteDistance(const vector<string>& stops) {
  double result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Sphere::Distance(GetStop(stops[i - 1])->position, GetStop(stops[i])->position);
  }
  return result;
}

Range<TransportInfo::StopsDict::const_iterator> TransportInfo::GetStopsRange() const { return AsRange(this->stops_); }
Range<TransportInfo::BusesDict::const_iterator> TransportInfo::GetBusesRange() const { return AsRange(this->buses_); }

size_t TransportInfo::StopsCount() const { return this->stops_.size(); }
size_t TransportInfo::BusesCount() const { return this->buses_.size(); }