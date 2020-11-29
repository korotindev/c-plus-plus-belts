#include <algorithm>
#include <cassert>

#include "map_renderer.h"
#include "sphere.h"

using namespace std;

static Svg::Point ParsePoint(const Json::Node& json) {
  const auto& array = json.AsArray();
  return {array[0].AsDouble(), array[1].AsDouble()};
}

static Svg::Color ParseColor(const Json::Node& json) {
  if (json.IsString()) {
    return json.AsString();
  }
  const auto& array = json.AsArray();
  assert(array.size() == 3 || array.size() == 4);
  Svg::Rgb rgb{static_cast<uint8_t>(array[0].AsInt()), static_cast<uint8_t>(array[1].AsInt()),
               static_cast<uint8_t>(array[2].AsInt())};
  if (array.size() == 3) {
    return rgb;
  } else {
    return Svg::Rgba{rgb, array[3].AsDouble()};
  }
}

static vector<Svg::Color> ParseColors(const Json::Node& json) {
  const auto& array = json.AsArray();
  vector<Svg::Color> colors;
  colors.reserve(array.size());
  transform(begin(array), end(array), back_inserter(colors), ParseColor);
  return colors;
}

RenderSettings ParseRenderSettings(const Json::Dict& json) {
  RenderSettings result;
  result.max_width = json.at("width").AsDouble();
  result.max_height = json.at("height").AsDouble();
  result.padding = json.at("padding").AsDouble();
  result.palette = ParseColors(json.at("color_palette"));
  result.line_width = json.at("line_width").AsDouble();
  result.underlayer_color = ParseColor(json.at("underlayer_color"));
  result.underlayer_width = json.at("underlayer_width").AsDouble();
  result.stop_radius = json.at("stop_radius").AsDouble();
  result.bus_label_offset = ParsePoint(json.at("bus_label_offset"));
  result.bus_label_font_size = json.at("bus_label_font_size").AsInt();
  result.stop_label_offset = ParsePoint(json.at("stop_label_offset"));
  result.stop_label_font_size = json.at("stop_label_font_size").AsInt();
  result.outer_margin = json.at("outer_margin").AsDouble();

  const auto& layers_array = json.at("layers").AsArray();
  result.layers.reserve(layers_array.size());
  for (const auto& layer_node : layers_array) {
    result.layers.push_back(layer_node.AsString());
  }

  return result;
}

static unordered_set<string> FindSupportStops(const Descriptions::BusesDict& buses_dict) {
  unordered_set<string> support_stops;
  unordered_map<string, const Descriptions::Bus*> stops_first_bus;
  unordered_map<string, int> stops_rank;
  for (const auto& [_, bus_ptr] : buses_dict) {
    for (const string& stop : bus_ptr->endpoints) {
      support_stops.insert(stop);
    }
    for (const string& stop : bus_ptr->stops) {
      ++stops_rank[stop];
      const auto [it, inserted] = stops_first_bus.emplace(stop, bus_ptr);
      if (!inserted && it->second != bus_ptr) {
        support_stops.insert(stop);
      }
    }
  }

  for (const auto& [stop, rank] : stops_rank) {
    if (rank > 2) {
      support_stops.insert(stop);
    }
  }

  return support_stops;
}

static unordered_map<string, Sphere::Point> ComputeInterpolatedStopsGeoCoords(
    const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict) {
  const unordered_set<string> support_stops = FindSupportStops(buses_dict);

  unordered_map<string, Sphere::Point> stops_coords;
  for (const auto& [stop_name, stop_ptr] : stops_dict) {
    stops_coords[stop_name] = stop_ptr->position;
  }

  for (const auto& [_, bus_ptr] : buses_dict) {
    const auto& stops = bus_ptr->stops;
    if (stops.empty()) {
      continue;
    }
    size_t last_support_idx = 0;
    stops_coords[stops[0]] = stops_dict.at(stops[0])->position;
    for (size_t stop_idx = 1; stop_idx < stops.size(); ++stop_idx) {
      if (support_stops.count(stops[stop_idx])) {
        const Sphere::Point prev_coord = stops_dict.at(stops[last_support_idx])->position;
        const Sphere::Point next_coord = stops_dict.at(stops[stop_idx])->position;
        const double lat_step =
            (next_coord.latitude - prev_coord.latitude) / static_cast<double>(stop_idx - last_support_idx);
        const double lon_step =
            (next_coord.longitude - prev_coord.longitude) / static_cast<double>(stop_idx - last_support_idx);
        for (size_t middle_stop_idx = last_support_idx + 1; middle_stop_idx < stop_idx; ++middle_stop_idx) {
          stops_coords[stops[middle_stop_idx]] = {
              prev_coord.latitude + lat_step * static_cast<double>(middle_stop_idx - last_support_idx),
              prev_coord.longitude + lon_step * static_cast<double>(middle_stop_idx - last_support_idx),
          };
        }
        stops_coords[stops[stop_idx]] = stops_dict.at(stops[stop_idx])->position;
        last_support_idx = stop_idx;
      }
    }
  }

  return stops_coords;
}

struct NeighboursDicts {
  unordered_map<double, vector<double>> neighbour_lats;
  unordered_map<double, vector<double>> neighbour_lons;
};

static NeighboursDicts BuildCoordNeighboursDicts(const unordered_map<string, Sphere::Point>& stops_coords,
                                                 const Descriptions::BusesDict& buses_dict) {
  unordered_map<double, vector<double>> neighbour_lats;
  unordered_map<double, vector<double>> neighbour_lons;
  for (const auto& [bus_name, bus_ptr] : buses_dict) {
    const auto& stops = bus_ptr->stops;
    if (stops.empty()) {
      continue;
    }
    Sphere::Point point_prev = stops_coords.at(stops[0]);
    for (size_t stop_idx = 1; stop_idx < stops.size(); ++stop_idx) {
      const auto point_cur = stops_coords.at(stops[stop_idx]);
      if (stops[stop_idx] != stops[stop_idx - 1]) {
        const auto [min_lat, max_lat] = minmax(point_prev.latitude, point_cur.latitude);
        const auto [min_lon, max_lon] = minmax(point_prev.longitude, point_cur.longitude);
        neighbour_lats[max_lat].push_back(min_lat);
        neighbour_lons[max_lon].push_back(min_lon);
      }
      point_prev = point_cur;
    }
  }

  for (auto* neighbours_dict : {&neighbour_lats, &neighbour_lons}) {
    for (auto& [_, values] : *neighbours_dict) {
      sort(begin(values), end(values));
      values.erase(unique(begin(values), end(values)), end(values));
    }
  }

  return {move(neighbour_lats), move(neighbour_lons)};
}

class CoordsCompressor {
 public:
  CoordsCompressor(const unordered_map<string, Sphere::Point>& stops_coords) {
    for (const auto& [_, coord] : stops_coords) {
      lats_.push_back({coord.latitude});
      lons_.push_back({coord.longitude});
    }

    sort(begin(lats_), end(lats_));
    sort(begin(lons_), end(lons_));
  }

  void FillIndices(const unordered_map<double, vector<double>>& neighbour_lats,
                   const unordered_map<double, vector<double>>& neighbour_lons) {
    FillCoordIndices(lats_, neighbour_lats);
    FillCoordIndices(lons_, neighbour_lons);
  }

  void FillTargets(double max_width, double max_height, double padding) {
    if (lats_.empty() || lons_.empty()) {
      return;
    }

    const size_t max_lat_idx = FindMaxLatIdx();
    const double y_step = max_lat_idx ? (max_height - 2 * padding) / static_cast<double>(max_lat_idx) : 0;

    const size_t max_lon_idx = FindMaxLonIdx();
    const double x_step = max_lon_idx ? (max_width - 2 * padding) / static_cast<double>(max_lon_idx) : 0;

    for (auto& [_, idx, target] : lats_) {
      target = max_height - padding - static_cast<double>(idx) * y_step;
    }
    for (auto& [_, idx, target] : lons_) {
      target = static_cast<double>(idx) * x_step + padding;
    }
  }

  double MapLat(double source) const { return Find(lats_, source).target; }
  double MapLon(double source) const { return Find(lons_, source).target; }

 private:
  struct CoordInfo {
    double source;
    size_t idx = 0;
    double target = 0;

    bool operator<(const CoordInfo& other) const { return source < other.source; }
  };

  vector<CoordInfo> lats_;
  vector<CoordInfo> lons_;

  void FillCoordIndices(vector<CoordInfo>& coords, const unordered_map<double, vector<double>>& neighbour_values) {
    for (auto coord_it = begin(coords); coord_it != end(coords); ++coord_it) {
      const auto neighbours_it = neighbour_values.find(coord_it->source);
      if (neighbours_it == neighbour_values.end()) {
        coord_it->idx = 0;
        continue;
      }
      const auto& neighbours = neighbours_it->second;
      optional<size_t> max_neighbour_idx;
      for (const double source : neighbours) {
        const size_t idx = Find(coords, source, coord_it).idx;
        if (idx > max_neighbour_idx) {
          max_neighbour_idx = idx;
        }
      }
      coord_it->idx = *max_neighbour_idx + 1;
    }
  }

  static const CoordInfo& Find(const vector<CoordInfo>& sorted_values, double source,
                               optional<vector<CoordInfo>::const_iterator> end_it = nullopt) {
    return *lower_bound(begin(sorted_values), end_it.value_or(end(sorted_values)), CoordInfo{source});
  }

  static size_t FindMaxIdx(const vector<CoordInfo>& coords) {
    return max_element(begin(coords), end(coords),
                       [](const CoordInfo& lhs, const CoordInfo& rhs) { return lhs.idx < rhs.idx; })
        ->idx;
  }

  size_t FindMaxLatIdx() const { return FindMaxIdx(lats_); }
  size_t FindMaxLonIdx() const { return FindMaxIdx(lons_); }
};

static map<string, Svg::Point> ComputeStopsCoordsByGrid(const Descriptions::StopsDict& stops_dict,
                                                        const Descriptions::BusesDict& buses_dict,
                                                        const RenderSettings& render_settings) {
  const auto stops_coords = ComputeInterpolatedStopsGeoCoords(stops_dict, buses_dict);

  const auto [neighbour_lats, neighbour_lons] = BuildCoordNeighboursDicts(stops_coords, buses_dict);

  CoordsCompressor compressor(stops_coords);
  compressor.FillIndices(neighbour_lats, neighbour_lons);
  compressor.FillTargets(render_settings.max_width, render_settings.max_height, render_settings.padding);

  map<string, Svg::Point> new_stops_coords;
  for (const auto& [stop_name, coord] : stops_coords) {
    new_stops_coords[stop_name] = {compressor.MapLon(coord.longitude), compressor.MapLat(coord.latitude)};
  }

  return new_stops_coords;
}

static unordered_map<string, Svg::Color> ChooseBusColors(const Descriptions::BusesDict& buses_dict,
                                                         const RenderSettings& render_settings) {
  const auto& palette = render_settings.palette;
  unordered_map<string, Svg::Color> bus_colors;
  size_t idx = 0;
  for (const auto& [bus_name, bus_ptr] : buses_dict) {
    bus_colors[bus_name] = palette[idx++ % palette.size()];
  }
  return bus_colors;
}

MapSettings::MapSettings(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
                         const RenderSettings& render_settings)
    : stops_coords_(ComputeStopsCoordsByGrid(stops_dict, buses_dict, render_settings)),
      bus_colors_(ChooseBusColors(buses_dict, render_settings)) {}

MapRenderingSettings::MapRenderingSettings(const Descriptions::StopsDict& stops_dict,
                                           const Descriptions::BusesDict& buses_dict,
                                           const Json::Dict& render_settings_json)
    : render_settings_(ParseRenderSettings(render_settings_json)),
      map_settings_(stops_dict, buses_dict, render_settings_) {}

Svg::Color MapRenderingSettings::GetBusColor(const string& bus_name) const {
  return map_settings_.bus_colors_.at(bus_name);
}

Svg::Point MapRenderingSettings::GetStopPoint(const string& stop_name) const {
  return map_settings_.stops_coords_.at(stop_name);
}
