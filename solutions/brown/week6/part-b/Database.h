#ifndef C_PLUS_PLUS_BELTS_DATABASE_H
#define C_PLUS_PLUS_BELTS_DATABASE_H

#include "Coordinate.h"
#include "ReadResponse.h"
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class StopsStorage;
class BusStorage;

struct Stop {
  std::string name;
  Coordinate coordinate;
  Stop(std::string, Coordinate);
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
    std::size_t operator()(std::pair<std::string, std::string> const &s) const noexcept {
      std::size_t h1 = std::hash<std::string>{}(s.first);
      std::size_t h2 = std::hash<std::string>{}(s.second);
      return h1 ^ (h2 << 1);
    }
  };

  std::unordered_map<std::string, StopData> storage;
  mutable std::unordered_map<std::pair<std::string, std::string>, double, PairHasher> distanceStorage;

public:
  void Add(Stop stop);
  bool Exist(const std::string &busName) const;
  void AddBusToStop(const std::string &stopName, const std::string &busName);
  double GetDistance(const std::string &lhsStopName, const std::string &rhsStopName) const;
  const std::set<std::string> &GetBuses(const std::string &stopName) const;
};

class BusStorage {
  std::unordered_map<std::string, std::vector<std::string>> storage;
  std::unordered_map<std::string, std::unordered_set<std::string>> uniqueStorage;

public:
  void Add(Bus bus);
  const std::vector<std::string> &GetStops(const std::string &busName) const;
  bool Exist(const std::string &busName) const;
  size_t GetUniqueStopsCount(const std::string &busName) const;
};

class Database {
  StopsStorage stopsStorage;
  BusStorage busStorage;

public:
  void EntertainStop(Stop stop);
  void EntertainBus(Bus bus);
  std::unique_ptr<ReadBusResponse> ReadBus(const std::string &busName);
  std::unique_ptr<ReadStopResponse> ReadStop(const std::string &stopName);
};

#endif // C_PLUS_PLUS_BELTS_DATABASE_H
