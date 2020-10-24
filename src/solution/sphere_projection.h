#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "sphere.h"
#include "svg.h"
#include "utils.h"

namespace Sphere {

  class Projector {
   public:
    Projector(const Descriptions::StopsDict& stops_dict, double max_width, double max_height, double padding);

    Svg::Point operator()(Point point) const;

   private:
    const double padding_;
    double x_step;
    double y_step;
    double max_height;
    std::unordered_map<double, size_t> longitude_to_group_id;
    std::unordered_map<double, size_t> latitude_to_group_id;
  };
}  // namespace Sphere
