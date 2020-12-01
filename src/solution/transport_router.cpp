#include "transport_router.h"

using namespace std;

TransportRouter::TransportRouter(Messages::TransportRouter message) {
  routing_settings_ = MakeRoutingSettings(move(*message.mutable_routing_settings()));
  graph_ = BusGraph(message.vertices_info_size());

  vertices_info_.resize(message.vertices_info_size());
  for (auto& vertex_info_msg : *message.mutable_vertices_info()) {
    vertices_info_.push_back(MakeVertexInfo(move(vertex_info_msg)));
  }

  stops_vertex_ids_.reserve(message.stops_vertex_ids_size());
  for (auto& stop_vertex_ids_msg : *message.mutable_stops_vertex_ids()) {
    const auto [it, _] = stops_vertex_ids_.emplace(MakeStopVertexIdsPair(move(stop_vertex_ids_msg)));
    graph_.AddEdge({it->second.out, it->second.in, static_cast<double>(routing_settings_.bus_wait_time)});
  }

  edges_info_.reserve(message.bus_edges_info_size());
  for (auto& bus_edge_info_msg : *message.mutable_bus_edges_info()) {
    auto bus_edge_info = MakeBusEdgeInfo(move(bus_edge_info_msg));
    graph_.AddEdge({bus_edge_info.start_vertex_id, bus_edge_info.finish_vertex_id, bus_edge_info.ride_time});
    edges_info_.emplace_back(move(bus_edge_info));
  }

  router_ = std::make_unique<Router>(graph_);
}

TransportRouter::TransportRouter(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
                                 const Json::Dict& routing_settings_json)
    : routing_settings_(MakeRoutingSettings(routing_settings_json)) {
  const size_t vertex_count = stops_dict.size() * 2;
  vertices_info_.resize(vertex_count);
  graph_ = BusGraph(vertex_count);

  FillGraphWithStops(stops_dict);
  FillGraphWithBuses(stops_dict, buses_dict);

  router_ = std::make_unique<Router>(graph_);
}

TransportRouter::RoutingSettings TransportRouter::MakeRoutingSettings(const Json::Dict& json) {
  return {
      json.at("bus_wait_time").AsInt(),
      json.at("bus_velocity").AsDouble(),
  };
}

void TransportRouter::FillGraphWithStops(const Descriptions::StopsDict& stops_dict) {
  Graph::VertexId vertex_id = 0;

  for (const auto& [stop_name, _] : stops_dict) {
    auto& vertex_ids = stops_vertex_ids_[stop_name];
    vertex_ids.in = vertex_id++;
    vertex_ids.out = vertex_id++;
    vertices_info_[vertex_ids.in] = {stop_name};
    vertices_info_[vertex_ids.out] = {stop_name};

    edges_info_.emplace_back(WaitEdgeInfo{});
    const Graph::EdgeId edge_id =
        graph_.AddEdge({vertex_ids.out, vertex_ids.in, static_cast<double>(routing_settings_.bus_wait_time)});
    assert(edge_id == edges_info_.size() - 1);
  }

  assert(vertex_id == graph_.GetVertexCount());
}

void TransportRouter::FillGraphWithBuses(const Descriptions::StopsDict& stops_dict,
                                         const Descriptions::BusesDict& buses_dict) {
  for (const auto& [_, bus_item] : buses_dict) {
    const auto& bus = *bus_item;
    const size_t stop_count = bus.stops.size();
    if (stop_count <= 1) {
      continue;
    }
    auto compute_distance_from = [&stops_dict, &bus](size_t lhs_idx) {
      return Descriptions::ComputeStopsDistance(*stops_dict.at(bus.stops[lhs_idx]),
                                                *stops_dict.at(bus.stops[lhs_idx + 1]));
    };
    for (size_t start_stop_idx = 0; start_stop_idx + 1 < stop_count; ++start_stop_idx) {
      const Graph::VertexId start_vertex = stops_vertex_ids_[bus.stops[start_stop_idx]].in;
      int total_distance = 0;
      for (size_t finish_stop_idx = start_stop_idx + 1; finish_stop_idx < stop_count; ++finish_stop_idx) {
        total_distance += compute_distance_from(finish_stop_idx - 1);
        auto bus_edge_info = BusEdgeInfo{
            .bus_name = bus.name,
            .start_stop_idx = start_stop_idx,
            .finish_stop_idx = finish_stop_idx,
            .start_vertex_id = start_vertex,
            .finish_vertex_id = stops_vertex_ids_[bus.stops[finish_stop_idx]].out,
            .ride_time =
                total_distance * 1.0 / (routing_settings_.bus_velocity * 1000.0 / 60),  // m / (km/h * 1000 / 60) = min
        };
        const Graph::EdgeId edge_id =
            graph_.AddEdge({bus_edge_info.start_vertex_id, bus_edge_info.finish_vertex_id, bus_edge_info.ride_time});
        edges_info_.push_back(move(bus_edge_info));

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
          .start_stop_idx = bus_edge_info.start_stop_idx,
          .finish_stop_idx = bus_edge_info.finish_stop_idx,
          .span_count = bus_edge_info.finish_stop_idx - bus_edge_info.start_stop_idx,
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

Messages::TransportRouter::RoutingSettings TransportRouter::RoutingSettings::Serialize() const {
  Messages::TransportRouter::RoutingSettings message;
  message.set_bus_velocity(bus_velocity);
  message.set_bus_wait_time(bus_wait_time);
  return message;
}

TransportRouter::RoutingSettings TransportRouter::MakeRoutingSettings(
    Messages::TransportRouter::RoutingSettings message) {
  return {.bus_wait_time = message.bus_wait_time(), .bus_velocity = message.bus_velocity()};
}

std::pair<std::string, TransportRouter::StopVertexIds> TransportRouter::MakeStopVertexIdsPair(
    Messages::TransportRouter::StopVertexIds message) {
  return {move(*message.mutable_name()), TransportRouter::StopVertexIds{.in = message.in(), .out = message.out()}};
}

Messages::TransportRouter::StopVertexIds TransportRouter::StopVertexIds::Serialize() const {
  Messages::TransportRouter::StopVertexIds message;
  message.set_in(in);
  message.set_out(out);
  return message;
}

TransportRouter::VertexInfo TransportRouter::MakeVertexInfo(Messages::TransportRouter::VertexInfo message) {
  return {.stop_name = move(*message.mutable_stop_name())};
}

Messages::TransportRouter::VertexInfo TransportRouter::VertexInfo::Serialize() const {
  Messages::TransportRouter::VertexInfo message;
  message.set_stop_name(stop_name);
  return message;
}

TransportRouter::BusEdgeInfo TransportRouter::MakeBusEdgeInfo(Messages::TransportRouter::BusEdgeInfo message) {
  return {.bus_name = move(*message.mutable_bus_name()),
          .start_stop_idx = message.start_stop_idx(),
          .finish_stop_idx = message.finish_stop_idx(),
          .start_vertex_id = message.start_vertex_id(),
          .finish_vertex_id = message.finish_vertex_id(),
          .ride_time = message.ride_time()};
}

Messages::TransportRouter::BusEdgeInfo TransportRouter::BusEdgeInfo::Serialize() const {
  Messages::TransportRouter::BusEdgeInfo message;
  message.set_bus_name(bus_name);
  message.set_start_stop_idx(start_stop_idx);
  message.set_finish_stop_idx(finish_stop_idx);
  message.set_start_vertex_id(start_vertex_id);
  message.set_finish_vertex_id(finish_vertex_id);
  message.set_ride_time(ride_time);
  return message;
}

Messages::TransportRouter TransportRouter::Serialize() const {
  Messages::TransportRouter message;
  *message.mutable_routing_settings() = routing_settings_.Serialize();
  for (const auto& vertex_info : vertices_info_) {
    *message.add_vertices_info() = vertex_info.Serialize();
  }

  for (const auto& [name, vertex_info] : stops_vertex_ids_) {
    auto stop_vertex_info_msg = vertex_info.Serialize();
    stop_vertex_info_msg.set_name(name);
    *message.add_stops_vertex_ids() = move(stop_vertex_info_msg);
  }

  for (const auto& edge_info : edges_info_) {
    if (holds_alternative<BusEdgeInfo>(edge_info)) {
      *message.add_bus_edges_info() = get<BusEdgeInfo>(edge_info).Serialize();
    }
  }

  return message;
}
