#pragma once

#include "json.h"
#include "render_settings.pb.h"
#include "svg.h"

struct RenderSettings {
  double max_width;
  double max_height;
  double padding;
  double outer_margin;
  std::vector<Svg::Color> palette;
  double line_width;
  Svg::Color underlayer_color;
  double underlayer_width;
  double stop_radius;
  Svg::Point bus_label_offset;
  int bus_label_font_size;
  Svg::Point stop_label_offset;
  int stop_label_font_size;
  std::vector<std::string> layers;
  double company_radius;
  double company_line_width;

  void Serialize(TCProto::RenderSettings& proto) const;
  static RenderSettings Deserialize(const TCProto::RenderSettings& proto);
  static RenderSettings Parse(const Json::Dict& json);
};
