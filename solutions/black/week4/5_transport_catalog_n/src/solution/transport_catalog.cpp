#include "transport_catalog.h"
#include "map_renderer.h"
#include "route_renderer.h"
#include "utils.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

using namespace std;

TransportCatalog::TransportCatalog(
    vector<Descriptions::InputQuery> data,
    const Json::Dict& routing_settings_json,
    const Json::Dict& render_settings_json
) {
  auto stops_end = partition(begin(data), end(data), [](const auto& item) {
    return holds_alternative<Descriptions::Stop>(item);
  });

  for (auto& item : Range{begin(data), stops_end}) {
    auto& stop = get<Descriptions::Stop>(item);
    stops_.insert({stop.name, {}});
    string name = stop.name;
    stops_dict_[name] = make_unique<Descriptions::Stop>(move(stop));
  }

  for (auto& item : Range{stops_end, end(data)}) {
    auto& bus = get<Descriptions::Bus>(item);

    buses_[bus.name] = Bus{
      bus.stops.size(),
      ComputeUniqueItemsCount(AsRange(bus.stops)),
      ComputeRoadRouteLength(bus.stops, stops_dict_),
      ComputeGeoRouteDistance(bus.stops, stops_dict_)
    };

    for (const string& stop_name : bus.stops) {
      stops_.at(stop_name).bus_names.insert(bus.name);
    }
    string name = bus.name;
    buses_dict_[name] = make_unique<Descriptions::Bus>(move(bus));
  }

  router_ = make_unique<TransportRouter>(stops_dict_, buses_dict_, routing_settings_json);

  map_rendering_settings_ = make_unique<const MapRenderingSettings>(stops_dict_, buses_dict_, render_settings_json);
}

const TransportCatalog::Stop* TransportCatalog::GetStop(const string& name) const {
  return GetValuePointer(stops_, name);
}

const TransportCatalog::Bus* TransportCatalog::GetBus(const string& name) const {
  return GetValuePointer(buses_, name);
}

optional<TransportCatalog::RouteInfo> TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const {
  if (auto route = router_->FindRoute(stop_from, stop_to)) {
    RouteInfo route_info;
    ostringstream ss;
  // TODO: partially cache it ??
    RouteRenderer(*map_rendering_settings_, buses_dict_, *route, stop_to).Render().Render(ss);
    route_info.transport_route_info = move(*route);
    route_info.map = ss.str();
    return route_info;
  } else {
    return nullopt;
  }
}

string TransportCatalog::RenderMap() const {
  ostringstream out;
  // TODO: cache it ??
  MapRenderer(*map_rendering_settings_, buses_dict_).Render().Render(out);
  return out.str();
}

int TransportCatalog::ComputeRoadRouteLength(
    const vector<string>& stops,
    const Descriptions::StopsDict& stops_dict
) {
  int result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Descriptions::ComputeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
  }
  return result;
}

double TransportCatalog::ComputeGeoRouteDistance(
    const vector<string>& stops,
    const Descriptions::StopsDict& stops_dict
) {
  double result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Sphere::Distance(
      stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position
    );
  }
  return result;
}

