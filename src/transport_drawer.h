#pragma once

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <algorithm>

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
  void DrawBusRoute(size_t id, Svg::Document &document) const;
  void DrawStop(size_t id, Svg::Document &document) const;
  void DrawStopName(size_t id, Svg::Document &document) const;

  struct Stop {
    std::string name;
    Svg::Point position;
  };

  struct Bus {
    std::string name;
    std::vector<std::string> stops;
  };

  std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops_;
  std::unordered_map<std::string_view, std::shared_ptr<Bus>> buses_;
  std::vector<std::string_view> sorted_stops_names_;
  std::vector<std::string_view> sorted_buses_names_;

  RenderSettings render_settings_;
  ProjectionSettings projection_settings_;
};
