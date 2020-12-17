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

  // struct YellowPages {
  //   struct Rubric {
  //     uint64_t id;
  //     std::string name;
  //     std::vector<std::string> keywords;

  //     static Rubric ParseFrom(const Json::Dict& attrs);
  //   };

  //   struct Company {
  //     struct Name {
  //       std::string value;
  //       enum Type { MAIN, SYNONYM, SHORT } type;

  //       static Name ParseFrom(const Json::Dict& attrs);
  //     };
  //     struct Phone {
  //       std::string formatted;
  //       enum Type { PHONE, FAX } type;
  //       std::string country_code;
  //       std::string local_code;
  //       std::string number;
  //       std::string extension;
  //       std::string description;

  //       static Phone ParseFrom(const Json::Dict& attrs);
  //     };
  //     struct Url {
  //       std::string value;

  //       static Url ParseFrom(const Json::Dict& attrs);
  //     };

  //     std::vector<Name> names;
  //     std::vector<Phone> phones;
  //     std::vector<Url> urls;
  //     std::vector<uint64_t> rubric_ids;

  //     static Company ParseFrom(const Json::Dict& attrs);
  //   };

  //   std::vector<Company> companies;
  //   std::unordered_map<uint64_t, Rubric> rubrics;
  // };

  using InputQuery = std::variant<Stop, Bus, YellowPages::Company>;

  std::vector<InputQuery> ReadDescriptions(const Json::Array& nodes);
  YellowPages::Database ReadYellowPages(const Json::Dict& attrs);

  template <typename Object>
  using Dict = std::map<std::string, const Object*>;

  using StopsDict = Dict<Stop>;
  using BusesDict = Dict<Bus>;
}  // namespace Descriptions
