#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <memory>

#include "descriptions.h"
#include "json.h"
#include "svg.h"
#include "transport_info.h"
#include "transport_router.h"
#include "utils.h"

class TransportCatalog {
 public:
  TransportCatalog(std::vector<Descriptions::InputQuery> data, const Json::Dict& routing_settings_json,
                   const Json::Dict& render_settings_json);

  const TransportInfo::Stop* GetStop(const std::string& name) const;
  const TransportInfo::Bus* GetBus(const std::string& name) const;

  std::optional<TransportRouter::RouteInfo> FindRoute(const std::string& stop_from, const std::string& stop_to) const;

  std::string RenderMap() const;

 private:
  std::shared_ptr<TransportInfo> transport_info_;
  std::unique_ptr<TransportRouter> router_;
  Svg::Document map_;
};
