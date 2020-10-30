#include "sphere_projection.h"

#include <algorithm>

using namespace std;

namespace Sphere {
  Projector::Projector(vector<PointObject> &point_objects, shared_ptr<const TransportInfo> transport_info,
                       double max_width, double max_height, double padding)
      : transport_info_(move(transport_info)), padding_(padding), max_height(max_height) {
    if (point_objects.empty()) {
      return;
    }

    sort(point_objects.begin(), point_objects.end(), [this](const PointObject &lhs, const PointObject &rhs) {
      return lhs.position.latitude < rhs.position.latitude;
    });

    size_t max_latitude_group_id = CollideNonClosestPointObjects(
        point_objects, [this](const PointObject *point_object, optional<MaxGroupIdSearchResult> group_id_info) {
          size_t group_id = group_id_info ? group_id_info->max_latitude_group + 1 : 0;
          this->object_groups[point_object->id].latitude_group = group_id;
          return group_id;
        });

    y_step = max_latitude_group_id > 0 ? (max_height - 2 * padding_) / static_cast<double>(max_latitude_group_id) : 0;

    sort(point_objects.begin(), point_objects.end(), [this](const PointObject &lhs, const PointObject &rhs) {
      return lhs.position.longitude < rhs.position.longitude;
    });

    size_t max_longitude_group_id = CollideNonClosestPointObjects(
        point_objects, [this](const PointObject *point_object, optional<MaxGroupIdSearchResult> group_id_info) {
          size_t group_id = group_id_info ? group_id_info->max_longitude_group + 1 : 0;
          this->object_groups[point_object->id].longitude_group = group_id;
          return group_id;
        });

    x_step = max_longitude_group_id > 0 ? (max_width - 2 * padding_) / static_cast<double>(max_longitude_group_id) : 0;
  }

  Svg::Point Projector::operator()(const size_t id) const {
    const double y_zoom = static_cast<double>(object_groups.at(id).latitude_group);
    const double x_zoom = static_cast<double>(object_groups.at(id).longitude_group);
    return {
        .x = x_zoom * x_step + padding_,
        .y = max_height - padding_ - y_zoom * y_step,
    };
  }

  optional<Projector::MaxGroupIdSearchResult> Projector::FindMaxGroupIdInRouteNeighbours(
      const Sphere::Projector::PointObject &stop_po,
      const vector<const Sphere::Projector::PointObject *> &processed) const {
    optional<MaxGroupIdSearchResult> result;

    const auto stop_ptr = transport_info_->GetStop(stop_po.id);
    const auto &bus_names = stop_ptr->bus_names;

    for (const auto other_stop_po : processed) {
      const auto other_stop_ptr = transport_info_->GetStop(other_stop_po->id);

      auto other_stop_ptr_group_it = object_groups.find(other_stop_ptr->id);

      if (other_stop_ptr_group_it == object_groups.end()) {
        continue;
      }

      bool has_short_path_forward = stop_ptr->distances.count(other_stop_ptr->name) > 0;
      bool has_short_path_backward = other_stop_ptr->distances.count(stop_ptr->name) > 0;

      if (has_short_path_forward || has_short_path_backward) {
        const auto &other_stop_buses = transport_info_->GetStop(other_stop_ptr->name)->bus_names;

        for (const auto &bus_name : bus_names) {
          if (auto it = other_stop_buses.find(bus_name); it != other_stop_buses.cend()) {
            const auto &stops = transport_info_->GetBus(bus_name)->stops;

            for (size_t i = 1; i < stops.size(); ++i) {
              if ((stops[i] == stop_ptr->name && stops[i - 1] == other_stop_ptr->name) ||
                  (stops[i] == other_stop_ptr->name && stops[i - 1] == stop_ptr->name)) {
                if (result) {
                  result->max_longitude_group =
                      max(other_stop_ptr_group_it->second.longitude_group, result->max_longitude_group);
                  result->max_latitude_group =
                      max(other_stop_ptr_group_it->second.latitude_group, result->max_latitude_group);

                } else {
                  result =
                      MaxGroupIdSearchResult{.max_latitude_group = other_stop_ptr_group_it->second.latitude_group,
                                             .max_longitude_group = other_stop_ptr_group_it->second.longitude_group};
                }

                break;
              }
            }
          }
        }
      }
    }

    return result;
  }

}  // namespace Sphere
