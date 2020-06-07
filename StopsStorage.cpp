#include "StopsStorage.h"

using namespace std;

Stop::Stop(std::string name, Coordinate coordinate, std::vector<StopDistance> knownDistances)
  : name(move(name)),
    coordinate(coordinate),
    knownDistances(move(knownDistances)) {}


bool operator==(const StopDistance& lhs, const StopDistance& rhs) {
  return make_pair(lhs.name, lhs.distance) == make_pair(rhs.name, rhs.distance);
}

std::ostream& operator<<(std::ostream& output, const StopDistance& data) {
  return output << "StopDistance{" << data.name << ", " << data.distance << "}";
}

void StopsStorage::Add(Stop stop) {
  static size_t current_id = 0;

  for (auto &[targetStopName, distance] : stop.knownDistances) {
    distanceStorage.emplace(Road(stop.name, targetStopName), RoadData(distance));
  }

  auto& stopData = storage[stop.name];
  stopData.id = current_id;
  stopData.coordinate = stop.coordinate;
  idToStop[current_id] = move(stop.name);
  current_id++;
}

double StopsStorage::GetDistanceV2(const string& lhsStopName, const string& rhsStopName) const {
  if (auto it = distanceStorage.find(Road(lhsStopName, rhsStopName)); it != distanceStorage.end()) {
    return it->second.distance;
  }

  if (auto it = distanceStorage.find(Road(rhsStopName, lhsStopName)); it != distanceStorage.end()) {
    return it->second.distance;
  }

  return 0.0;
}

double StopsStorage::GetDistance(const string& lhsStopName, const string& rhsStopName) const {
  auto lhsCoord = storage.find(lhsStopName)->second.coordinate;
  auto rhsCoord = storage.find(rhsStopName)->second.coordinate;
  auto distance = lhsCoord.GetDistance(rhsCoord);
  return distance;
}


const set<string>& StopsStorage::GetBuses(const string& stopName) const {
  static const set<string> defaultBuses;
  if (auto it = storage.find(stopName); it != storage.end()) {
    return it->second.buses;
  }
  return defaultBuses;
}

bool StopsStorage::Exist(const std::string& stopName) const {
  return storage.find(stopName) != storage.end();
}

void StopsStorage::AddBusToStop(const std::string& stopName, const std::string& busName) {
  storage[stopName].buses.insert(busName);
}

bool operator==(const StopsStorage::Road& lhs, const StopsStorage::Road& rhs) {
  return make_pair(lhs.from, lhs.to) == make_pair(rhs.from, rhs.to);
}

void StopsStorage::BuildRouter() {
  auto stopsCount = storage.size();
  graph = make_unique<Graph::DirectedWeightedGraph<double>>(stopsCount);
  for (auto &[road, roadData] : distanceStorage) {
    roadData.id = graph->AddEdge(
      Graph::Edge<double>{
        GetVertexIdFromStopName(road.from),
        GetVertexIdFromStopName(road.to),
        roadData.distance
      }
    );
    idToRoad.try_emplace(roadData.id, road);
  }
  router = make_unique<Graph::Router<double>>(*graph.get());
}

size_t StopsStorage::GetVertexIdFromStopName(const string& stopName) const {
  return storage.at(stopName).id;
}

const string& StopsStorage::GetStopNameFromVertexId(const size_t& vertexId) const {
  return idToStop.at(vertexId);
}

const StopsStorage::Road& StopsStorage::GetRoadFromEdgeId(const size_t& vertexId) const {
  return idToRoad.at(vertexId);
}

void StopsStorage::ReadRoute(const string& from, const string& to) {
  auto fromVertexId = GetVertexIdFromStopName(from);
  auto toVertexId = GetVertexIdFromStopName(to);
  auto routeInfo = router->BuildRoute(fromVertexId, toVertexId);

  if (routeInfo != nullopt) {
    cout << routeInfo->id << " " << routeInfo->weight << " " << routeInfo->edge_count << endl;
    for (size_t i = 0; i < routeInfo->edge_count; i++) {
      auto edgeId = router->GetRouteEdge(routeInfo->id, i);
      auto road = GetRoadFromEdgeId(edgeId);
      cout << road.from << " -> "  << road.to << endl;
    }
  }
  // TODO проверить, что делает этот метод
  // router->ReleaseRoute(routeInfo->id);
}

StopsStorage::Road::Road(std::string from_, std::string to_) {
  from = from_;
  to = to_;
}

StopsStorage::RoadData::RoadData(double distance_) {
  distance = distance_;
}
