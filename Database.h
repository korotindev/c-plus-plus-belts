#ifndef C_PLUS_PLUS_BELTS_DATABASE_H
#define C_PLUS_PLUS_BELTS_DATABASE_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "Coordinate.h"

class StopsStorage;

class Stop {
  friend StopsStorage;
  std::string name;
  Coordinate coordinate;
public:
  Stop(std::string, Coordinate);
};

class BusStorage;

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
public:
  void Add(Bus bus);
  const std::vector<std::string>& GetStops(const std::string& busName) const;
  size_t GetUniqueStopsCount(const std::string& busName) const;
};

struct ReadBusResponse {
  std::string busName;
  explicit ReadBusResponse(std::string busName_);
  virtual ~ReadBusResponse() = default;
  virtual void Print(std::ostream& output) = 0;
};

struct ReadNoBusResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  void Print(std::ostream& output) override;
};

struct ReadBusMetricsResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  void Print(std::ostream& output) override;
  size_t stopsCount;
  size_t uniqueStopsCount;
  double routeDistance;
};

class Database {
  StopsStorage stopsStorage;
  BusStorage busStorage;

public:
  void EntertainStop(Stop stop);
  void EntertainBus(Bus bus);
  std::unique_ptr<ReadBusResponse> ReadBus(const std::string& busName);
};


#endif //C_PLUS_PLUS_BELTS_DATABASE_H
