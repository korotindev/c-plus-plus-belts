#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "graph.h"
#include "json.h"
#include "router.h"
#include "transport_catalog.pb.h"

class TransportRouter {
 private:
  using BusGraph = Graph::DirectedWeightedGraph<double>;
  using Router = Graph::Router<double>;

 public:
  TransportRouter(Messages::TransportRouter message);
  TransportRouter(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
                  const Json::Dict& routing_settings_json);

  struct RouteInfo {
    double total_time;

    struct BusItem {
      std::string bus_name;
      double time;
      size_t start_stop_idx;
      size_t finish_stop_idx;
      size_t span_count;
    };
    struct WaitItem {
      std::string stop_name;
      double time;
    };

    using Item = std::variant<BusItem, WaitItem>;
    std::vector<Item> items;
  };

  std::optional<RouteInfo> FindRoute(const std::string& stop_from, const std::string& stop_to) const;
  Messages::TransportRouter Serialize() const;

 private:
  struct RoutingSettings {
    int bus_wait_time;    // in minutes
    double bus_velocity;  // km/h
    Messages::TransportRouter::RoutingSettings Serialize() const;
  };

  static RoutingSettings MakeRoutingSettings(const Json::Dict& json);
  static RoutingSettings MakeRoutingSettings(Messages::TransportRouter::RoutingSettings message);

  void FillGraphWithStops(const Descriptions::StopsDict& stops_dict);

  void FillGraphWithBuses(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict);

  struct StopVertexIds {
    Graph::VertexId in;
    Graph::VertexId out;
    Messages::TransportRouter::StopVertexIds Serialize() const;
  };

  static std::pair<std::string, StopVertexIds> MakeStopVertexIdsPair(Messages::TransportRouter::StopVertexIds message);

  struct VertexInfo {
    std::string stop_name;
    Messages::TransportRouter::VertexInfo Serialize() const;
  };

  static VertexInfo MakeVertexInfo(Messages::TransportRouter::VertexInfo message);

  struct BusEdgeInfo {
    std::string bus_name;
    size_t start_stop_idx;
    size_t finish_stop_idx;
    size_t start_vertex_id;
    size_t finish_vertex_id;
    double ride_time;
    Messages::TransportRouter::BusEdgeInfo Serialize() const;
  };

  static BusEdgeInfo MakeBusEdgeInfo(Messages::TransportRouter::BusEdgeInfo message);

  struct WaitEdgeInfo {};
  using EdgeInfo = std::variant<BusEdgeInfo, WaitEdgeInfo>;

  Messages::TransportRouter::GraphRouterInternalData SerializeGraphRouter() const;
  static std::unique_ptr<Router> MakeGraphRouter(const BusGraph& grapth,
                                                 Messages::TransportRouter::GraphRouterInternalData message);

  RoutingSettings routing_settings_;
  BusGraph graph_;
  // TODO: Tell about this unique_ptr usage case
  std::unique_ptr<Router> router_;
  std::unordered_map<std::string, StopVertexIds> stops_vertex_ids_;
  std::vector<VertexInfo> vertices_info_;
  std::vector<EdgeInfo> edges_info_;
};