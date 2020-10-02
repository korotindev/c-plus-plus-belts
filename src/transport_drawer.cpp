#include "transport_drawer.h"

#include <sstream>

using namespace std;

Svg::Color parse_color(const Json::Node &node) {
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

TransportDrawer::TransportDrawer(const Json::Dict &render_settings_json)
    : render_settings_(MakeRenderSettings(render_settings_json)) {}

TransportDrawer::RenderSettings TransportDrawer::MakeRenderSettings(const Json::Dict &json) {
  const auto &palette = json.at("color_palette").AsArray();
  vector<Svg::Color> color_palette;
  transform(palette.cbegin(), palette.cend(), back_inserter(color_palette),
            [](auto &node) { return parse_color(node); });

  RenderSettings render_settings{
      .width = json.at("width").AsDouble(),
      .height = json.at("height").AsDouble(),
      .padding = json.at("padding").AsDouble(),
      .stop_radius = json.at("stop_radius").AsDouble(),
      .line_width = json.at("line_width").AsDouble(),
      .stop_label_font_size = static_cast<uint32_t>(json.at("stop_label_font_size").AsInt()),
      .stop_label_offset = Svg::Point{.x = json.at("stop_label_offset").AsArray()[0].AsDouble(),
                                      .y = json.at("stop_label_offset").AsArray()[1].AsDouble()},
      .underlayer_color = parse_color(json.at("underlayer_color")),
      .underlayer_width = json.at("underlayer_width").AsDouble(),
      .color_palette = move(color_palette)};

  return render_settings;
}

TransportDrawer::Map TransportDrawer::Draw(const TransportCatalog &) const {
  Map map;

  return map;
}