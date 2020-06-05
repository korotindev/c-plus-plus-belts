#ifndef C_PLUS_PLUS_BELTS_DATABASE_H
#define C_PLUS_PLUS_BELTS_DATABASE_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <memory>
#include "ReadResponse.h"
#include "Coordinate.h"

class StopsStorage;
class BusStorage;

struct StopDistance {
  std::string name;
  double distance;
};

bool operator==(const StopDistance& lhs, const StopDistance& rhs);
std::ostream& operator<<(std::ostream& output, const StopDistance& data);

struct Stop {
  std::string name;
  Coordinate coordinate;
  std::vector<StopDistance> knownDistances;
  Stop(std::string, Coordinate, std::vector<StopDistance>);
};


struct Bus {
  std::string name;
  std::vector<std::string> stopsNames;
  Bus(std::string name, std::vector<std::string> stopsNames);
};

class StopsStorage {
  struct StopData {
    Coordinate coordinate;
    std::set<std::string> buses;
  };

  struct PairHasher {
    std::size_t operator()(std::pair<std::string, std::string> const& s) const noexcept {
      std::size_t h1 = std::hash<std::string>{}(s.first);
      std::size_t h2 = std::hash<std::string>{}(s.second);
      return h1 ^ (h2 << 1);
    }
  };

  std::unordered_map<std::string, StopData> storage;
  std::unordered_map<std::pair<std::string, std::string>, double, PairHasher> distanceStorage;
public:
  void Add(Stop stop);
  bool Exist(const std::string& busName) const;
  void AddBusToStop(const std::string& stopName, const std::string& busName);
  const std::set<std::string>& GetBuses(const std::string& stopName) const;

  double GetDistance(const std::string& lhsStopName, const std::string& rhsStopName) const;
  double GetDistanceV2(const std::string& lhsStopName, const std::string& rhsStopName) const;
};

class BusStorage {
  std::unordered_map<std::string, std::vector<std::string>> storage;
  std::unordered_map<std::string, std::unordered_set<std::string>> uniqueStorage;
public:
  void Add(Bus bus);
  const std::vector<std::string>& GetStops(const std::string& busName) const;
  bool Exist(const std::string& busName) const;
  size_t GetUniqueStopsCount(const std::string& busName) const;
};

class Database {
  StopsStorage stopsStorage;
  BusStorage busStorage;

public:
  void EntertainStop(Stop stop);
  void EntertainBus(Bus bus);
  std::unique_ptr<ReadBusResponse> ReadBus(const std::string& busName, const size_t requestId);
  std::unique_ptr<ReadStopResponse> ReadStop(const std::string& stopName, const size_t requestId);
};

#endif //C_PLUS_PLUS_BELTS_DATABASE_H
