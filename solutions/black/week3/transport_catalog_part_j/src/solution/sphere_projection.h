#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <unordered_map>
#include <vector>

#include "sphere.h"
#include "svg.h"
#include "utils.h"

namespace Sphere {

  class Projector {
   public:
    Projector(std::vector<Point> &points, double max_width, double max_height, double padding);

    Svg::Point operator()(Point point) const;

   private:
    const double padding_;
    double x_step;
    double y_step;
    double max_height;
    std::unordered_map<double, double> longitude_to_idx;
    std::unordered_map<double, double> latitude_to_idx;
  };
}  // namespace Sphere
