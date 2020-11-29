#include "route_renderer.h"

#include <algorithm>
#include <cassert>

#include "map_renderer.h"
#include "sphere.h"
#include "map_rendering_utils.h"

using namespace std;

RouteRenderer::RouteRenderer(const MapRenderingSettings& settings, const Descriptions::BusesDict& buses_dict)
    : settings_(settings), buses_dict_(buses_dict) {}

void RouteRenderer::RenderBusLines(Svg::Document& svg) const {
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

void RouteRenderer::RenderBusLabels(Svg::Document& svg) const {
  for (const auto& [bus_name, bus_ptr] : buses_dict_) {
    const auto& stops = bus_ptr->stops;
    if (!stops.empty()) {
      for (const string& stop_name : bus_ptr->endpoints) {
        RenderEndpointStopLabel(svg, bus_name, stop_name, settings_);
      }
    }
  }
}

void RouteRenderer::RenderStopPoints(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : settings_.map_settings_.stops_coords_) {
    RenderStopPoint(svg, stop_point, settings_);
  }
}

void RouteRenderer::RenderStopLabels(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : settings_.map_settings_.stops_coords_) {
    RenderStopLabel(svg, stop_name, stop_point, settings_);
  }
}

const unordered_map<string, void (RouteRenderer::*)(Svg::Document&) const> RouteRenderer::LAYER_ACTIONS = {
    {"bus_lines", &RouteRenderer::RenderBusLines},
    {"bus_labels", &RouteRenderer::RenderBusLabels},
    {"stop_points", &RouteRenderer::RenderStopPoints},
    {"stop_labels", &RouteRenderer::RenderStopLabels},
};

Svg::Document RouteRenderer::Render() const {
  Svg::Document svg = MapRenderer(settings_, buses_dict_).Render();

  RenderBigWhiteSquare(svg, settings_);

  for (const auto& layer : settings_.render_settings_.layers) {
    (this->*LAYER_ACTIONS.at(layer))(svg);
  }

  return svg;
}