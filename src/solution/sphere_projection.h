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
    struct PointObject {
      const std::string id;
      const Sphere::Point position;
    };

    using PointObjectsCollider =
        std::function<bool(const PointObject& object, const std::vector<const PointObject*>& group)>;

    Projector(std::vector<PointObject>& points, const PointObjectsCollider& collider, double max_width,
              double max_height, double padding);

    Svg::Point operator()(const std::string& name) const;

   private:
    const double padding_;
    double x_step;
    double y_step;
    double max_height;

    struct ObjectGroup {
      size_t latitude_group;
      size_t longitude_group;
    };

    std::unordered_map<std::string, ObjectGroup> object_groups;
  };
}  // namespace Sphere
