#include "real.h"

#include <cassert>

#include "real_sphere_projection.h"
#include "map_stops_distributor.h"
#include "sphere.h"
#include "utils.h"

using namespace std;

namespace MapRenderers {
  namespace Real {
    static map<string, Svg::Point> ComputeStopsCoords(shared_ptr<const TransportInfo> transport_info,
                                                      const RenderSettings& render_settings) {
      const double max_width = render_settings.max_width;
      const double max_height = render_settings.max_height;
      const double padding = render_settings.padding;

      vector<Sphere::Point> point_objects;

      point_objects.reserve(transport_info->StopsCount());
      for (const auto stop_ptr : transport_info->GetStopsRange()) {
        point_objects.push_back(stop_ptr->position);
      }

      const RealSphere::RealProjector projector(begin(point_objects), end(point_objects), max_width, max_height,
                                                padding);

      map<string, Svg::Point> stops_coords;
      for (const auto stop_ptr : transport_info->GetStopsRange()) {
        stops_coords[stop_ptr->name] = projector(stop_ptr->position);
      }

      return stops_coords;
    }

    
    static map<string, Svg::Point> ComputeStopsCoordsWithDistribution(shared_ptr<const TransportInfo> transport_info,
                                                      const RenderSettings& render_settings) {
      const double max_width = render_settings.max_width;
      const double max_height = render_settings.max_height;
      const double padding = render_settings.padding;

      vector<Sphere::Point> point_objects;

      MapStopsDistributor distributor(transport_info);

      point_objects.reserve(transport_info->StopsCount());
      for (const auto stop_ptr : transport_info->GetStopsRange()) {
        point_objects.push_back(distributor(stop_ptr->id));
      }

      const RealSphere::RealProjector projector(begin(point_objects), end(point_objects), max_width, max_height,
                                                padding);

      map<string, Svg::Point> stops_coords;
      for (const auto stop_ptr : transport_info->GetStopsRange()) {
        stops_coords[stop_ptr->name] = projector(distributor(stop_ptr->id));
      }

      return stops_coords;
    }

    void RealMapRenderer::Prepare(shared_ptr<const TransportInfo> transport_info,
                                  const Json::Dict& render_settings_json) {
      transport_info_ = move(transport_info);
      render_settings_ = ParseRenderSettings(render_settings_json);
      stops_coords_ = ComputeStopsCoords(transport_info_, render_settings_);
      bus_colors_ = ChooseBusColors(transport_info_, render_settings_);
    }

    void RealMapRenderer::RenderBusLines(Svg::Document& svg) const {
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

    void RealMapRenderer::RenderBusLabels(Svg::Document& svg) const {
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

    void RealMapRenderer::RenderStopPoints(Svg::Document& svg) const {
      for (const auto& [stop_name, stop_point] : stops_coords_) {
        svg.Add(Svg::Circle{}.SetCenter(stop_point).SetRadius(render_settings_.stop_radius).SetFillColor("white"));
      }
    }

    void RealMapRenderer::RenderStopLabels(Svg::Document& svg) const {
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

    const unordered_map<string, void (RealMapRenderer::*)(Svg::Document&) const> RealMapRenderer::LAYER_ACTIONS = {
        {"bus_lines", &RealMapRenderer::RenderBusLines},
        {"bus_labels", &RealMapRenderer::RenderBusLabels},
        {"stop_points", &RealMapRenderer::RenderStopPoints},
        {"stop_labels", &RealMapRenderer::RenderStopLabels},
    };

    Svg::Document RealMapRenderer::Render() const {
      Svg::Document svg;

      for (const auto& layer : render_settings_.layers) {
        (this->*LAYER_ACTIONS.at(layer))(svg);
      }

      return svg;
    }

    void RealWithDistributionMapRender::Prepare(shared_ptr<const TransportInfo> transport_info,
                                                const Json::Dict& render_settings_json) {
      transport_info_ = move(transport_info);
      render_settings_ = ParseRenderSettings(render_settings_json);
      stops_coords_ = ComputeStopsCoordsWithDistribution(transport_info_, render_settings_);
      bus_colors_ = ChooseBusColors(transport_info_, render_settings_);
    }
  }  // namespace Real
}  // namespace MapRenderers
