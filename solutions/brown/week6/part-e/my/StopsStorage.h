#ifndef C_PLUS_PLUS_BELTS_STOPSSTORAGE_H
#define C_PLUS_PLUS_BELTS_STOPSSTORAGE_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <memory>
#include "ReadResponse.h"
#include "Coordinate.h"
#include "Router.h"
#include "CustomUtils.h"

class Database;

struct StopDistance {
  std::string name;
  double distance;
};

struct Stop {
  std::string name;
  Coordinate coordinate;
  std::vector<StopDistance> knownDistances;
  Stop(std::string, Coordinate, std::vector<StopDistance>);
};

struct StopData {
  Coordinate coordinate;
  std::set<std::string> buses;
};

struct Road {
  explicit Road(std::string from, std::string to);
  std::string from;
  std::string to;
};

struct RoadData {
  explicit RoadData(double distance);
  size_t id{};
  double distance;
};

struct RoadHasher {
  std::size_t operator()(const Road& road) const noexcept {
    std::size_t h1 = std::hash<std::string>{}(road.from);
    std::size_t h2 = std::hash<std::string>{}(road.to);
    return h1 ^ (h2 << 1);
  }
};

class StopsStorage {
  friend class Database;
  std::unordered_map<std::string, StopData> storage;
  std::unordered_map<Road, RoadData, RoadHasher> distanceStorage;
public:
  void Add(Stop stop);
  bool Exist(const std::string& busName) const;
  void AddBusToStop(const std::string& stopName, const std::string& busName);
  const std::set<std::string>& GetBuses(const std::string& stopName) const;
  double GetDistance(const std::string& lhsStopName, const std::string& rhsStopName) const;
  double GetDistanceV2(const std::string& lhsStopName, const std::string& rhsStopName) const;
};

bool operator==(const Road& lhs, const Road& rhs);
bool operator==(const StopDistance& lhs, const StopDistance& rhs);
std::ostream& operator<<(std::ostream& output, const StopDistance& data);

#endif //C_PLUS_PLUS_BELTS_STOPSSTORAGE_H
