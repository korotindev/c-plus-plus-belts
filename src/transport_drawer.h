#pragma once

#include <algorithm>
#include <memory>
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

  std::shared_ptr<const std::string> Draw() const;

 private:
  std::shared_ptr<const std::string> svg_map;
  void DrawBusRoute(size_t id, const Descriptions::Bus *bus, const Descriptions::StopsDict &stops_dict,
                    Svg::Document &document) const;
  void DrawStop(const Descriptions::Stop *stop, Svg::Document &document) const;
  void DrawStopName(const Descriptions::Stop *stop, Svg::Document &document) const;
  RenderSettings MakeRenderSettings(const Json::Dict &render_settings_json);

  struct Projection {
    Projection(const RenderSettings &render_settings);
    const RenderSettings &render_settings_;
    double max_lat = 0.0;
    double min_lon = 0.0;
    double zoom_coef = 0.0;
    Svg::Point ConvertSpherePoint(const Sphere::Point &sphere_point) const;
    void CaculateZoom(double min_lat, double max_lon);
  };

  RenderSettings render_settings_;
  Projection projection_;
};
