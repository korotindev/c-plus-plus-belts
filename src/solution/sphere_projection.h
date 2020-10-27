#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "sphere.h"
#include "svg.h"
#include "utils.h"

namespace Sphere {

  class Projector {
   public:
    using StopsCollider =
        std::function<bool(const Descriptions::Stop* stop_ptr, const std::vector<const Descriptions::Stop*>& group)>;

    Projector(const Descriptions::StopsDict& stops_dict, const StopsCollider& collider, double max_width,
              double max_height, double padding);

    Svg::Point operator()(const Descriptions::Stop* stop) const;

   private:
    const double padding_;
    double x_step;
    double y_step;
    double max_height;

    struct StopGroup {
      size_t latitude_group;
      size_t longitude_group;
    };

    std::unordered_map<std::string, StopGroup> stop_groups;
  };
}  // namespace Sphere
