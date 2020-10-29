#pragma once

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <iterator>

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

  using StopPtr = std::shared_ptr<Stop>;
  using BusPtr = std::shared_ptr<Bus>;
  using ConstStopPtr = std::shared_ptr<const Stop>;
  using ConstBusPtr = std::shared_ptr<const Bus>;

  ConstStopPtr GetStop(const std::string& name) const;
  ConstBusPtr GetBus(const std::string& name) const;

  void AddStop(Descriptions::Stop&& stop_desc);
  void AddBus(Descriptions::Bus&& bus_desc);

  using StopsVector = std::vector<StopPtr>;
  using BusesVector = std::vector<BusPtr>;


  Range<ConstSharedPtrsVectorIterator<Stop>> GetStopsRange() const;
  Range<ConstSharedPtrsVectorIterator<Bus>> GetBusesRange() const;

  size_t StopsCount() const;
  size_t BusesCount() const;

  int ComputeStopsDistance(ConstStopPtr lhs, ConstStopPtr rhs) const;
  int ComputeStopsDistance(const std::string& lhs, const std::string& rhs) const;

 private:
  int ComputeRoadRouteLength(const std::vector<std::string>& stops);
  double ComputeGeoRouteDistance(const std::vector<std::string>& stops);

  StopsVector stops_;
  BusesVector buses_;
  std::unordered_map<std::string, StopPtr> indexed_stops_;
  std::unordered_map<std::string, BusPtr> indexed_buses_;
};