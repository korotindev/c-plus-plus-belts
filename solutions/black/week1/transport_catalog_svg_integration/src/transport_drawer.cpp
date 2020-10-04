#include "transport_drawer.h"

#include <sstream>

using namespace std;

Svg::Color ParseColor(const Json::Node &node) {
  variant<Svg::Rgb, Svg::Rgba, string> color;

  if (holds_alternative<string>(node.GetBase())) {
    color = node.AsString();
  } else {
    auto rgb_args = node.AsArray();
    if (rgb_args.size() == 3) {
      color = Svg::Rgb{.red = static_cast<uint8_t>(rgb_args[0].AsInt()),
                       .green = static_cast<uint8_t>(rgb_args[1].AsInt()),
                       .blue = static_cast<uint8_t>(rgb_args[2].AsInt())};
    } else {
      color = Svg::Rgba{.red = static_cast<uint8_t>(rgb_args[0].AsInt()),
                        .green = static_cast<uint8_t>(rgb_args[1].AsInt()),
                        .blue = static_cast<uint8_t>(rgb_args[2].AsInt()),
                        .alpha = rgb_args[3].AsDouble()};
    }
  }

  return visit([](auto t) { return Svg::Color(t); }, color);
}

TransportDrawer::TransportDrawer(const Descriptions::StopsDict &stops_dict, const Descriptions::BusesDict &buses_dict,
                                 const Json::Dict &render_settings_json)
    : render_settings_(MakeRenderSettings(render_settings_json)),
      projection_(render_settings_),
      renderer_(make_shared<Svg::Document>()) {
  double min_lat = 0, max_lon = 0;

  if (!stops_dict.empty()) {
    min_lat = stops_dict.begin()->second->position.latitude;
    projection_.min_lon = stops_dict.begin()->second->position.longitude;
  }

  vector<string> sorted_stops_names;
  sorted_stops_names.reserve(stops_dict.size());
  for (const auto &[_, stop_info] : stops_dict) {
    sorted_stops_names.emplace_back(stop_info->name);

    projection_.max_lat = max(projection_.max_lat, stop_info->position.latitude);
    min_lat = min(min_lat, stop_info->position.latitude);

    max_lon = max(max_lon, stop_info->position.longitude);
    projection_.min_lon = min(projection_.min_lon, stop_info->position.longitude);
  }
  sort(sorted_stops_names.begin(), sorted_stops_names.end());

  vector<string> sorted_buses_names;
  sorted_buses_names.reserve(buses_dict.size());
  for (const auto &[_, bus_info] : buses_dict) {
    sorted_buses_names.emplace_back(bus_info->name);
  }
  sort(sorted_buses_names.begin(), sorted_buses_names.end());

  projection_.CaculateZoom(min_lat, max_lon);

  stringstream out;
  for (size_t i = 0; i < sorted_buses_names.size(); ++i) {
    DrawBusRoute(i, buses_dict.at(sorted_buses_names[i]), stops_dict);
  }

  for (size_t i = 0; i < sorted_buses_names.size(); ++i) {
    DrawBusName(i, buses_dict.at(sorted_buses_names[i]), stops_dict);
  }

  for (size_t i = 0; i < sorted_stops_names.size(); ++i) {
    DrawStop(stops_dict.at(sorted_stops_names[i]));
  }

  for (size_t i = 0; i < sorted_stops_names.size(); ++i) {
    DrawStopName(stops_dict.at(sorted_stops_names[i]));
  }
}

TransportDrawer::RenderSettings TransportDrawer::MakeRenderSettings(const Json::Dict &json) {
  const auto &palette = json.at("color_palette").AsArray();
  vector<Svg::Color> color_palette;
  transform(palette.cbegin(), palette.cend(), back_inserter(color_palette),
            [](auto &node) { return ParseColor(node); });


  const auto &layers_nodes = json.at("layers").AsArray();
  vector<string> layers;
  transform(layers_nodes.cbegin(), layers_nodes.cend(), back_inserter(layers),
            [](auto &node) { return node.AsString(); });

  RenderSettings render_settings{
      .width = json.at("width").AsDouble(),
      .height = json.at("height").AsDouble(),
      .padding = json.at("padding").AsDouble(),
      .stop_radius = json.at("stop_radius").AsDouble(),
      .line_width = json.at("line_width").AsDouble(),
      .stop_label_font_size = static_cast<uint32_t>(json.at("stop_label_font_size").AsInt()),
      .stop_label_offset = Svg::Point{.x = json.at("stop_label_offset").AsArray()[0].AsDouble(),
                                      .y = json.at("stop_label_offset").AsArray()[1].AsDouble()},
      .underlayer_color = ParseColor(json.at("underlayer_color")),
      .underlayer_width = json.at("underlayer_width").AsDouble(),
      .color_palette = move(color_palette),
      .bus_label_font_size = static_cast<uint32_t>(json.at("bus_label_font_size").AsInt()),
      .bus_label_offset = Svg::Point{.x = json.at("bus_label_offset").AsArray()[0].AsDouble(),
                                     .y = json.at("bus_label_offset").AsArray()[1].AsDouble()},
      .layers = move(layers)};

  return render_settings;
}

TransportDrawer::Projection::Projection(const RenderSettings &render_settings) : render_settings_(render_settings) {}

void TransportDrawer::Projection::CaculateZoom(double min_lat, double max_lon) {
  double width_zoom_coef = -1;
  if (max_lon != min_lon) {
    width_zoom_coef = (render_settings_.width - 2 * render_settings_.padding) / (max_lon - min_lon);
  }

  double height_zoom_coef = -1;
  if (max_lat != min_lat) {
    height_zoom_coef = (render_settings_.height - 2 * render_settings_.padding) / (max_lat - min_lat);
  }

  if ((width_zoom_coef == -1) ^ (height_zoom_coef == -1)) {
    zoom_coef = max(height_zoom_coef, width_zoom_coef);
  } else if ((width_zoom_coef != -1) && (height_zoom_coef != -1)) {
    zoom_coef = min(height_zoom_coef, width_zoom_coef);
  }
}

Svg::Point TransportDrawer::Projection::ConvertSpherePoint(const Sphere::Point &sphere_point) const {
  return {
      .x = (sphere_point.longitude - min_lon) * zoom_coef + render_settings_.padding,
      .y = (max_lat - sphere_point.latitude) * zoom_coef + render_settings_.padding,
  };
}

void TransportDrawer::DrawBusRoute(size_t id, const Descriptions::Bus *bus,
                                   const Descriptions::StopsDict &stops_dict) const {
  const Svg::Color &color = render_settings_.color_palette[id % render_settings_.color_palette.size()];
  auto polyline = Svg::Polyline()
                      .SetStrokeColor(color)
                      .SetStrokeWidth(render_settings_.line_width)
                      .SetStrokeLineCap("round")
                      .SetStrokeLineJoin("round");

  for (const auto stop_name : bus->stops) {
    auto stop = stops_dict.at(stop_name);
    polyline.AddPoint(projection_.ConvertSpherePoint(stop->position));
  }

  renderer_->Add("bus_lines", move(polyline));
}

void TransportDrawer::DrawBusName(size_t id, const Descriptions::Bus *bus,
                                  const Descriptions::StopsDict &stops_dict) const {
  const Svg::Color &color = render_settings_.color_palette[id % render_settings_.color_palette.size()];
  auto draw_text = [&](const Descriptions::Stop *stop) {
    auto shared_text = Svg::Text()
                           .SetPoint(projection_.ConvertSpherePoint(stop->position))
                           .SetOffset(render_settings_.bus_label_offset)
                           .SetFontSize(render_settings_.bus_label_font_size)
                           .SetFontFamily("Verdana")
                           .SetFontWeight("bold")
                           .SetData(bus->name);

    auto underlayer = Svg::Text(shared_text)
                          .SetFillColor(render_settings_.underlayer_color)
                          .SetStrokeColor(render_settings_.underlayer_color)
                          .SetStrokeWidth(render_settings_.underlayer_width)
                          .SetStrokeLineCap("round")
                          .SetStrokeLineJoin("round");

    auto text = Svg::Text(shared_text).SetFillColor(color);

    renderer_->Add("bus_labels", move(underlayer));
    renderer_->Add("bus_labels", move(text));
  };

  if (bus->stops.size() > 1) {
    auto first_final_stop = stops_dict.at(bus->stops[0]);
    auto second_final_stop = stops_dict.at(bus->stops.at(bus->stops.size() / 2));
    draw_text(first_final_stop);

    if (!(bus->is_roundtrip) && first_final_stop != second_final_stop) {
      draw_text(second_final_stop);
    }
  }
}

void TransportDrawer::DrawStop(const Descriptions::Stop *stop) const {
  auto circle = Svg::Circle()
                    .SetCenter(projection_.ConvertSpherePoint(stop->position))
                    .SetRadius(render_settings_.stop_radius)
                    .SetFillColor("white");

  renderer_->Add("stop_points", move(circle));
}

void TransportDrawer::DrawStopName(const Descriptions::Stop *stop) const {
  auto shared_text = Svg::Text()
                         .SetPoint(projection_.ConvertSpherePoint(stop->position))
                         .SetOffset(render_settings_.stop_label_offset)
                         .SetFontSize(render_settings_.stop_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetData(stop->name);

  auto underlayer = Svg::Text(shared_text)
                        .SetFillColor(render_settings_.underlayer_color)
                        .SetStrokeColor(render_settings_.underlayer_color)
                        .SetStrokeWidth(render_settings_.underlayer_width)
                        .SetStrokeLineCap("round")
                        .SetStrokeLineJoin("round");

  auto text = Svg::Text(shared_text).SetFillColor("black");

  renderer_->Add("stop_labels", move(underlayer));
  renderer_->Add("stop_labels", move(text));
}

shared_ptr<const string> TransportDrawer::Draw() const {
  if (!svg_map_) {
    ostringstream ss;
    renderer_->Render(ss, render_settings_.layers);
    this->svg_map_ = make_shared<const string>(ss.str());
  }

  return svg_map_;
}