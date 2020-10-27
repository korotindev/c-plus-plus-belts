#pragma once 

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "sphere.h"
#include "utils.h"

class TransportInfo {
 public:
  struct Stop {
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, int> distances;
    std::set<std::string> bus_names;
  };

  struct Bus {
    std::string name;
    std::vector<std::string> stops;
    std::vector<std::string> endpoints;
    size_t unique_stop_count = 0;
    int road_route_length = 0;
    double geo_route_length = 0.0;
  };

  using StopsDict = std::unordered_map<std::string, Stop>;
  using BusesDict = std::unordered_map<std::string, Bus>;

  const Stop* GetStop(const std::string& name) const;
  const Bus* GetBus(const std::string& name) const;
  Stop* GetStop(const std::string& name);
  Bus* GetBus(const std::string& name);

  void AddStop(const Descriptions::Stop& stop);
  void AddBus(const Descriptions::Bus& bus);

  Range<StopsDict::const_iterator> GetStopsRange() const;
  Range<BusesDict::const_iterator> GetBusesRange() const;

  size_t StopsCount() const;
  size_t BusesCount() const;

  int ComputeStopsDistance(const Stop* lhs, const Stop* rhs) const;
  int ComputeStopsDistance(const std::string &lhs, const std::string &rhs) const;

 private:
  int ComputeRoadRouteLength(const std::vector<std::string>& stops);
  double ComputeGeoRouteDistance(const std::vector<std::string>& stops);

  StopsDict stops_;
  BusesDict buses_;
};