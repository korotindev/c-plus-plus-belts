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

class Stop {
  friend StopsStorage;
  std::string name;
  Coordinate coordinate;
public:
  Stop(std::string, Coordinate);
};

class Bus {
  friend BusStorage;
  std::string name;
  std::vector<std::string> stopsNames;
public:
  Bus(std::string name, std::vector<std::string> stopsNames);
};

class StopsStorage {
  struct PairHasher {
    std::size_t operator()(std::pair<std::string, std::string> const& s) const noexcept {
      std::size_t h1 = std::hash<std::string>{}(s.first);
      std::size_t h2 = std::hash<std::string>{}(s.second);
      return h1 ^ (h2 << 1);
    }
  };

  std::unordered_map<std::string, Coordinate> storage;
  mutable std::unordered_map<std::pair<std::string, std::string>, double, PairHasher> distanceStorage;
public:
  void Add(Stop stop);
  double GetDistance(const std::string& lhsStopName, const std::string& rhsStopName) const;
};

class BusStorage {
  std::unordered_map<std::string, std::vector<std::string>> storage;
  std::unordered_map<std::string, std::unordered_set<std::string>> uniqueStorage;
  std::unordered_map<std::string, std::set<std::string>> stopsToBuses;
public:
  void Add(Bus bus);
  const std::vector<std::string>& GetStops(const std::string& busName) const;
  const std::set<std::string>& GetBuses(const std::string& stopName) const;
  bool ExistBus(const std::string& busName) const;
  bool ExistStop(const std::string& busName) const;
  size_t GetUniqueStopsCount(const std::string& busName) const;
};

class Database {
  StopsStorage stopsStorage;
  BusStorage busStorage;

public:
  void EntertainStop(Stop stop);
  void EntertainBus(Bus bus);
  std::unique_ptr<ReadBusResponse> ReadBus(const std::string& busName);
  std::unique_ptr<ReadStopResponse> ReadStop(const std::string& stopName);
};


#endif //C_PLUS_PLUS_BELTS_DATABASE_H
