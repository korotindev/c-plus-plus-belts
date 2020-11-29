#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "svg.h"

struct RenderSettings {
  double max_width;
  double max_height;
  double padding;
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
  double outer_margin;
};

struct MapSettings {
  std::map<std::string, Svg::Point> stops_coords_;
  std::unordered_map<std::string, Svg::Color> bus_colors_;
  MapSettings(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
              const RenderSettings& render_settings);
};

class MapRenderingSettings {
 public:
  MapRenderingSettings(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
                       const Json::Dict& render_settings_json);

  Svg::Color GetBusColor(const std::string& bus_name) const;
  Svg::Point GetStopPoint(const std::string& bus_name) const;

  RenderSettings render_settings_;
  MapSettings map_settings_;
};
