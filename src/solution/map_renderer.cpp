#include "map_renderer.h"

#include <algorithm>
#include <iterator>

#include "sphere.h"
#include "svg_serialize.h"
#include "map_renderer_helpers.h"

using namespace std;

MapRenderer::MapRenderer(const Descriptions::StopsDict& stops_dict, const Descriptions::BusesDict& buses_dict,
                         const YellowPages::Database& yellow_pages, const Json::Dict& render_settings_json)
    : render_settings_(RenderSettings::Parse(render_settings_json)),
      bus_colors_(ChooseBusColors(buses_dict, render_settings_)),
      buses_dict_(CopyBusesDict(buses_dict)) {
  CoordsMapping mapping = ComputeStopsCoordsByGrid(stops_dict, buses_dict, yellow_pages, render_settings_);
  stops_coords_ = move(mapping.stops);
  companies_coords_ = move(mapping.companies);
}

void MapRenderer::Serialize(TCProto::MapRenderer& proto) {
  render_settings_.Serialize(*proto.mutable_render_settings());

  for (const auto& [name, point] : stops_coords_) {
    auto& stop_coords_proto = *proto.add_stops_coords();
    stop_coords_proto.set_name(name);
    Svg::SerializePoint(point, *stop_coords_proto.mutable_point());
  }

  for (const auto& [name, point] : companies_coords_) {
    auto& companies_coords_proto = *proto.add_companies_coords();
    companies_coords_proto.set_name(name);
    Svg::SerializePoint(point, *companies_coords_proto.mutable_point());
  }

  for (const auto& [name, color] : bus_colors_) {
    auto& bus_color_proto = *proto.add_bus_colors();
    bus_color_proto.set_name(name);
    Svg::SerializeColor(color, *bus_color_proto.mutable_color());
  }

  for (const auto& [_, bus] : buses_dict_) {
    bus.Serialize(*proto.add_bus_descriptions());
  }
}

std::unique_ptr<MapRenderer> MapRenderer::Deserialize(const TCProto::MapRenderer& proto) {
  std::unique_ptr<MapRenderer> renderer_holder(new MapRenderer);
  auto& renderer = *renderer_holder;

  renderer.render_settings_ = RenderSettings::Deserialize(proto.render_settings());

  for (const auto& stop_coords_proto : proto.stops_coords()) {
    renderer.stops_coords_.emplace(stop_coords_proto.name(), Svg::DeserializePoint(stop_coords_proto.point()));
  }

  for (const auto& companies_coords_proto : proto.companies_coords()) {
    renderer.companies_coords_.emplace(companies_coords_proto.name(), Svg::DeserializePoint(companies_coords_proto.point()));
  }

  for (const auto& bus_color_proto : proto.bus_colors()) {
    renderer.bus_colors_.emplace(bus_color_proto.name(), Svg::DeserializeColor(bus_color_proto.color()));
  }

  for (const auto& bus_proto : proto.bus_descriptions()) {
    renderer.buses_dict_.emplace(bus_proto.name(), Descriptions::Bus::Deserialize(bus_proto));
  }

  return renderer_holder;
}

using RouteBusItem = TransportRouter::RouteInfo::RideBusItem;
using RouteWaitItem = TransportRouter::RouteInfo::WaitBusItem;
using WalkToCompanyItem = TransportRouter::RouteInfo::WalkToCompanyItem;

void MapRenderer::RenderBusLines(Svg::Document& svg) const {
  for (const auto& [bus_name, bus] : buses_dict_) {
    const auto& stops = bus.stops;
    if (stops.empty()) {
      continue;
    }
    Svg::Polyline line;
    line.SetStrokeColor(bus_colors_.at(bus_name))
        .SetStrokeWidth(render_settings_.line_width)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round");
    for (const auto& stop_name : stops) {
      line.AddPoint(stops_coords_.at(stop_name));
    }
    svg.Add(line);
  }
}

void MapRenderer::RenderRouteBusLines(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  for (const auto& item : route.items) {
    if (!holds_alternative<RouteBusItem>(item)) {
      continue;
    }
    const auto& bus_item = get<RouteBusItem>(item);
    const string& bus_name = bus_item.bus_name;
    const auto& stops = buses_dict_.at(bus_name).stops;
    if (stops.empty()) {
      continue;
    }
    Svg::Polyline line;
    line.SetStrokeColor(bus_colors_.at(bus_name))
        .SetStrokeWidth(render_settings_.line_width)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round");
    for (size_t stop_idx = bus_item.start_stop_idx; stop_idx <= bus_item.finish_stop_idx; ++stop_idx) {
      const string& stop_name = stops[stop_idx];
      line.AddPoint(stops_coords_.at(stop_name));
    }
    svg.Add(line);
  }
}

void MapRenderer::RenderBusLabel(Svg::Document& svg, const string& bus_name, const string& stop_name) const {
  const auto& color = bus_colors_.at(bus_name);  // can be optimized a bit by moving upper
  const auto point = stops_coords_.at(stop_name);
  const auto base_text = Svg::Text{}
                             .SetPoint(point)
                             .SetOffset(render_settings_.bus_label_offset)
                             .SetFontSize(render_settings_.bus_label_font_size)
                             .SetFontFamily("Verdana")
                             .SetFontWeight("bold")
                             .SetData(bus_name);
  svg.Add(Svg::Text(base_text)
              .SetFillColor(render_settings_.underlayer_color)
              .SetStrokeColor(render_settings_.underlayer_color)
              .SetStrokeWidth(render_settings_.underlayer_width)
              .SetStrokeLineCap("round")
              .SetStrokeLineJoin("round"));
  svg.Add(Svg::Text(base_text).SetFillColor(color));
}

void MapRenderer::RenderBusLabels(Svg::Document& svg) const {
  for (const auto& [bus_name, bus] : buses_dict_) {
    const auto& stops = bus.stops;
    if (!stops.empty()) {
      for (const string& endpoint : bus.endpoints) {
        RenderBusLabel(svg, bus_name, endpoint);
      }
    }
  }
}

void MapRenderer::RenderRouteBusLabels(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  for (const auto& item : route.items) {
    // TODO: remove copypaste with bus lines rendering
    if (!holds_alternative<RouteBusItem>(item)) {
      continue;
    }
    const auto& bus_item = get<RouteBusItem>(item);
    const string& bus_name = bus_item.bus_name;
    const auto& bus = buses_dict_.at(bus_name);
    const auto& stops = bus.stops;
    if (stops.empty()) {
      continue;
    }
    for (const size_t stop_idx : {bus_item.start_stop_idx, bus_item.finish_stop_idx}) {
      const auto stop_name = stops[stop_idx];
      if (stop_idx == 0 || stop_idx == stops.size() - 1 ||
          find(begin(bus.endpoints), end(bus.endpoints), stop_name) != end(bus.endpoints)) {
        RenderBusLabel(svg, bus_name, stop_name);
      }
    }
  }
}

void MapRenderer::RenderStopPoint(Svg::Document& svg, Svg::Point point) const {
  svg.Add(Svg::Circle{}.SetCenter(point).SetRadius(render_settings_.stop_radius).SetFillColor("white"));
}

void MapRenderer::RenderStopPoints(Svg::Document& svg) const {
  for (const auto& [_, stop_point] : stops_coords_) {
    RenderStopPoint(svg, stop_point);
  }
}

void MapRenderer::RenderRouteStopPoints(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  for (const auto& item : route.items) {
    // TODO: remove copypaste with bus lines rendering
    if (!holds_alternative<RouteBusItem>(item)) {
      continue;
    }
    const auto& bus_item = get<RouteBusItem>(item);
    const string& bus_name = bus_item.bus_name;
    const auto& stops = buses_dict_.at(bus_name).stops;
    if (stops.empty()) {
      continue;
    }
    for (size_t stop_idx = bus_item.start_stop_idx; stop_idx <= bus_item.finish_stop_idx; ++stop_idx) {
      const string& stop_name = stops[stop_idx];
      RenderStopPoint(svg, stops_coords_.at(stop_name));
    }
  }
}

void MapRenderer::RenderStopLabel(Svg::Document& svg, Svg::Point point, const string& name) const {
  auto base_text = Svg::Text{}
                       .SetPoint(point)
                       .SetOffset(render_settings_.stop_label_offset)
                       .SetFontSize(render_settings_.stop_label_font_size)
                       .SetFontFamily("Verdana")
                       .SetData(name);
  svg.Add(Svg::Text(base_text)
              .SetFillColor(render_settings_.underlayer_color)
              .SetStrokeColor(render_settings_.underlayer_color)
              .SetStrokeWidth(render_settings_.underlayer_width)
              .SetStrokeLineCap("round")
              .SetStrokeLineJoin("round"));
  svg.Add(base_text.SetFillColor("black"));
}

void MapRenderer::RenderStopLabels(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : stops_coords_) {
    RenderStopLabel(svg, stop_point, stop_name);
  }
}

void MapRenderer::RenderRouteStopLabels(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  if (route.items.empty()) {
    return;
  }
  for (const auto& item : route.items) {
    if (!holds_alternative<RouteWaitItem>(item)) {
      continue;
    }
    const auto& wait_item = get<RouteWaitItem>(item);
    const string& stop_name = wait_item.stop_name;
    RenderStopLabel(svg, stops_coords_.at(stop_name), stop_name);
  }

  // draw stop label for last stop
  if (holds_alternative<RouteBusItem>(route.items.back())) {
    const auto& last_bus_item = get<RouteBusItem>(route.items.back());
    const string& last_stop_name = buses_dict_.at(last_bus_item.bus_name).stops[last_bus_item.finish_stop_idx];
    RenderStopLabel(svg, stops_coords_.at(last_stop_name), last_stop_name);
  } else {
    const auto& company_item = get<WalkToCompanyItem>(route.items.back());
    const string& last_stop_name = company_item.stop_name;
    RenderStopLabel(svg, stops_coords_.at(last_stop_name), last_stop_name);
  }
}

void MapRenderer::RenderRouteCompanyLines(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  if (!holds_alternative<WalkToCompanyItem>(route.items.back())) {
    return;
  }

  Svg::Polyline line;
  line.SetStrokeColor("black")
      .SetStrokeWidth(render_settings_.company_line_width)
      .SetStrokeLineCap("round")
      .SetStrokeLineJoin("round");
  const auto& walk = get<WalkToCompanyItem>(route.items.back());
  line.AddPoint(stops_coords_.at(walk.stop_name));
  line.AddPoint(companies_coords_.at("company__" + walk.company->id()));
  svg.Add(line); 
}

void MapRenderer::RenderRouteCompanyPoints(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  if (!holds_alternative<WalkToCompanyItem>(route.items.back())) {
    return;
  }
  const auto& walk = get<WalkToCompanyItem>(route.items.back());

  svg.Add(Svg::Circle{}
              .SetCenter(companies_coords_.at("company__" + walk.company->id()))
              .SetRadius(render_settings_.company_radius)
              .SetFillColor("black"));
}

void MapRenderer::RenderRouteCompanyLabels(Svg::Document& svg, const TransportRouter::RouteInfo& route) const {
  if (!holds_alternative<WalkToCompanyItem>(route.items.back())) {
    return;
  }
  const auto& walk = get<WalkToCompanyItem>(route.items.back());

  RenderStopLabel(svg, companies_coords_.at("company__" + walk.company->id()), walk.company->cached_full_name());
}

void MapRenderer::RenderDummy(Svg::Document&) const { }

const unordered_map<string, void (MapRenderer::*)(Svg::Document&) const> MapRenderer::MAP_LAYER_ACTIONS = {
    {"bus_lines", &MapRenderer::RenderBusLines},
    {"bus_labels", &MapRenderer::RenderBusLabels},
    {"stop_points", &MapRenderer::RenderStopPoints},
    {"stop_labels", &MapRenderer::RenderStopLabels},
    {"company_lines", &MapRenderer::RenderDummy},
    {"company_points", &MapRenderer::RenderDummy},
    {"company_labels", &MapRenderer::RenderDummy},
};

const unordered_map<string, void (MapRenderer::*)(Svg::Document&, const TransportRouter::RouteInfo&) const>
    MapRenderer::ROUTE_LAYER_ACTIONS = {
        {"bus_lines", &MapRenderer::RenderRouteBusLines},
        {"bus_labels", &MapRenderer::RenderRouteBusLabels},
        {"stop_points", &MapRenderer::RenderRouteStopPoints},
        {"stop_labels", &MapRenderer::RenderRouteStopLabels},
        {"company_lines", &MapRenderer::RenderRouteCompanyLines},
        {"company_points", &MapRenderer::RenderRouteCompanyPoints},
        {"company_labels", &MapRenderer::RenderRouteCompanyLabels},

};

Svg::Document MapRenderer::Render() const {
  Svg::Document svg;

  for (const auto& layer : render_settings_.layers) {
    (this->*MAP_LAYER_ACTIONS.at(layer))(svg);
  }

  return svg;
}

Svg::Document MapRenderer::RenderRoute(Svg::Document svg, const TransportRouter::RouteInfo& route) const {
  const double outer_margin = render_settings_.outer_margin;
  svg.Add(Svg::Rectangle{}
              .SetFillColor(render_settings_.underlayer_color)
              .SetTopLeftPoint({-outer_margin, -outer_margin})
              .SetBottomRightPoint(
                  {render_settings_.max_width + outer_margin, render_settings_.max_height + outer_margin}));

  for (const auto& layer : render_settings_.layers) {
    (this->*ROUTE_LAYER_ACTIONS.at(layer))(svg, route);
  }

  return svg;
}