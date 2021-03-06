#include "transport_info.h"

using namespace std;

shared_ptr<const TransportInfo::Stop> TransportInfo::GetStop(const string& name) const {
  return GetSharedValue(indexed_stops_, name);
}

shared_ptr<const TransportInfo::Bus> TransportInfo::GetBus(const string& name) const {
  return GetSharedValue(indexed_buses_, name);
}

shared_ptr<TransportInfo::Stop> TransportInfo::GetStop(const string& name) {
  return GetSharedValue(indexed_stops_, name);
}

shared_ptr<TransportInfo::Bus> TransportInfo::GetBus(const string& name) {
  return GetSharedValue(indexed_buses_, name);
}

void TransportInfo::AddStop(Descriptions::Stop&& stop_desc) {
  auto stop = make_shared<Stop>();
  stop->id = stops_.size();
  stop->name = move(stop_desc.name);
  stop->position = move(stop_desc.position);
  stop->distances = move(stop_desc.distances);

  stops_.push_back(stop);
  indexed_stops_[stop->name] = stop;
}

void TransportInfo::AddBus(Descriptions::Bus&& bus_desc) {
  auto bus = make_shared<Bus>();
  bus->id = buses_.size();
  bus->name = move(bus_desc.name);
  bus->stops = move(bus_desc.stops);
  bus->endpoints = move(bus_desc.endpoints);
  bus->unique_stop_count = ComputeUniqueItemsCount(AsRange(bus->stops));
  bus->road_route_length = ComputeRoadRouteLength(bus->stops);
  bus->geo_route_length = ComputeGeoRouteDistance(bus->stops);

  for (const string& stop_name : bus->stops) {
    GetStop(stop_name)->bus_names.insert(bus->name);
  }

  buses_.push_back(bus);
  indexed_buses_[bus->name] = bus;
}

int TransportInfo::ComputeStopsDistance(std::shared_ptr<const Stop> lhs, std::shared_ptr<const Stop> rhs) const {
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

Range<TransportInfo::StopsVector::const_iterator> TransportInfo::GetStopsRange() const { return AsRange(this->stops_); }
Range<TransportInfo::BusesVector::const_iterator> TransportInfo::GetBusesRange() const { return AsRange(this->buses_); }

size_t TransportInfo::StopsCount() const { return this->stops_.size(); }
size_t TransportInfo::BusesCount() const { return this->buses_.size(); }