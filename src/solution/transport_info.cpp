#include "transport_info.h"

using namespace std;

TransportInfo::ConstStopPtr TransportInfo::GetStop(const string& name) const {
  return GetSharedValue(name_indexed_stops_, name);
}

TransportInfo::ConstBusPtr TransportInfo::GetBus(const string& name) const {
  return GetSharedValue(name_indexed_buses_, name);
}

TransportInfo::ConstStopPtr TransportInfo::GetStop(const size_t id) const {
  return GetSharedValue(id_indexed_stops_, id);
}

TransportInfo::ConstBusPtr TransportInfo::GetBus(const size_t id) const {
  return GetSharedValue(id_indexed_buses_, id);
}

void TransportInfo::AddStop(Descriptions::Stop&& stop_desc) {
  auto stop = make_shared<Stop>();
  stop->id = stops_.size();
  stop->name = move(stop_desc.name);
  stop->position = move(stop_desc.position);
  stop->distances = move(stop_desc.distances);

  stops_.push_back(stop);
  name_indexed_stops_[stop->name] = stop;
  id_indexed_stops_[stop->id] = stop;
}

void TransportInfo::AddBus(Descriptions::Bus&& bus_desc) {
  auto bus = make_shared<Bus>();
  bus->id = buses_.size();
  bus->name = move(bus_desc.name);
  bus->is_roundtrip = bus_desc.is_roundtrip;
  bus->stops = move(bus_desc.stops);
  bus->endpoints = move(bus_desc.endpoints);
  bus->unique_stop_count = ComputeUniqueItemsCount(AsRange(bus->stops));
  bus->road_route_length = ComputeRoadRouteLength(bus->stops);
  bus->geo_route_length = ComputeGeoRouteDistance(bus->stops);

  for (const string& stop_name : bus->stops) {
    auto stop_ptr = GetSharedValue(name_indexed_stops_, stop_name);
    stop_ptr->bus_names.insert(bus->name);
  }

  buses_.push_back(bus);
  name_indexed_buses_[bus->name] = bus;
  id_indexed_buses_[bus->id] = bus;
}

int TransportInfo::ComputeStopsDistance(TransportInfo::ConstStopPtr lhs, TransportInfo::ConstStopPtr rhs) const {
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

Range<ConstSharedPtrsVectorIterator<TransportInfo::Stop>> TransportInfo::GetStopsRange() const {
  return AsRange(this->stops_);
}
Range<ConstSharedPtrsVectorIterator<TransportInfo::Bus>> TransportInfo::GetBusesRange() const {
  return AsRange(this->buses_);
}

size_t TransportInfo::StopsCount() const { return this->stops_.size(); }
size_t TransportInfo::BusesCount() const { return this->buses_.size(); }