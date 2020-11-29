#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "svg.h"
#include "transport_router.h"
#include "map_rendering_settings.h"
#include "utils.h"

namespace Responses {
  struct Stop {
    std::set<std::string> bus_names;
  };

  struct Bus {
    size_t stops_count = 0;
    size_t unique_stop_count = 0;
    int road_route_length = 0;
    double geo_route_length = 0.0;
  };

  struct RouteInfo {
    TransportRouter::RouteInfo transport_route_info;
    std::string map;
  };
}  // namespace Responses

class TransportCatalog {
 private:
  using Bus = Responses::Bus;
  using Stop = Responses::Stop;
  using RouteInfo = Responses::RouteInfo;

 public:
  TransportCatalog(std::vector<Descriptions::InputQuery> data, const Json::Dict& routing_settings_json,
                   const Json::Dict& render_settings_json);

  const Stop* GetStop(const std::string& name) const;
  const Bus* GetBus(const std::string& name) const;

  std::optional<RouteInfo> FindRoute(const std::string& stop_from, const std::string& stop_to) const;

  std::string RenderMap() const;

 private:
  static int ComputeRoadRouteLength(const std::vector<std::string>& stops, const Descriptions::StopsDict& stops_dict);

  static double ComputeGeoRouteDistance(const std::vector<std::string>& stops,
                                        const Descriptions::StopsDict& stops_dict);

  Descriptions::StopsDict stops_dict_;
  Descriptions::BusesDict buses_dict_;
  std::unordered_map<std::string, Stop> stops_;
  std::unordered_map<std::string, Bus> buses_;
  std::unique_ptr<TransportRouter> router_;
  std::unique_ptr<const MapRenderingSettings> map_rendering_settings_;
};
