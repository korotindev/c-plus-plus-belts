#include "map_rendering_utils.h"

using namespace std;

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

void RenderEndpointStopLabel(Svg::Document& svg, const string& bus_name, const string& stop_name,
                             const MapRenderingSettings& settings) {
  const auto stop_point = settings.GetStopPoint(stop_name);
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

void RenderBigWhiteSquare(Svg::Document& svg, const MapRenderingSettings& settings) {
  svg.Add(Svg::Rect{}
              .SetPoint({.x = -settings.render_settings_.outer_margin, .y = -settings.render_settings_.outer_margin})
              .SetHeight(settings.render_settings_.max_height + 2.0 * settings.render_settings_.outer_margin)
              .SetWidth(settings.render_settings_.max_width + 2.0 * settings.render_settings_.outer_margin)
              .SetFillColor(settings.render_settings_.underlayer_color));
}