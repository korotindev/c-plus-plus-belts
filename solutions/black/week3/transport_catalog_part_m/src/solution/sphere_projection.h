#pragma once

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "descriptions.h"
#include "sphere.h"
#include "svg.h"
#include "transport_info.h"
#include "utils.h"

namespace Sphere {

  class Projector {
   public:
    struct PointObject {
      size_t id;
      Sphere::Point position;
    };

    Projector(std::vector<PointObject>& points, std::shared_ptr<const TransportInfo> transport_info, double max_width,
              double max_height, double padding);

    Svg::Point operator()(const size_t id) const;

   private:
    std::shared_ptr<const TransportInfo> transport_info_;
    const double padding_;
    double x_step;
    double y_step;
    double max_height;

    struct ObjectGroup {
      size_t latitude_group;
      size_t longitude_group;
    };

    struct MaxGroupIdSearchResult {
      size_t max_latitude_group;
      size_t max_longitude_group;
    };

    std::optional<MaxGroupIdSearchResult> FindMaxGroupIdInRouteNeighbours(
        const Sphere::Projector::PointObject& stop_po,
        const std::vector<const Sphere::Projector::PointObject*>& processed) const;

    template <typename Saver>
    size_t CollideNonClosestPointObjects(std::vector<Projector::PointObject>& point_objects, Saver&& saver) {
      std::vector<const Projector::PointObject*> processed;

      size_t max_group_id = 0;

      for (size_t i = 0; i < point_objects.size(); i++) {
        auto max_groups_info = FindMaxGroupIdInRouteNeighbours(point_objects[i], processed);
        size_t group_id = saver(&point_objects[i], max_groups_info);
        max_group_id = std::max(group_id, max_group_id);
        processed.push_back(&point_objects[i]);
      }

      return max_group_id;
    }

    std::unordered_map<size_t, ObjectGroup> object_groups;
  };
}  // namespace Sphere
