#pragma once

#include "json.h"
#include "sphere.h"

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include "transport_catalog.pb.h"


namespace Descriptions {
  struct Stop {
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, int> distances;

    static Stop ParseFrom(const Json::Dict& attrs);
  };

  int ComputeStopsDistance(const Stop& lhs, const Stop& rhs);

  struct Bus {
    std::string name;
    std::vector<std::string> stops;
    std::vector<std::string> endpoints;
    
    Messages::Descriptions::Bus Serialize() const;
    static Bus ParseFrom(const Json::Dict& attrs);
    static Bus ParseFrom(Messages::Descriptions::Bus message);
  };

  using InputQuery = std::variant<Stop, Bus>;

  std::vector<InputQuery> ReadDescriptions(const Json::Array& nodes);

  template <typename Object>
  using Dict = std::map<std::string, const Object*>;

  using StopsDict = Dict<Stop>;
  using BusesDict = Dict<Bus>;
}
