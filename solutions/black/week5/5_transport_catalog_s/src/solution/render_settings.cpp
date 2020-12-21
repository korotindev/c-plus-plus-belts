#include "render_settings.h"

#include "svg_serialize.h"

using namespace std;

static vector<Svg::Color> ParseColors(const Json::Node& json) {
  const auto& array = json.AsArray();
  vector<Svg::Color> colors;
  colors.reserve(array.size());
  transform(begin(array), end(array), back_inserter(colors), Svg::ParseColor);
  return colors;
}

void RenderSettings::Serialize(TCProto::RenderSettings& proto) const {
  proto.set_max_width(max_width);
  proto.set_max_height(max_height);
  proto.set_padding(padding);
  proto.set_outer_margin(outer_margin);

  for (const Svg::Color& color : palette) {
    Svg::SerializeColor(color, *proto.add_palette());
  }

  proto.set_line_width(line_width);
  Svg::SerializeColor(underlayer_color, *proto.mutable_underlayer_color());
  proto.set_underlayer_width(underlayer_width);
  proto.set_stop_radius(stop_radius);
  Svg::SerializePoint(bus_label_offset, *proto.mutable_bus_label_offset());
  proto.set_bus_label_font_size(bus_label_font_size);
  Svg::SerializePoint(stop_label_offset, *proto.mutable_stop_label_offset());
  proto.set_stop_label_font_size(stop_label_font_size);
  proto.set_company_radius(company_radius);
  proto.set_company_line_width(company_line_width);

  for (const string& layer : layers) {
    proto.add_layers(layer);
  }
}

RenderSettings RenderSettings::Deserialize(const TCProto::RenderSettings& proto) {
  RenderSettings settings;
  settings.max_width = proto.max_width();
  settings.max_height = proto.max_height();
  settings.padding = proto.padding();
  settings.outer_margin = proto.outer_margin();

  settings.palette.reserve(proto.palette_size());
  for (const auto& color : proto.palette()) {
    settings.palette.push_back(Svg::DeserializeColor(color));
  }

  settings.line_width = proto.line_width();
  settings.underlayer_color = Svg::DeserializeColor(proto.underlayer_color());
  settings.underlayer_width = proto.underlayer_width();
  settings.stop_radius = proto.stop_radius();
  settings.bus_label_offset = Svg::DeserializePoint(proto.bus_label_offset());
  settings.bus_label_font_size = proto.bus_label_font_size();
  settings.stop_label_offset = Svg::DeserializePoint(proto.stop_label_offset());
  settings.stop_label_font_size = proto.stop_label_font_size();
  settings.company_radius = proto.company_radius();
  settings.company_line_width = proto.company_line_width();

  settings.layers.reserve(proto.layers_size());
  for (const auto& layer : proto.layers()) {
    settings.layers.push_back(layer);
  }

  return settings;
}

RenderSettings RenderSettings::Parse(const Json::Dict& json) {
  RenderSettings result;
  result.max_width = json.at("width").AsDouble();
  result.max_height = json.at("height").AsDouble();
  result.padding = json.at("padding").AsDouble();
  result.outer_margin = json.at("outer_margin").AsDouble();
  result.palette = ParseColors(json.at("color_palette"));
  result.line_width = json.at("line_width").AsDouble();
  result.underlayer_color = Svg::ParseColor(json.at("underlayer_color"));
  result.underlayer_width = json.at("underlayer_width").AsDouble();
  result.stop_radius = json.at("stop_radius").AsDouble();
  result.bus_label_offset = Svg::ParsePoint(json.at("bus_label_offset"));
  result.bus_label_font_size = json.at("bus_label_font_size").AsInt();
  result.stop_label_offset = Svg::ParsePoint(json.at("stop_label_offset"));
  result.stop_label_font_size = json.at("stop_label_font_size").AsInt();
  result.company_radius = json.at("company_radius").AsDouble();
  result.company_line_width = json.at("company_line_width").AsDouble();

  const auto& layers_array = json.at("layers").AsArray();
  result.layers.reserve(layers_array.size());
  for (const auto& layer_node : layers_array) {
    result.layers.push_back(layer_node.AsString());
  }

  return result;
}