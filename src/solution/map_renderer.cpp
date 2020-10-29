#include "map_renderer.h"

#include <cassert>

#include "map_stops_distributor.h"
#include "sphere.h"
#include "sphere_projection.h"
#include "utils.h"

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

  const auto& layers_array = json.at("layers").AsArray();
  result.layers.reserve(layers_array.size());
  for (const auto& layer_node : layers_array) {
    result.layers.push_back(layer_node.AsString());
  }

  return result;
}

static map<string, Svg::Point> ComputeStopsCoords(shared_ptr<const TransportInfo> transport_info,
                                                  const RenderSettings& render_settings) {
  const double max_width = render_settings.max_width;
  const double max_height = render_settings.max_height;
  const double padding = render_settings.padding;

  MapStopsDistributor distributor(transport_info);

  vector<Sphere::Projector::PointObject> point_objects;
  point_objects.reserve(transport_info->StopsCount());
  for (const auto stop_ptr : transport_info->GetStopsRange()) {
    // point_objects.push_back({stop_ptr->name, stop_ptr->position});
    point_objects.push_back({stop_ptr->name, distributor(stop_ptr->id)});
  }

  auto stops_collider = [&transport_info](const Sphere::Projector::PointObject& stop_po,
                                          const vector<const Sphere::Projector::PointObject*>& group) {
    const auto stop_ptr = transport_info->GetStop(stop_po.id);
    const auto& bus_names = stop_ptr->bus_names;

    for (const auto other_stop_po : group) {
      const auto other_stop_ptr = transport_info->GetStop(other_stop_po->id);

      bool has_short_path_forward = stop_ptr->distances.count(other_stop_ptr->name) > 0;
      bool has_short_path_backward = other_stop_ptr->distances.count(stop_ptr->name) > 0;

      if (has_short_path_forward || has_short_path_backward) {
        const auto& other_stop_buses = transport_info->GetStop(other_stop_ptr->name)->bus_names;

        for (const auto& bus_name : bus_names) {
          if (auto it = other_stop_buses.find(bus_name); it != other_stop_buses.cend()) {
            const auto& stops = transport_info->GetBus(bus_name)->stops;

            for (size_t i = 1; i < stops.size(); ++i) {
              if ((stops[i] == stop_ptr->name && stops[i - 1] == other_stop_ptr->name) ||
                  (stops[i] == other_stop_ptr->name && stops[i - 1] == stop_ptr->name)) {
                return true;
              }
            }
          }
        }
      }
    }
    return false;
  };

  const Sphere::Projector projector(point_objects, stops_collider, max_width, max_height, padding);

  map<string, Svg::Point> stops_coords;
  for (const auto bus_ptr : transport_info->GetStopsRange()) {
    stops_coords[bus_ptr->name] = projector(bus_ptr->name);
  }

  return stops_coords;
}

static unordered_map<string, Svg::Color> ChooseBusColors(shared_ptr<const TransportInfo> transport_info,
                                                         const RenderSettings& render_settings) {
  const auto& palette = render_settings.palette;
  unordered_map<string, Svg::Color> bus_colors;
  for (const auto bus_ptr : transport_info->GetBusesRange()) {
    bus_colors[bus_ptr->name] = palette[bus_ptr->id % palette.size()];
  }
  return bus_colors;
}

void DefaultMapRenderer::Prepare(shared_ptr<const TransportInfo> transport_info, const Json::Dict& render_settings_json) {
  transport_info_ = move(transport_info);
  render_settings_ = ParseRenderSettings(render_settings_json);
  stops_coords_ = ComputeStopsCoords(transport_info_, render_settings_);
  bus_colors_ = ChooseBusColors(transport_info_, render_settings_);
}

void DefaultMapRenderer::RenderBusLines(Svg::Document& svg) const {
  for (const auto bus_ptr : transport_info_->GetBusesRange()) {
    const auto& stops = bus_ptr->stops;
    if (stops.empty()) {
      continue;
    }
    Svg::Polyline line;
    line.SetStrokeColor(bus_colors_.at(bus_ptr->name))
        .SetStrokeWidth(render_settings_.line_width)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round");
    for (const auto& stop_name : stops) {
      line.AddPoint(stops_coords_.at(stop_name));
    }
    svg.Add(line);
  }
}

void DefaultMapRenderer::RenderBusLabels(Svg::Document& svg) const {
  for (const auto bus_ptr : transport_info_->GetBusesRange()) {
    const auto& stops = bus_ptr->stops;
    if (!stops.empty()) {
      const auto& color = bus_colors_.at(bus_ptr->name);
      for (const string& endpoint : bus_ptr->endpoints) {
        const auto point = stops_coords_.at(endpoint);
        const auto base_text = Svg::Text{}
                                   .SetPoint(point)
                                   .SetOffset(render_settings_.bus_label_offset)
                                   .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size))
                                   .SetFontFamily("Verdana")
                                   .SetFontWeight("bold")
                                   .SetData(bus_ptr->name);
        svg.Add(Svg::Text(base_text)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round"));
        svg.Add(Svg::Text(base_text).SetFillColor(color));
      }
    }
  }
}

void DefaultMapRenderer::RenderStopPoints(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : stops_coords_) {
    svg.Add(Svg::Circle{}.SetCenter(stop_point).SetRadius(render_settings_.stop_radius).SetFillColor("white"));
  }
}

void DefaultMapRenderer::RenderStopLabels(Svg::Document& svg) const {
  for (const auto& [stop_name, stop_point] : stops_coords_) {
    const auto base_text = Svg::Text{}
                               .SetPoint(stop_point)
                               .SetOffset(render_settings_.stop_label_offset)
                               .SetFontSize(static_cast<uint32_t>(render_settings_.stop_label_font_size))
                               .SetFontFamily("Verdana")
                               .SetData(stop_name);
    svg.Add(Svg::Text(base_text)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap("round")
                .SetStrokeLineJoin("round"));
    svg.Add(Svg::Text(base_text).SetFillColor("black"));
  }
}

const unordered_map<string, void (DefaultMapRenderer::*)(Svg::Document&) const> DefaultMapRenderer::LAYER_ACTIONS = {
    {"bus_lines", &DefaultMapRenderer::RenderBusLines},
    {"bus_labels", &DefaultMapRenderer::RenderBusLabels},
    {"stop_points", &DefaultMapRenderer::RenderStopPoints},
    {"stop_labels", &DefaultMapRenderer::RenderStopLabels},
};

Svg::Document DefaultMapRenderer::Render() const {
  Svg::Document svg;

  for (const auto& layer : render_settings_.layers) {
    (this->*LAYER_ACTIONS.at(layer))(svg);
  }

  return svg;
}
