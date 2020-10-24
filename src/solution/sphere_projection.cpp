#include "sphere_projection.h"

namespace Sphere {

  Projector::Projector(std::vector<Point> &points, double max_width, double max_height, double padding)
      : padding_(padding), max_height(max_height) {
    if (points.empty()) {
      return;
    }

    double distances_count = static_cast<double>(points.size() - 1);

    x_step = (max_width - 2 * padding) / distances_count;
    y_step = (max_height - 2 * padding) / distances_count;

    sort(points.begin(), points.end(),
         [](const Point &lhs, const Point &rhs) { return lhs.longitude < rhs.longitude; });

    double idx = 0;

    for (const auto &point : points) {
      longitude_to_idx[point.longitude] = idx;
      idx++;
    }

    sort(points.begin(), points.end(), [](const Point &lhs, const Point &rhs) { return lhs.latitude < rhs.latitude; });

    idx = 0;

    for (const auto &point : points) {
      latitude_to_idx[point.latitude] = idx;
      idx++;
    }
  }

  Svg::Point Projector::operator()(Point point) const {
    return {
        longitude_to_idx.at(point.longitude) * x_step + padding_,
        max_height - padding_ -latitude_to_idx.at(point.latitude) * y_step
    };
  }

}  // namespace Sphere
