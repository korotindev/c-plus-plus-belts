#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_info.h"

namespace MapRenderers {
  namespace Real {
    class RealMapRenderer : public IMapRenderer {
     public:
      void Prepare(std::shared_ptr<const TransportInfo> transport_info,
                   const Json::Dict& render_settings_json) override;
      Svg::Document Render() const override;

     private:
      std::map<std::string, Svg::Point> stops_coords_;
      std::unordered_map<std::string, Svg::Color> bus_colors_;

      void RenderBusLines(Svg::Document& svg) const;
      void RenderBusLabels(Svg::Document& svg) const;
      void RenderStopPoints(Svg::Document& svg) const;
      void RenderStopLabels(Svg::Document& svg) const;

      static const std::unordered_map<std::string, void (RealMapRenderer::*)(Svg::Document&) const> LAYER_ACTIONS;
    };

  }  // namespace Real
}  // namespace MapRenderers
