#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "descriptions.pb.h"
#include "database.pb.h"
#include "company.pb.h"
#include "json.h"
#include "sphere.h"

namespace Descriptions {
  struct Stop {
    std::string name;
    Sphere::Point position;
    std::unordered_map<std::string, size_t> distances;

    static Stop ParseFrom(const Json::Dict& attrs);
  };

  size_t ComputeStopsDistance(const Stop& lhs, const Stop& rhs);

  struct Bus {
    std::string name;
    std::vector<std::string> stops;
    std::vector<std::string> endpoints;

    static Bus ParseFrom(const Json::Dict& attrs);

    void Serialize(TCProto::BusDescription& proto) const;
    static Bus Deserialize(const TCProto::BusDescription& proto);
  };

  using InputQuery = std::variant<Stop, Bus, YellowPages::Company>;

  std::vector<InputQuery> ReadDescriptions(const Json::Array& nodes);
  YellowPages::Database ReadYellowPages(const Json::Dict& attrs);

  template <typename Object>
  using Dict = std::map<std::string, const Object*>;

  using StopsDict = Dict<Stop>;
  using BusesDict = Dict<Bus>;
}  // namespace Descriptions
