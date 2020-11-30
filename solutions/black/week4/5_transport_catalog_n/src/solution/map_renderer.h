#pragma once

#include <unordered_map>
#include <iostream>
#include "svg.h"

#include "map_rendering_settings.h"
#include "transport_catalog.h"

class MapRenderer {
 public:
  MapRenderer(const MapRenderingSettings& map_rendering_settings,
                           const Descriptions::BusesDict& buses_dict);
  Svg::Document Render() const;

 private:
  void RenderBusLines(Svg::Document& svg) const;
  void RenderBusLabels(Svg::Document& svg) const;
  void RenderStopPoints(Svg::Document& svg) const;
  void RenderStopLabels(Svg::Document& svg) const;

  const MapRenderingSettings& settings_;
  const Descriptions::BusesDict& buses_dict_;
  const static std::unordered_map<std::string, void (MapRenderer::*)(Svg::Document&) const> LAYER_ACTIONS;
};