#pragma once

#include "json.h"
#include "svg.h"
#include "svg.pb.h"

namespace Svg {

  void SerializePoint(Point point, SvgProto::Point& proto);
  Point DeserializePoint(const SvgProto::Point& proto);
  Svg::Point ParsePoint(const Json::Node& json);

  void SerializeColor(const Color& color, SvgProto::Color& proto);
  Color DeserializeColor(const SvgProto::Color& proto);
  Svg::Color ParseColor(const Json::Node& json);

}  // namespace Svg
