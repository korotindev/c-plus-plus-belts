#include "Database.h"

using namespace std;

void Database::EntertainStop(Stop stop) {
  stopsStorage.Add(move(stop));
}

void Database::EntertainBus(Bus bus) {
  for (auto& stopName : bus.stopsNames) {
    stopsStorage.AddBusToStop(stopName, bus.name);
  }
  busStorage.Add(move(bus));
}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string& busName, const size_t requestId) {
  if (!busStorage.Exist(busName)) {
    return make_unique<ReadNoBusResponse>(busName, requestId);
  }

  const auto& stops = busStorage.GetStops(busName);
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

std::unique_ptr<ReadStopResponse> Database::ReadStop(const string& stopName, const size_t requestId) {
  if (!stopsStorage.Exist(stopName)) {
    return make_unique<ReadNoStopResponse>(stopName, requestId);
  }

  auto response = make_unique<ReadStopMetricsResponse>(stopName, requestId);
  const auto& sortedBuses = stopsStorage.GetBuses(stopName);
  response->buses = vector<string>(sortedBuses.begin(), sortedBuses.end());
  return response;
}

unique_ptr<ReadRouteResponse>
Database::ReadRoute(const size_t requestId, const std::string& fromStopName, const std::string& toStopName) {
  auto waitStopFrom = stopNameToWaitVertex.at(fromStopName);
  auto waitStopTo = stopNameToWaitVertex.at(toStopName);
  auto path = router->BuildRoute(waitStopFrom->id, waitStopTo->id);
  if (path == nullopt) {
    return make_unique<ReadNoRouteResponse>(requestId);
  }

  auto response = make_unique<ReadRouteMetricsResponse>(requestId);

  response->totalTime = path->weight;

  unique_ptr<ReadRouteResponseBusItem> currentBusItem;
  for (size_t i = 0; i < path->edge_count; i++) {
    auto edge_id = router->GetRouteEdge(path->id, i);
    const auto& edge = edges[edge_id];
    if (edge->type == CustomGraphEdgeType::Boarding) {
      if (currentBusItem != nullptr) {
        response->items.push_back(move(currentBusItem));
        currentBusItem = nullptr;
      }
      response->items.push_back(make_unique<ReadRouteResponseWaitItem>(edge->from->stopName, edge->weight));
    } else {
      if (currentBusItem == nullptr) {
        currentBusItem = make_unique<ReadRouteResponseBusItem>(edge->busName, edge->weight, 1);
      } else {
        currentBusItem->spanCount++;
        currentBusItem->time += edge->weight;
      }
    }
  }

  if (currentBusItem != nullptr) {
    response->items.push_back(move(currentBusItem));
  }

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

Database::CustomGraphVertex::CustomGraphVertex(
  size_t id_,
  Database::CustomGraphVertexType type_,
  std::string_view stopName_
) : id(id_),
    type(type_),
    stopName(stopName_) {}

Database::CustomGraphBusVertex::CustomGraphBusVertex(
  size_t id_,
  Database::CustomGraphVertexType type_,
  std::string_view stopName_,
  std::string_view busName_
) : CustomGraphVertex(id_, type_, stopName_), busName(busName_) {}

string Database::CustomGraphBusVertex::ToString() const {
  stringstream output;
  output << stopName << "(";
  if (type == CustomGraphVertexType::Wait) {
    output << "wait";
  } else {
    output << "ride on (" << busName << ")";
  }
  output << ")";
  return output.str();
}


shared_ptr<Database::CustomGraphBusVertex>
Database::FindOrCreateRideStop(std::string_view stopName, std::string_view busName) {
  auto& localStopsStorage = stopNameAndBusToRideVertex[stopName];

  if (auto it = localStopsStorage.find(busName); it != localStopsStorage.end()) {
    return it->second;
  }

  auto rideStop = make_shared<CustomGraphBusVertex>(
    vertices.size(),
    CustomGraphVertexType::Ride,
    stopName,
    busName
  );
  vertices.push_back(rideStop);
  stopNameAndBusToRideVertex[rideStop->stopName].emplace(busName, rideStop);
  return rideStop;
}

void Database::BuildRouter() {
  auto vertex_count = busStorage.storage.size() * stopsStorage.storage.size() + stopsStorage.storage.size();
  graph = make_unique<Graph::DirectedWeightedGraph<double>>(vertex_count);
  for (const auto &[stopName, _] : stopsStorage.storage) {
    auto fromVertex = make_shared<CustomGraphVertex>(
      vertices.size(),
      CustomGraphVertexType::Wait,
      stopName
    );
    vertices.push_back(fromVertex);
    stopNameToWaitVertex.emplace(fromVertex->stopName, fromVertex);
  }

  for (const auto &[busName, stopsNames] : busStorage.storage) {
    if (stopsNames.size() < 3) {
      continue;
    }
    for (size_t i = 0; i < stopsNames.size() - 1; i++) {
      auto& fromStopName = stopsNames[i];
      auto waitStopFrom = stopNameToWaitVertex.at(fromStopName);
      auto rideStopFrom = FindOrCreateRideStop(fromStopName, busName);
      auto& toStopName = stopsNames[i + 1];
      auto waitStopTo = stopNameToWaitVertex.at(toStopName);
      auto rideStopTo = FindOrCreateRideStop(toStopName, busName);

      auto boardingEdge = make_shared<CustomGraphEdge>(
        CustomGraphEdge{
          edges.size(),
          CustomGraphEdgeType::Boarding,
          static_cast<double>(Settings::GetBusWaitTime()),
          waitStopFrom,
          rideStopFrom,
          busName,
        }
      );
      edges.push_back(boardingEdge);
      graph->AddEdge(boardingEdge->ToGeneric());

      double weight = stopsStorage.GetDistanceV2(fromStopName, toStopName)
                      / (static_cast<double>(Settings::GetBusVelocity()) * 1000.0 / 60.0);

      auto rideToWaitVertexEdge = make_shared<CustomGraphEdge>(
        CustomGraphEdge{
          edges.size(),
          CustomGraphEdgeType::Riding,
          weight,
          rideStopFrom,
          waitStopTo,
          busName,
        }
      );
      edges.push_back(rideToWaitVertexEdge);
      graph->AddEdge(rideToWaitVertexEdge->ToGeneric());

      if (i + 1 != stopsNames.size() - 1) {
        auto rideToRideVertexEdge = make_shared<CustomGraphEdge>(
          CustomGraphEdge{
            edges.size(),
            CustomGraphEdgeType::Riding,
            weight,
            rideStopFrom,
            rideStopTo,
            busName,
          }
        );

        edges.push_back(rideToRideVertexEdge);
        graph->AddEdge(rideToRideVertexEdge->ToGeneric());
      }
    }
  }

  router = make_unique<Graph::Router<double>>(*graph.get());
}
