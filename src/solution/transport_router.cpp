#include "transport_router.h"

using namespace std;

TransportRouter::TransportRouter(shared_ptr<const TransportInfo> transport_info,
                                 const Json::Dict& routing_settings_json)
    : routing_settings_(MakeRoutingSettings(routing_settings_json)) {
  const size_t vertex_count = transport_info->StopsCount() * 2;
  vertices_info_.resize(vertex_count);
  graph_ = BusGraph(vertex_count);

  FillGraphWithStops(transport_info);
  FillGraphWithBuses(transport_info);

  router_ = std::make_unique<Router>(graph_);
}

TransportRouter::RoutingSettings TransportRouter::MakeRoutingSettings(const Json::Dict& json) {
  return {
      json.at("bus_wait_time").AsInt(),
      json.at("bus_velocity").AsDouble(),
  };
}

void TransportRouter::FillGraphWithStops(shared_ptr<const TransportInfo> transport_info) {
  Graph::VertexId vertex_id = 0;

  for (const auto& [stop_name, _] : transport_info->GetStopsRange()) {
    auto& vertex_ids = stops_vertex_ids_[stop_name];
    vertex_ids.in = vertex_id++;
    vertex_ids.out = vertex_id++;
    vertices_info_[vertex_ids.in] = {stop_name};
    vertices_info_[vertex_ids.out] = {stop_name};

    edges_info_.push_back(WaitEdgeInfo{});
    const Graph::EdgeId edge_id =
        graph_.AddEdge({vertex_ids.out, vertex_ids.in, static_cast<double>(routing_settings_.bus_wait_time)});
    assert(edge_id == edges_info_.size() - 1);
  }

  assert(vertex_id == graph_.GetVertexCount());
}

void TransportRouter::FillGraphWithBuses(shared_ptr<const TransportInfo> transport_info) {
  for (const auto& [_, bus] : transport_info->GetBusesRange()) {
    const size_t stop_count = bus.stops.size();
    if (stop_count <= 1) {
      continue;
    }

    for (size_t start_stop_idx = 0; start_stop_idx + 1 < stop_count; ++start_stop_idx) {
      const Graph::VertexId start_vertex = stops_vertex_ids_[bus.stops[start_stop_idx]].in;
      int total_distance = 0;
      for (size_t finish_stop_idx = start_stop_idx + 1; finish_stop_idx < stop_count; ++finish_stop_idx) {
        total_distance +=
            transport_info->ComputeStopsDistance(bus.stops[finish_stop_idx - 1], bus.stops[finish_stop_idx]);
        edges_info_.push_back(BusEdgeInfo{
            .bus_name = bus.name,
            .span_count = finish_stop_idx - start_stop_idx,
        });
        const Graph::EdgeId edge_id = graph_.AddEdge({
            start_vertex, stops_vertex_ids_[bus.stops[finish_stop_idx]].out,
            total_distance * 1.0 / (routing_settings_.bus_velocity * 1000.0 / 60)  // m / (km/h * 1000 / 60) = min
        });
        assert(edge_id == edges_info_.size() - 1);
      }
    }
  }
}

optional<TransportRouter::RouteInfo> TransportRouter::FindRoute(const string& stop_from, const string& stop_to) const {
  const Graph::VertexId vertex_from = stops_vertex_ids_.at(stop_from).out;
  const Graph::VertexId vertex_to = stops_vertex_ids_.at(stop_to).out;
  const auto route = router_->BuildRoute(vertex_from, vertex_to);
  if (!route) {
    return nullopt;
  }

  RouteInfo route_info = {.total_time = route->weight, .items = {}};
  route_info.items.reserve(route->edge_count);
  for (size_t edge_idx = 0; edge_idx < route->edge_count; ++edge_idx) {
    const Graph::EdgeId edge_id = router_->GetRouteEdge(route->id, edge_idx);
    const auto& edge = graph_.GetEdge(edge_id);
    const auto& edge_info = edges_info_[edge_id];
    if (holds_alternative<BusEdgeInfo>(edge_info)) {
      const BusEdgeInfo& bus_edge_info = get<BusEdgeInfo>(edge_info);
      route_info.items.push_back(RouteInfo::BusItem{
          .bus_name = bus_edge_info.bus_name,
          .time = edge.weight,
          .span_count = bus_edge_info.span_count,
      });
    } else {
      const Graph::VertexId vertex_id = edge.from;
      route_info.items.push_back(RouteInfo::WaitItem{
          .stop_name = vertices_info_[vertex_id].stop_name,
          .time = edge.weight,
      });
    }
  }

  // Releasing in destructor of some proxy object would be better,
  // but we do not expect exceptions in normal workflow
  router_->ReleaseRoute(route->id);
  return route_info;
}
