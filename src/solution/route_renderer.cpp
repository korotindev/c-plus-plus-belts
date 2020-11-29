#include "route_renderer.h"

#include <algorithm>
#include <cassert>

#include "map_renderer.h"
#include "map_rendering_utils.h"
#include "sphere.h"

using namespace std;

RouteRenderer::RouteRenderer(const MapRenderingSettings& settings, const Descriptions::BusesDict& buses_dict,
                             const TransportRouter::RouteInfo& route_info, const string& stop_to)
    : settings_(settings), buses_dict_(buses_dict), route_info_(route_info), stop_to_(stop_to) {}

void RouteRenderer::RenderRouteLine(const std::function<RouteRenderer::BusProcessor>& processor) const {
  for (size_t i = 1; i < route_info_.items.size(); i += 2) {
    std::string start_stop_name = std::get<TransportRouter::RouteInfo::WaitItem>(route_info_.items[i - 1]).stop_name;
    std::string finish_stop_name =
        i + 1 < route_info_.items.size()
            ? std::get<TransportRouter::RouteInfo::WaitItem>(route_info_.items[i + 1]).stop_name
            : stop_to_;
    auto& bus_item = std::get<TransportRouter::RouteInfo::BusItem>(route_info_.items[i]);
    auto& stops = buses_dict_.at(bus_item.bus_name)->stops;
    size_t span_count = bus_item.span_count;
    size_t idx = 0;
    for (size_t i = 0; i + span_count < stops.size(); i++) {
      if (stops[i] == start_stop_name && stops[i + span_count] == finish_stop_name) {
        idx = i;
        break;
      }
    }

    processor(bus_item, idx, idx + span_count);
  }
}

void RouteRenderer::RenderBusLines(Svg::Document& svg) const {
  RenderRouteLine([this, &svg](const TransportRouter::RouteInfo::BusItem& bus, size_t from, size_t to) {
    const auto& stops = buses_dict_.at(bus.bus_name)->stops;
    auto line = InitializeLineForBus(bus.bus_name, settings_);
    for (; from <= to; from++) {
      line.AddPoint(settings_.GetStopPoint(stops[from]));
    }
    svg.Add(line);
  });
}

void RouteRenderer::RenderBusLabels(Svg::Document& svg) const {
  for (const auto& [bus_name, bus_ptr] : buses_dict_) {
    const auto& stops = bus_ptr->stops;
    if (!stops.empty()) {
      for (const string& stop_name : bus_ptr->endpoints) {
        RenderEndpointStopLabel(svg, bus_name, stop_name, settings_);
      }
    }
  }
}

void RouteRenderer::RenderStopPoints(Svg::Document& svg) const {
  RenderRouteLine([this, &svg](const TransportRouter::RouteInfo::BusItem& bus, size_t from, size_t to) {
    const auto& stops = buses_dict_.at(bus.bus_name)->stops;
    for (; from <= to; from++) {
      RenderStopPoint(svg, settings_.GetStopPoint(stops[from]), settings_);
    }
  });
}

void RouteRenderer::RenderStopLabels(Svg::Document& svg) const {
  optional<string> last_stop_name;
  RenderRouteLine(
      [this, &svg, &last_stop_name](const TransportRouter::RouteInfo::BusItem& bus, size_t from, size_t to) {
        const auto& stops = buses_dict_.at(bus.bus_name)->stops;
        RenderStopLabel(svg, stops[from], settings_.GetStopPoint(stops[from]), settings_);
        last_stop_name = stops[to];
      });

  if (last_stop_name) {
    RenderStopLabel(svg, *last_stop_name, settings_.GetStopPoint(*last_stop_name), settings_);
  }
}

const unordered_map<string, void (RouteRenderer::*)(Svg::Document&) const> RouteRenderer::LAYER_ACTIONS = {
    {"bus_lines", &RouteRenderer::RenderBusLines},
    {"bus_labels", &RouteRenderer::RenderBusLabels},
    {"stop_points", &RouteRenderer::RenderStopPoints},
    {"stop_labels", &RouteRenderer::RenderStopLabels},
};

Svg::Document RouteRenderer::Render() const {
  Svg::Document svg = MapRenderer(settings_, buses_dict_).Render();

  RenderBigWhiteSquare(svg, settings_);

  for (const auto& layer : settings_.render_settings_.layers) {
    (this->*LAYER_ACTIONS.at(layer))(svg);
  }

  return svg;
}