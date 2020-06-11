#ifndef C_PLUS_PLUS_BELTS_DATABASE_H
#define C_PLUS_PLUS_BELTS_DATABASE_H

#include <memory>
#include <vector>
#include <string_view>
#include <unordered_map>
#include "ReadResponse.h"
#include "Graph.h"
#include "Coordinate.h"
#include "Router.h"
#include "StopsStorage.h"
#include "BusStorage.h"
#include "Settings.h"

class Database {
  enum CustomGraphVertexType { Wait, Ride };
  enum CustomGraphEdgeType { Boarding, Riding };

  struct CustomGraphVertex {
    CustomGraphVertex(size_t id, CustomGraphVertexType type, std::string_view stopName);
    virtual ~CustomGraphVertex() {}
    size_t id;
    CustomGraphVertexType type;
    std::string_view stopName;
    virtual std::string ToString() const;
  };

  struct CustomGraphBusVertex : public CustomGraphVertex {
    CustomGraphBusVertex(size_t id, CustomGraphVertexType type, std::string_view stopName, std::string_view busName);
    std::string_view busName;
    std::string ToString() const override;
  };

  struct CustomGraphEdge {
    size_t id;
    CustomGraphEdgeType type;
    double weight;
    std::shared_ptr<CustomGraphVertex> from;
    std::shared_ptr<CustomGraphVertex> to;
    std::string_view busName;
    Graph::Edge<double> ToGeneric() const;
    std::string ToString() const;
  };

  std::unique_ptr<Graph::DirectedWeightedGraph<double>> graph;
  std::unique_ptr<Graph::Router<double>> router;
  std::unordered_map<std::string_view, std::shared_ptr<CustomGraphVertex>> stopNameToWaitVertex;
  std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::shared_ptr<CustomGraphBusVertex>>> stopNameAndBusToRideVertex;
  std::vector<std::shared_ptr<CustomGraphVertex>> vertices;
  std::vector<std::shared_ptr<CustomGraphEdge>> edges;
  StopsStorage stopsStorage;
  BusStorage busStorage;
  std::shared_ptr<CustomGraphBusVertex> FindOrCreateRideStop(std::string_view stopName, std::string_view busName);
public:
  void BuildRouter();
  void EntertainStop(Stop stop);
  void EntertainBus(Bus bus);
  std::unique_ptr<ReadBusResponse> ReadBus(const std::string& busName, const size_t requestId);
  std::unique_ptr<ReadStopResponse> ReadStop(const std::string& stopName, const size_t requestId);
  std::unique_ptr<ReadRouteResponse> ReadRoute(const size_t requestId, const std::string& from, const std::string& to);
};

#endif //C_PLUS_PLUS_BELTS_DATABASE_H
