
#ifndef C_PLUS_PLUS_BELTS_BUSSTORAGE_H
#define C_PLUS_PLUS_BELTS_BUSSTORAGE_H

#include "Coordinate.h"
#include "ReadResponse.h"
#include "Router.h"
#include "StopsStorage.h"
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

class Database;

struct Bus {
  std::string name;
  std::vector<std::string> stopsNames;
  Bus(std::string name, std::vector<std::string> stopsNames);
};

class BusStorage {
  friend class Database;
  std::unordered_map<std::string, std::vector<std::string>> storage;
  std::unordered_map<std::string, std::unordered_set<std::string>> uniqueStorage;

public:
  void Add(Bus bus);
  const std::vector<std::string> &GetStops(const std::string &busName) const;
  bool Exist(const std::string &busName) const;
  size_t GetUniqueStopsCount(const std::string &busName) const;
};

#endif // C_PLUS_PLUS_BELTS_BUSSTORAGE_H
