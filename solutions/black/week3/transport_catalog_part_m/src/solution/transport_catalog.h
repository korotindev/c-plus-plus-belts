#pragma once

#include <memory>
#include <string>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "svg.h"
#include "transport_info.h"
#include "transport_router.h"
#include "map_renderer.h"

class TransportCatalog {
 public:
  TransportCatalog(std::vector<Descriptions::InputQuery> data, const Json::Dict& routing_settings_json,
                   const Json::Dict& render_settings_json, std::unique_ptr<IMapRenderer> renderer);

  std::shared_ptr<const TransportInfo::Stop> GetStop(const std::string& name) const;
  std::shared_ptr<const TransportInfo::Bus> GetBus(const std::string& name) const;

  std::optional<TransportRouter::RouteInfo> FindRoute(const std::string& stop_from, const std::string& stop_to) const;

  void SetRenderer(std::unique_ptr<IMapRenderer> renderer);
  std::string RenderMap() const;

 private:
  std::shared_ptr<TransportInfo> transport_info_;
  std::unique_ptr<TransportRouter> router_;
  Svg::Document map_;
};
