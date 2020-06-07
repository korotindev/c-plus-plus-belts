#include "BusStorage.h"

using namespace std;

Bus::Bus(std::string name_, std::vector<std::string> stopsNames_)
  : name(move(name_)),
    stopsNames(move(stopsNames_)) {}

void BusStorage::Add(Bus bus) {
  storage[bus.name] = bus.stopsNames;
  uniqueStorage[move(bus.name)].insert(make_move_iterator(bus.stopsNames.begin()),
                                       make_move_iterator(bus.stopsNames.end()));
}

size_t BusStorage::GetUniqueStopsCount(const std::string& busName) const {
  if (auto it = uniqueStorage.find(busName); it != uniqueStorage.end()) {
    return it->second.size();
  }
  return 0;
}

bool BusStorage::Exist(const std::string& busName) const {
  return storage.find(busName) != storage.end();
}

const vector<string>& BusStorage::GetStops(const std::string& busName) const {
  static const vector<string> defaultStops;
  if (auto it = storage.find(busName); it != storage.end()) {
    return it->second;
  }
  return defaultStops;
}
