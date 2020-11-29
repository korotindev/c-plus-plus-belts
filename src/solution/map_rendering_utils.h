#include <string>

#include "map_rendering_settings.h"
#include "svg.h"

Svg::Polyline InitializeLineForBus(const std::string& bus_name, const MapRenderingSettings& settings);

void RenderStopLabel(Svg::Document& svg, const std::string& data, const Svg::Point& stop_point,
                     const MapRenderingSettings& settings);

void RenderEndpointStopLabel(Svg::Document& svg, const std::string& bus_name, const std::string& stop_name,
                             const MapRenderingSettings& settings);

void RenderStopPoint(Svg::Document& svg, const Svg::Point& stop_point, const MapRenderingSettings& settings);

void RenderBigWhiteSquare(Svg::Document& svg, const MapRenderingSettings& settings);