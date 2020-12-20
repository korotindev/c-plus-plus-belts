#pragma once

#include <map>
#include <string>

#include "descriptions.h"
#include "render_settings.h"
#include "svg.h"

struct CoordsMapping {
  std::map<std::string, Svg::Point> stops;
  std::unordered_map<std::string, Svg::Point> companies;
};

std::map<std::string, Descriptions::Bus> CopyBusesDict(const Descriptions::BusesDict& source);

CoordsMapping ComputeStopsCoordsByGrid(const Descriptions::StopsDict& stops_dict,
                                       const Descriptions::BusesDict& buses_dict,
                                       const YellowPages::Database& yellow_pages,
                                       const RenderSettings& render_settings);

std::unordered_map<std::string, Svg::Color> ChooseBusColors(const Descriptions::BusesDict& buses_dict,
                                                            const RenderSettings& render_settings);