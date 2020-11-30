#include "map_renderer.h"

#include <algorithm>
#include <cassert>

#include "map_rendering_utils.h"
#include "sphere.h"

using namespace std;

MapRenderer::MapRenderer(const MapRenderingSettings& settings, const Descriptions::BusesDict& buses_dict)
    : settings_(settings), buses_dict_(buses_dict) {}

void MapRenderer::RenderBusLines(Svg::Document& svg) const {
  for (const auto& [bus_name, bus_ptr] : buses_dict_) {
    const auto& stops = bus_ptr->stops;
    if (stops.empty()) {
      continue;
    }
    auto line = InitializeLineForBus(bus_name, settings_);
    for (const auto& stop_name : stops) {
      line.AddPoint(settings_.GetStopPoint(stop_name));
    }
    svg.Add(line);
  }
}

void MapRenderer::RenderBusLabels(Svg::Document& svg) const {
  for (const auto& [bus_name, bus_ptr] : buses_dict_) {
    const auto& stops = bus_ptr->stops;
    if (!stops.empty()) {
      for (const string& stop_name : bus_ptr->endpoints) {
        RenderEndpointStopLabel(svg, bus_name, stop_name, settings_);
      }
    }
  }
}

void MapRenderer::RenderStopPoints(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : settings_.map_settings_.stops_coords_) {
    RenderStopPoint(svg, stop_point, settings_);
  }
}

void MapRenderer::RenderStopLabels(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : settings_.map_settings_.stops_coords_) {
    RenderStopLabel(svg, stop_name, stop_point, settings_);
  }
}

const unordered_map<string, void (MapRenderer::*)(Svg::Document&) const> MapRenderer::LAYER_ACTIONS = {
    {"bus_lines", &MapRenderer::RenderBusLines},
    {"bus_labels", &MapRenderer::RenderBusLabels},
    {"stop_points", &MapRenderer::RenderStopPoints},
    {"stop_labels", &MapRenderer::RenderStopLabels},
};

Svg::Document MapRenderer::Render() const {
  Svg::Document svg;

  for (const auto& layer : settings_.render_settings_.layers) {
    (this->*LAYER_ACTIONS.at(layer))(svg);
  }

  return svg;
}