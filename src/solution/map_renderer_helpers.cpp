#include "map_renderer_helpers.h"
#include "coords_compressor.h"

using namespace std;

struct NeighboursDicts {
  unordered_map<double, vector<double>> neighbour_lats;
  unordered_map<double, vector<double>> neighbour_lons;
};

map<string, Descriptions::Bus> CopyBusesDict(const Descriptions::BusesDict& source) {
  map<string, Descriptions::Bus> target;
  for (const auto& [name, data_ptr] : source) {
    target.emplace(name, *data_ptr);
  }
  return target;
}

static unordered_set<string> FindBusSupportStops(const Descriptions::BusesDict& buses_dict) {
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
    const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
    const YellowPages::Database& yellow_pages) {
  const unordered_set<string> support_stops = FindBusSupportStops(buses_dict);

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
        const double lat_step = (next_coord.latitude - prev_coord.latitude) / (stop_idx - last_support_idx);
        const double lon_step = (next_coord.longitude - prev_coord.longitude) / (stop_idx - last_support_idx);
        for (size_t middle_stop_idx = last_support_idx + 1; middle_stop_idx < stop_idx; ++middle_stop_idx) {
          stops_coords[stops[middle_stop_idx]] = {
              prev_coord.latitude + lat_step * (middle_stop_idx - last_support_idx),
              prev_coord.longitude + lon_step * (middle_stop_idx - last_support_idx),
          };
        }
        stops_coords[stops[stop_idx]] = stops_dict.at(stops[stop_idx])->position;
        last_support_idx = stop_idx;
      }
    }
  }
  
  for(int company_idx = 0; company_idx < yellow_pages.companies_size(); company_idx++) {
    const auto& company = yellow_pages.companies()[company_idx];
    stops_coords["company__" + company.id()] = {
      company.address().coords().lat(),
      company.address().coords().lon()
    };
  }

  return stops_coords;
}

static NeighboursDicts BuildCoordNeighboursDicts(const unordered_map<string, Sphere::Point>& stops_coords,
                                                 const Descriptions::BusesDict& buses_dict,
                                                 const YellowPages::Database& yellow_pages) {
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

  for(int company_idx = 0; company_idx < yellow_pages.companies_size(); company_idx++) {
    const auto& company = yellow_pages.companies()[company_idx];
    const auto point_cur = stops_coords.at("company__" + company.id());

    for(int nearby_stop_idx = 0; nearby_stop_idx < company.nearby_stops_size(); nearby_stop_idx++) {
      Sphere::Point point_prev = stops_coords.at(company.nearby_stops()[nearby_stop_idx].name());
      const auto [min_lat, max_lat] = minmax(point_prev.latitude, point_cur.latitude);
      const auto [min_lon, max_lon] = minmax(point_prev.longitude, point_cur.longitude);
      neighbour_lats[max_lat].push_back(min_lat);
      neighbour_lons[max_lon].push_back(min_lon);
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

CoordsMapping ComputeStopsCoordsByGrid(const Descriptions::StopsDict& stops_dict,
                                                 const Descriptions::BusesDict& buses_dict,
                                                 const YellowPages::Database& yellow_pages,
                                                 const RenderSettings& render_settings) {
  const auto stops_coords = ComputeInterpolatedStopsGeoCoords(stops_dict, buses_dict, yellow_pages);

  const auto [neighbour_lats, neighbour_lons] = BuildCoordNeighboursDicts(stops_coords, buses_dict, yellow_pages);

  CoordsCompressor compressor(stops_coords);
  compressor.FillIndices(neighbour_lats, neighbour_lons);
  compressor.FillTargets(render_settings.max_width, render_settings.max_height, render_settings.padding);

  CoordsMapping mapping;
  for (const auto& [stop_name, coord] : stops_coords) {
    if (stop_name.find("company__") == 0) {
      mapping.companies[stop_name] = {compressor.MapLon(coord.longitude), compressor.MapLat(coord.latitude)};
    } else {
      mapping.stops[stop_name] = {compressor.MapLon(coord.longitude), compressor.MapLat(coord.latitude)};
    }
  }

  return mapping;
}

unordered_map<string, Svg::Color> ChooseBusColors(const Descriptions::BusesDict& buses_dict,
                                                  const RenderSettings& render_settings) {
  const auto& palette = render_settings.palette;
  unordered_map<string, Svg::Color> bus_colors;
  int idx = 0;
  for (const auto& [bus_name, bus_ptr] : buses_dict) {
    bus_colors[bus_name] = palette[idx++ % palette.size()];
  }
  return bus_colors;
}