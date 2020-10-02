#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "svg.h"

class TransportDrawer {
 public:
  TransportDrawer(const Descriptions::StopsDict &stops_dict, const Descriptions::BusesDict &buses_dict,
                  const Json::Dict &render_settings_json);

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

  struct ProjectionSettings {
    double min_lat = 0.0;
    double max_lat = 0.0;
    double min_lon = 0.0;
    double max_lon = 0.0;
    double zoom_coef = 0.0;
  };

  struct Map {
    std::string svg;
  };

  Map Draw() const;

 private:
  static RenderSettings MakeRenderSettings(const Json::Dict &json);
  static ProjectionSettings MakeProjectionSettings(const RenderSettings &render_settings,
                                                   const Descriptions::StopsDict &stops_dict);

  Svg::Point ConvertSpherePointToSvgPoint(Sphere::Point sphere_point) const;

  Descriptions::StopsDict stops_dict_;
  Descriptions::BusesDict buses_dict_;

  RenderSettings render_settings_;
  ProjectionSettings projection_settings_;
};
