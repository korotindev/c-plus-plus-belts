#include "transport_catalog.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "map_renderer.h"
#include "utils.h"

using namespace std;

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery> data, const Json::Dict& routing_settings_json,
                                   const Json::Dict& render_settings_json) {
  auto stops_end =
      partition(begin(data), end(data), [](const auto& item) { return holds_alternative<Descriptions::Stop>(item); });

  Descriptions::StopsDict stops_dict;
  for (const auto& item : Range{begin(data), stops_end}) {
    const auto& stop = get<Descriptions::Stop>(item);
    stops_dict[stop.name] = &stop;
    stops_.insert({stop.name, {}});
  }

  Descriptions::BusesDict buses_dict;
  for (const auto& item : Range{stops_end, end(data)}) {
    const auto& bus = get<Descriptions::Bus>(item);

    buses_dict[bus.name] = &bus;
    buses_[bus.name] =
        Bus{bus.stops.size(), ComputeUniqueItemsCount(AsRange(bus.stops)),
            ComputeRoadRouteLength(bus.stops, stops_dict), ComputeGeoRouteDistance(bus.stops, stops_dict)};

    for (const string& stop_name : bus.stops) {
      stops_.at(stop_name).bus_names.insert(bus.name);
    }
  }

  router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);

  map_ = BuildMap(stops_dict, buses_dict, render_settings_json);
}

const TransportCatalog::Stop* TransportCatalog::GetStop(const string& name) const {
  return GetValuePointer(stops_, name);
}

const TransportCatalog::Bus* TransportCatalog::GetBus(const string& name) const {
  return GetValuePointer(buses_, name);
}

optional<TransportRouter::RouteInfo> TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const {
  return router_->FindRoute(stop_from, stop_to);
}

string TransportCatalog::RenderMap() const {
  ostringstream out;
  out.precision(6);
  map_.Render(out);
  return out.str();
}

int TransportCatalog::ComputeRoadRouteLength(const vector<string>& stops, const Descriptions::StopsDict& stops_dict) {
  int result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Descriptions::ComputeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
  }
  return result;
}

double TransportCatalog::ComputeGeoRouteDistance(const vector<string>& stops,
                                                 const Descriptions::StopsDict& stops_dict) {
  double result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Sphere::Distance(stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position);
  }
  return result;
}

Svg::Document TransportCatalog::BuildMap(const Descriptions::StopsDict& stops_dict,
                                         const Descriptions::BusesDict& buses_dict,
                                         const Json::Dict& render_settings_json) {
  if (stops_dict.empty()) {
    return {};
  }

  auto stops_collider = [this, &buses_dict](const Descriptions::Stop* stop_ptr,
                                            const vector<const Descriptions::Stop*>& group) {
    const auto& bus_names = this->GetStop(stop_ptr->name)->bus_names;

    for (const auto other_stop_ptr : group) {
      bool has_short_path_forward = stop_ptr->distances.count(other_stop_ptr->name) > 0;
      bool has_short_path_backward = other_stop_ptr->distances.count(stop_ptr->name) > 0;

      if (has_short_path_forward || has_short_path_backward) {
        const auto& other_stop_buses = this->GetStop(other_stop_ptr->name)->bus_names;
        for (const auto& bus_name : bus_names) {
          if (auto it = other_stop_buses.find(bus_name); it != other_stop_buses.cend()) {
            const auto& stops = buses_dict.at(bus_name)->stops;
            for (size_t i = 1; i < stops.size(); ++i) {
              if ((stops[i] == stop_ptr->name && stops[i - 1] == other_stop_ptr->name) ||
                  (stops[i] == other_stop_ptr->name && stops[i - 1] == stop_ptr->name)) {
                return true;
              }
            }
          }
        }
      }
    }
    return false;
  };

  return MapRenderer(stops_dict, buses_dict, render_settings_json, stops_collider).Render();
}
