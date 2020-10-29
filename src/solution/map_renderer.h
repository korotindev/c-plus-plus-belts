#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "sphere_projection.h"
#include "svg.h"
#include "transport_info.h"

struct RenderSettings {
  double max_width;
  double max_height;
  double padding;
  std::vector<Svg::Color> palette;
  double line_width;
  Svg::Color underlayer_color;
  double underlayer_width;
  double stop_radius;
  Svg::Point bus_label_offset;
  int bus_label_font_size;
  Svg::Point stop_label_offset;
  int stop_label_font_size;
  std::vector<std::string> layers;
};

class IMapRenderer {
 public:
  virtual void Prepare(std::shared_ptr<const TransportInfo> transport_info, const Json::Dict& render_settings_json) = 0;
  virtual Svg::Document Render() const = 0;
  virtual ~IMapRenderer() {}

 protected:
  std::shared_ptr<const TransportInfo> transport_info_;
  RenderSettings render_settings_;
};

class DefaultMapRenderer : public IMapRenderer {
 public:
  void Prepare(std::shared_ptr<const TransportInfo> transport_info, const Json::Dict& render_settings_json) override;
  Svg::Document Render() const override;

 private:
  std::map<std::string, Svg::Point> stops_coords_;
  std::unordered_map<std::string, Svg::Color> bus_colors_;

  void RenderBusLines(Svg::Document& svg) const;
  void RenderBusLabels(Svg::Document& svg) const;
  void RenderStopPoints(Svg::Document& svg) const;
  void RenderStopLabels(Svg::Document& svg) const;

  static const std::unordered_map<std::string, void (DefaultMapRenderer::*)(Svg::Document&) const> LAYER_ACTIONS;
};
