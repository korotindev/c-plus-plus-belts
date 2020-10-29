#pragma once

#include <iterator>
#include <memory>
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
    size_t id;
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, int> distances;
    std::set<std::string, size_t> bus_names;
    std::unordered_map<size_t, size_t> buses_stat;
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

  ConstStopPtr GetStop(const size_t id) const;
  ConstBusPtr GetBus(const size_t id) const;

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
  std::unordered_map<std::string, StopPtr> name_indexed_stops_;
  std::unordered_map<size_t, StopPtr> id_indexed_stops_;

  std::unordered_map<std::string, BusPtr> name_indexed_buses_;
  std::unordered_map<size_t, BusPtr> id_indexed_buses_;
};