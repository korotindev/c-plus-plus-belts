#include "map_renderer.h"

#include <algorithm>
#include <cassert>

#include "sphere.h"

using namespace std;

namespace {
  Svg::Polyline InitializeLineForBus(const string& bus_name, const MapRenderingSettings& settings) {
    Svg::Polyline line;

    line.SetStrokeColor(settings.GetBusColor(bus_name))
        .SetStrokeWidth(settings.render_settings_.line_width)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round");

    return line;
  }

  void RenderStopLabel(Svg::Document& svg, const string& data, const Svg::Point& stop_point,
                       const MapRenderingSettings& settings) {
    const auto base_text = Svg::Text{}
                               .SetPoint(stop_point)
                               .SetOffset(settings.render_settings_.stop_label_offset)
                               .SetFontSize(static_cast<uint32_t>(settings.render_settings_.stop_label_font_size))
                               .SetFontFamily("Verdana")
                               .SetData(data);
    svg.Add(Svg::Text(base_text)
                .SetFillColor(settings.render_settings_.underlayer_color)
                .SetStrokeColor(settings.render_settings_.underlayer_color)
                .SetStrokeWidth(settings.render_settings_.underlayer_width)
                .SetStrokeLineCap("round")
                .SetStrokeLineJoin("round"));
    svg.Add(Svg::Text(base_text).SetFillColor("black"));
  }

  void RenderEndpointStopLabel(Svg::Document& svg, const string& bus_name, const Svg::Point& stop_point,
                               const MapRenderingSettings& settings) {
    const auto color = settings.GetBusColor(bus_name);
    const auto base_text = Svg::Text{}
                               .SetPoint(stop_point)
                               .SetOffset(settings.render_settings_.bus_label_offset)
                               .SetFontSize(static_cast<uint32_t>(settings.render_settings_.bus_label_font_size))
                               .SetFontFamily("Verdana")
                               .SetFontWeight("bold")
                               .SetData(bus_name);
    svg.Add(Svg::Text(base_text)
                .SetFillColor(settings.render_settings_.underlayer_color)
                .SetStrokeColor(settings.render_settings_.underlayer_color)
                .SetStrokeWidth(settings.render_settings_.underlayer_width)
                .SetStrokeLineCap("round")
                .SetStrokeLineJoin("round"));
    svg.Add(Svg::Text(base_text).SetFillColor(color));
  }

  void RenderStopPoint(Svg::Document& svg, const Svg::Point& stop_point, const MapRenderingSettings& settings) {
    svg.Add(Svg::Circle{}.SetCenter(stop_point).SetRadius(settings.render_settings_.stop_radius).SetFillColor("white"));
  }

}  // namespace

MapRenderer::MapRenderer(const MapRenderingSettings& settings,
                         const Descriptions::BusesDict& buses_dict)
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
      for (const string& endpoint : bus_ptr->endpoints) {
        const auto stop_point = settings_.GetStopPoint(endpoint);
        RenderEndpointStopLabel(svg, bus_name, stop_point, settings_);
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