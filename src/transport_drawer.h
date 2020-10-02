#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "json.h"
#include "svg.h"

class TransportCatalog;

class TransportDrawer {
 public:
  TransportDrawer(const Json::Dict &render_settings_json);

  struct RenderSettings {
    double width;
    double height;
    double padding;
    double stop_radius;
    double line_width;
    uint32_t stop_label_font_size;
    Svg::Point stop_label_offset;
    Svg::Color underlayer_color;
    double underlayer_width;
    std::vector<Svg::Color> color_palette;
  };

  struct Map {
    std::string svg;
  };

  Map Draw(const TransportCatalog &db_) const;

 private:
  static RenderSettings MakeRenderSettings(const Json::Dict &json);

  RenderSettings render_settings_;
};
