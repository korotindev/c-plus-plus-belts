#pragma once

#include <string>
#include <variant>

#include "filters.h"
#include "json.h"
#include "transport_catalog.h"
#include "datetime.h"

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
    CompaniesFilter filter;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct RouteToCompany {
    std::string stop_from;
    CompaniesFilter filter;
    DateTime datetime;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  std::variant<Stop, Bus, Route, Map, FindCompanies, RouteToCompany> Read(const Json::Dict& attrs);

  Json::Array ProcessAll(const TransportCatalog& db, const Json::Array& requests);
}  // namespace Requests
