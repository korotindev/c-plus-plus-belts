#include "Database.h"

#include <utility>

using namespace std;

void Database::EntertainStop(Stop stop) { stopsStorage.Add(move(stop)); }

void Database::EntertainBus(Bus bus) {
  for (auto &stopName : bus.stopsNames) {
    stopsStorage.AddBusToStop(stopName, bus.name);
  }
  busStorage.Add(move(bus));
}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string &busName, const size_t requestId) {
  if (!busStorage.Exist(busName)) {
    return make_unique<ReadNoBusResponse>(busName, requestId);
  }

  const auto &stops = busStorage.GetStops(busName);
  auto response = make_unique<ReadBusMetricsResponse>(busName, requestId);

  response->stopsCount = stops.size();
  response->uniqueStopsCount = busStorage.GetUniqueStopsCount(busName);
  response->routeDistance = 0.0;
  response->routeDistanceV2 = 0.0;
  for (size_t i = 1; i < stops.size(); i++) {
    response->routeDistance += stopsStorage.GetDistance(stops[i - 1], stops[i]);
    response->routeDistanceV2 += stopsStorage.GetDistanceV2(stops[i - 1], stops[i]);
  }
  return response;
}

std::unique_ptr<ReadStopResponse> Database::ReadStop(const string &stopName, const size_t requestId) {
  if (!stopsStorage.Exist(stopName)) {
    return make_unique<ReadNoStopResponse>(stopName, requestId);
  }

  auto response = make_unique<ReadStopMetricsResponse>(stopName, requestId);
  const auto &sortedBuses = stopsStorage.GetBuses(stopName);
  response->buses = vector<string>(sortedBuses.begin(), sortedBuses.end());
  return response;
}

unique_ptr<ReadRouteResponse> Database::ReadRoute(const size_t requestId, const std::string &fromStopName,
                                                  const std::string &toStopName) {
  auto waitStopFrom = stopNameToWaitVertex.at(fromStopName);
  auto waitStopTo = stopNameToWaitVertex.at(toStopName);
  auto path = router->BuildRoute(waitStopFrom->id, waitStopTo->id);
  if (path == nullopt) {
    router->ReleaseRoute(path->id);
    return make_unique<ReadNoRouteResponse>(requestId);
  }

  auto response = make_unique<ReadRouteMetricsResponse>(requestId);

  response->totalTime = path->weight;

  for (size_t i = 0; i < path->edge_count; i++) {
    auto edge_id = router->GetRouteEdge(path->id, i);
    const auto &edge = edges[edge_id];
    if (edge->type == CustomGraphEdgeType::Boarding) {
      response->items.push_back(make_unique<ReadRouteResponseWaitItem>(edge->from->stopName, edge->weight));
    } else {
      response->items.emplace_back(
          make_unique<ReadRouteResponseBusItem>(edge->busName, edge->weight, edge->span_count));
    }
  }

  router->ReleaseRoute(path->id);
  return response;
}

Graph::Edge<double> Database::CustomGraphEdge::ToGeneric() const {
  return {
      from->id,
      to->id,
      weight,
  };
}

string Database::CustomGraphEdge::ToString() const {
  stringstream output;

  if (type == CustomGraphEdgeType::Riding) {
    output << "Riding on: " << busName;
  } else {
    output << "Boarding to: " << busName;
  }
  output << " from " << from->ToString() << " to " << to->ToString();
  output << " take " << weight;
  return output.str();
}

string Database::CustomGraphVertex::ToString() const {
  stringstream output;
  output << stopName << "(";
  if (type == CustomGraphVertexType::Wait) {
    output << "wait";
  } else {
    output << "ride";
  }
  output << ")";
  return output.str();
}

Database::CustomGraphVertex::CustomGraphVertex(size_t id_, Database::CustomGraphVertexType type_,
                                               std::string_view stopName_)
    : id(id_), type(type_), stopName(stopName_) {}

shared_ptr<Database::CustomGraphVertex>
Database::findOrCreateStop(std::unordered_map<std::string_view, std::shared_ptr<CustomGraphVertex>> &store,
                           CustomGraphVertexType type, std::string_view stopName) {
  if (auto it = store.find(stopName); it != store.end()) {
    return it->second;
  }

  auto stop = make_shared<CustomGraphVertex>(vertices.size(), type, stopName);
  vertices.push_back(stop);
  store[stopName] = stop;

  return stop;
}

void Database::createEdge(CustomGraphEdgeType type, shared_ptr<Database::CustomGraphVertex> from,
                          shared_ptr<Database::CustomGraphVertex> to, std::string_view busName, double weigh,
                          size_t span_count) {
  auto edge = make_shared<CustomGraphEdge>(
      CustomGraphEdge{edges.size(), type, weigh, std::move(from), std::move(to), busName, span_count});
  edges.push_back(edge);
  graph->AddEdge(edge->ToGeneric());
}

void Database::BuildRouter() {
  auto vertex_count = 2 * stopsStorage.storage.size();
  graph = make_unique<Graph::DirectedWeightedGraph<double>>(vertex_count);

  auto diffDistance = [this](const string &from, const string &to) {
    return stopsStorage.GetDistanceV2(from, to) / (static_cast<double>(Settings::GetBusVelocity()) * 1000.0 / 60.0);
  };

  for (const auto &[stopName, stopData] : stopsStorage.storage) {
    auto fromVertex = findOrCreateStop(stopNameToWaitVertex, CustomGraphVertexType::Wait, stopName);
    auto toVertex = findOrCreateStop(stopNameToRideVertex, CustomGraphVertexType::Ride, stopName);
    createEdge(CustomGraphEdgeType::Boarding, fromVertex, toVertex, "", static_cast<double>(Settings::GetBusWaitTime()),
               0);
  }

  for (const auto &[busName, stopsNames] : busStorage.storage) {
    if (stopsNames.size() < 3) {
      continue;
    }

    for (size_t i = 0; i < stopsNames.size(); i++) {
      const auto &fromStopName = stopsNames[i];
      auto fromVertex = findOrCreateStop(stopNameToRideVertex, CustomGraphVertexType::Ride, fromStopName);
      double diffSum = 0;
      for (size_t j = i + 1; j < stopsNames.size(); j++) {
        auto currentDiff = diffDistance(stopsNames[j - 1], stopsNames[j]);
        diffSum += currentDiff;
        const auto &toStopName = stopsNames[j];
        auto toVertex = findOrCreateStop(stopNameToWaitVertex, CustomGraphVertexType::Wait, toStopName);
        size_t span_count = j - i;
        createEdge(CustomGraphEdgeType::Riding, fromVertex, toVertex, busName, diffSum, span_count);
      }
    }
  }
  router = make_unique<Graph::Router<double>>(*graph.get());
}
