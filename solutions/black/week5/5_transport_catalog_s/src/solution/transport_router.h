#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "company.pb.h"
#include "descriptions.h"
#include "graph.h"
#include "json.h"
#include "router.h"
#include "transport_router.pb.h"

class TransportRouter {
 private:
  using BusGraph = Graph::DirectedWeightedGraph<double>;
  using Router = Graph::Router<double>;

 public:
  TransportRouter(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
                  const Json::Dict& routing_settings_json);

  void Serialize(TCProto::TransportRouter& proto) const;
  static std::unique_ptr<TransportRouter> Deserialize(const TCProto::TransportRouter& proto);

  struct RouteInfo {
    double total_time;

    struct RideBusItem {
      std::string bus_name;
      double time;
      size_t start_stop_idx;
      size_t finish_stop_idx;
      size_t span_count;
    };
    struct WaitBusItem {
      std::string stop_name;
      double time;
    };

    struct WalkToCompanyItem {
      const YellowPages::Company* company;
      std::string stop_name;
      double time;
    };

    using Item = std::variant<RideBusItem, WaitBusItem, WalkToCompanyItem>;
    std::vector<Item> items;
  };

  std::optional<RouteInfo> FindRoute(const std::string& stop_from, const std::string& stop_to) const;
  std::optional<RouteInfo> FindFastestRouteToAnyCompany(
      const std::string& stop_from, const std::vector<const YellowPages::Company*>& companies) const;

 private:
  TransportRouter() = default;

  struct RoutingSettings {
    int bus_wait_time;           // in minutes
    double bus_velocity;         // km/h
    double pedestrian_velocity;  // km/h
  };

  static RoutingSettings MakeRoutingSettings(const Json::Dict& json);

  void FillGraphWithStops(const Descriptions::StopsDict& stops_dict);

  void FillGraphWithBuses(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict);

  struct StopVertexIds {
    Graph::VertexId in;
    Graph::VertexId out;
  };
  struct VertexInfo {
    std::string stop_name;
  };

  struct BusEdgeInfo {
    std::string bus_name;
    size_t start_stop_idx;
    size_t finish_stop_idx;
  };
  struct WaitEdgeInfo {};
  using EdgeInfo = std::variant<BusEdgeInfo, WaitEdgeInfo>;

  RoutingSettings routing_settings_;
  BusGraph graph_;
  // TODO: Tell about this unique_ptr usage case
  std::unique_ptr<Router> router_;
  std::unordered_map<std::string, StopVertexIds> stops_vertex_ids_;
  std::vector<VertexInfo> vertices_info_;
  std::vector<EdgeInfo> edges_info_;
};
