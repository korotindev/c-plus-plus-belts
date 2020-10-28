#pragma once 

#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "descriptions.h"
#include "sphere.h"
#include "utils.h"

class TransportInfo {
 public:
  struct Stop {
    size_t id;
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, int> distances;
    std::set<std::string> bus_names;
  };

  struct Bus {
    size_t id;
    std::string name;
    std::vector<std::string> stops;
    std::vector<std::string> endpoints;
    size_t unique_stop_count = 0;
    int road_route_length = 0;
    double geo_route_length = 0.0;
  };

  std::shared_ptr<const Stop> GetStop(const std::string& name) const;
  std::shared_ptr<const Bus> GetBus(const std::string& name) const;
  std::shared_ptr<Stop> GetStop(const std::string& name);
  std::shared_ptr<Bus> GetBus(const std::string& name);

  void AddStop(Descriptions::Stop&& stop_desc);
  void AddBus(Descriptions::Bus&& bus_desc);

  using StopsVector = std::vector<std::shared_ptr<Stop>>;
  using BusesVector = std::vector<std::shared_ptr<Bus>>;

  Range<StopsVector::const_iterator> GetStopsRange() const;
  Range<BusesVector::const_iterator> GetBusesRange() const;

  size_t StopsCount() const;
  size_t BusesCount() const;

  int ComputeStopsDistance(std::shared_ptr<const Stop> lhs, std::shared_ptr<const Stop> rhs) const;
  int ComputeStopsDistance(const std::string &lhs, const std::string &rhs) const;

 private:
  int ComputeRoadRouteLength(const std::vector<std::string>& stops);
  double ComputeGeoRouteDistance(const std::vector<std::string>& stops);
  
  StopsVector stops_;
  BusesVector buses_;
  std::unordered_map<std::string, std::shared_ptr<Stop>> indexed_stops_;
  std::unordered_map<std::string, std::shared_ptr<Bus>> indexed_buses_;
};