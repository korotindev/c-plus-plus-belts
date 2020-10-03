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
      projection_settings_(MakeProjectionSettings(render_settings_, stops_dict)) {
  for (const auto &[_, stop_info] : stops_dict) {
    auto stop =
        make_shared<Stop>(Stop{.name = stop_info->name, .position = ConvertSpherePointToSvgPoint(stop_info->position)});
    sorted_stops_names_.emplace_back(stop->name);
    stops_.emplace(stop->name, stop);
  }

  sort(sorted_stops_names_.begin(), sorted_stops_names_.end());

  for (const auto &[_, bus_info] : buses_dict) {
    auto bus = make_shared<Bus>(Bus{.name = bus_info->name, .stops = {}});

    for (string_view stop_name_sv : bus_info->stops) {
      auto it = stops_.find(stop_name_sv);
      bus->stops.emplace_back(it->first);
    }
    sorted_buses_names_.emplace_back(bus->name);
    buses_.emplace(bus->name, bus);
  }

  sort(sorted_buses_names_.begin(), sorted_buses_names_.end());
}

TransportDrawer::RenderSettings TransportDrawer::MakeRenderSettings(const Json::Dict &json) {
  const auto &palette = json.at("color_palette").AsArray();
  vector<Svg::Color> color_palette;
  transform(palette.cbegin(), palette.cend(), back_inserter(color_palette),
            [](auto &node) { return ParseColor(node); });

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
      .color_palette = move(color_palette)};

  return render_settings;
}

TransportDrawer::ProjectionSettings TransportDrawer::MakeProjectionSettings(
    const TransportDrawer::RenderSettings &render_settings, const Descriptions::StopsDict &stops_dict) {
  ProjectionSettings projection_settings;

  if (!stops_dict.empty()) {
    projection_settings.min_lat = stops_dict.begin()->second->position.latitude;
    projection_settings.min_lon = stops_dict.begin()->second->position.longitude;
  }

  for (auto &[_, stop] : stops_dict) {
    projection_settings.max_lat = max(projection_settings.max_lat, stop->position.latitude);
    projection_settings.min_lat = min(projection_settings.min_lat, stop->position.latitude);

    projection_settings.max_lon = max(projection_settings.max_lon, stop->position.longitude);
    projection_settings.min_lon = min(projection_settings.min_lon, stop->position.longitude);
  }

  double width_zoom_coef = -1;
  if (projection_settings.max_lon != projection_settings.min_lon) {
    width_zoom_coef = (render_settings.width - 2 * render_settings.padding) /
                      (projection_settings.max_lon - projection_settings.min_lon);
  }

  double height_zoom_coef = -1;
  if (projection_settings.max_lat != projection_settings.min_lat) {
    height_zoom_coef = (render_settings.height - 2 * render_settings.padding) /
                       (projection_settings.max_lat - projection_settings.min_lat);
  }

  if ((width_zoom_coef == -1) ^ (height_zoom_coef == -1)) {
    projection_settings.zoom_coef = max(height_zoom_coef, width_zoom_coef);
  } else if ((width_zoom_coef != -1) && (height_zoom_coef != -1)) {
    projection_settings.zoom_coef = min(height_zoom_coef, width_zoom_coef);
  }

  return projection_settings;
}

Svg::Point TransportDrawer::ConvertSpherePointToSvgPoint(Sphere::Point sphere_point) const {
  return {
      .x = (sphere_point.longitude - projection_settings_.min_lon) * projection_settings_.zoom_coef +
           render_settings_.padding,
      .y = (projection_settings_.max_lat - sphere_point.latitude) * projection_settings_.zoom_coef +
           render_settings_.padding,
  };
}

void TransportDrawer::DrawBusRoute(size_t id, Svg::Document &document) const {
  const auto bus = buses_.at(sorted_buses_names_.at(id));
  const Svg::Color &color = render_settings_.color_palette[id % render_settings_.color_palette.size()];
  auto polyline = Svg::Polyline()
                      .SetStrokeColor(color)
                      .SetStrokeWidth(render_settings_.line_width)
                      .SetStrokeLineCap("round")
                      .SetStrokeLineJoin("round");

  for (const auto stop_name : bus->stops) {
    auto stop = stops_.at(stop_name);
    polyline.AddPoint(stop->position);
  }

  document.Add(move(polyline));
}

void TransportDrawer::DrawStop(size_t id, Svg::Document &document) const {
  const auto stop = stops_.at(sorted_stops_names_.at(id));
  auto circle = Svg::Circle().SetCenter(stop->position).SetRadius(render_settings_.stop_radius).SetFillColor("white");

  document.Add(move(circle));

  auto shared_text = Svg::Text()
                         .SetPoint(stop->position)
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

  document.Add(move(underlayer));
  document.Add(move(text));
}

TransportDrawer::Map TransportDrawer::Draw() const {
  Svg::Document doc;

  stringstream out;
  for (size_t i = 0; i < sorted_buses_names_.size(); ++i) {
    DrawBusRoute(i, doc);
  }

  for (size_t i = 0; i < sorted_stops_names_.size(); ++i) {
    DrawStop(i, doc);
  }

  doc.Render(out);
  return {.svg = out.str()};
}