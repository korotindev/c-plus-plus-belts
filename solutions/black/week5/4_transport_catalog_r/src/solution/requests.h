#pragma once

#include "json.h"
#include "transport_catalog.h"
#include "phone.pb.h"

#include <string>
#include <variant>


namespace Requests {
  struct Stop {
    std::string name;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct Bus {
    std::string name;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct Route {
    std::string stop_from;
    std::string stop_to;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct Map {
    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct FindCompanies {
    std::vector<std::string> names;
    std::vector<std::string> rubrics;
    std::vector<std::string> urls;
    std::vector<YellowPages::Phone> phones;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  std::variant<Stop, Bus, Route, Map, FindCompanies> Read(const Json::Dict& attrs);

  Json::Array ProcessAll(const TransportCatalog& db, const Json::Array& requests);
}
