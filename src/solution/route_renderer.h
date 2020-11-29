#pragma once

#include <functional>
#include <iostream>
#include <unordered_map>
#include <string>

#include "map_rendering_settings.h"
#include "svg.h"
#include "transport_router.h"

class RouteRenderer {
 public:
  RouteRenderer(const MapRenderingSettings& map_rendering_settings, const Descriptions::BusesDict& buses_dict,
                const TransportRouter::RouteInfo& route_info, const std::string& stop_to);
  Svg::Document Render() const;

 private:
  void RenderBusLines(Svg::Document& svg) const;
  void RenderBusLabels(Svg::Document& svg) const;
  void RenderStopPoints(Svg::Document& svg) const;
  void RenderStopLabels(Svg::Document& svg) const;

  using BusProcessor = void(const TransportRouter::RouteInfo::BusItem& bus, size_t from, size_t to);
  
  void RenderRouteLine(const std::function<BusProcessor> &processor) const;

  const MapRenderingSettings& settings_;
  const Descriptions::BusesDict& buses_dict_;
  const TransportRouter::RouteInfo& route_info_;
  const std::string& stop_to_;
  const static std::unordered_map<std::string, void (RouteRenderer::*)(Svg::Document&) const> LAYER_ACTIONS;
};