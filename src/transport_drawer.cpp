#include "transport_drawer.h"

#include <sstream>

using namespace std;

Svg::Color ParseColor(const Json::Node &node) {
  variant<Svg::Rgb, Svg::Rgba, string> color;

  if (holds_alternative<string>(node.GetBase())) {
    color = node.AsString();
  } else {
    auto rgb_args = node.AsArray();
    if (rgb_args.size() == 3) {
      color = Svg::Rgb{.red = static_cast<uint8_t>(rgb_args[0].AsInt()),
                       .green = static_cast<uint8_t>(rgb_args[1].AsInt()),
                       .blue = static_cast<uint8_t>(rgb_args[2].AsInt())};
    } else {
      color = Svg::Rgba{.red = static_cast<uint8_t>(rgb_args[0].AsInt()),
                        .green = static_cast<uint8_t>(rgb_args[1].AsInt()),
                        .blue = static_cast<uint8_t>(rgb_args[2].AsInt()),
                        .alpha = rgb_args[3].AsDouble()};
    }
  }

  return visit([](auto t) { return Svg::Color(t); }, color);
}

TransportDrawer::TransportDrawer(const Descriptions::StopsDict &stops_dict, const Descriptions::BusesDict &buses_dict,
                                 const Json::Dict &render_settings_json)
    : stops_dict_(stops_dict),
      buses_dict_(buses_dict),
      render_settings_(MakeRenderSettings(render_settings_json)),
      projection_settings_(MakeProjectionSettings(render_settings_, stops_dict)) {}

TransportDrawer::RenderSettings TransportDrawer::MakeRenderSettings(const Json::Dict &json) {
  const auto &palette = json.at("color_palette").AsArray();
  vector<Svg::Color> color_palette;
  transform(palette.cbegin(), palette.cend(), back_inserter(color_palette),
            [](auto &node) { return ParseColor(node); });

  RenderSettings render_settings{
      .width = json.at("width").AsDouble(),
      .height = json.at("height").AsDouble(),
      .padding = json.at("padding").AsDouble(),
      .stop_radius = json.at("stop_radius").AsDouble(),
      .line_width = json.at("line_width").AsDouble(),
      .stop_label_font_size = static_cast<uint32_t>(json.at("stop_label_font_size").AsInt()),
      .stop_label_offset = Svg::Point{.x = json.at("stop_label_offset").AsArray()[0].AsDouble(),
                                      .y = json.at("stop_label_offset").AsArray()[1].AsDouble()},
      .underlayer_color = ParseColor(json.at("underlayer_color")),
      .underlayer_width = json.at("underlayer_width").AsDouble(),
      .color_palette = move(color_palette)};

  return render_settings;
}

TransportDrawer::ProjectionSettings TransportDrawer::MakeProjectionSettings(
    const TransportDrawer::RenderSettings &render_settings, const Descriptions::StopsDict &stops_dict) {
  ProjectionSettings projection_settings;

  for (auto &[_, stop] : stops_dict) {
    projection_settings.max_lat = max(projection_settings.max_lat, stop->position.latitude);
    projection_settings.min_lat = min(projection_settings.min_lat, stop->position.latitude);

    projection_settings.max_lon = max(projection_settings.max_lon, stop->position.longitude);
    projection_settings.min_lon = min(projection_settings.min_lon, stop->position.longitude);
  }

  double width_zoom_coef = -1;
  if (projection_settings.max_lon != projection_settings.min_lon) {
    width_zoom_coef = (render_settings.width - 2 * render_settings.padding) /
                      (projection_settings.max_lon - projection_settings.min_lon);
  }

  double height_zoom_coef = -1;
  if (projection_settings.max_lat != projection_settings.min_lat) {
    height_zoom_coef = (render_settings.height - 2 * render_settings.padding) /
                       (projection_settings.max_lat - projection_settings.min_lat);
  }

  if ((width_zoom_coef == -1) ^ (height_zoom_coef == -1)) {
    projection_settings.zoom_coef = max(height_zoom_coef, width_zoom_coef);
  } else if ((width_zoom_coef != -1) && (height_zoom_coef != -1)) {
    projection_settings.zoom_coef = min(height_zoom_coef, width_zoom_coef);
  }

  return projection_settings;
}

Svg::Point TransportDrawer::ConvertSpherePointToSvgPoint(Sphere::Point sphere_point) const {
  return {
      .x = (sphere_point.longitude - projection_settings_.min_lon) * projection_settings_.zoom_coef +
           render_settings_.padding,
      .y = (projection_settings_.max_lat - sphere_point.latitude) * projection_settings_.zoom_coef +
           render_settings_.padding,
  };
}

TransportDrawer::Map TransportDrawer::Draw() const {
  Map map;

  return map;
}