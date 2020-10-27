#include "sphere_projection.h"

using namespace std;

namespace Sphere {

  namespace {
    template <typename Saver>
    void SaveCollidedGroup(const vector<const Projector::PointObject *> &group, size_t id, Saver &&saver) {
      for (const auto point_object : group) {
        saver(point_object, id);
      }
    }

    template <typename SortComparator, typename Saver>
    size_t CollideNonClosestPointObjects(vector<Projector::PointObject> &point_objects,
                                         const Projector::PointObjectsCollider &collider,
                                         SortComparator &&sort_comparator, Saver &&saver) {
      sort(point_objects.begin(), point_objects.end(), sort_comparator);

      size_t group_idx = 0;
      vector<const Projector::PointObject *> group;
      group.push_back(&point_objects[0]);

      for (size_t i = 1; i < point_objects.size(); i++) {
        if (collider(point_objects[i], group)) {
          SaveCollidedGroup(group, group_idx++, saver);
          group.clear();
        }
        group.push_back(&point_objects[i]);
      }

      SaveCollidedGroup(group, group_idx++, saver);

      return group_idx;
    }
  }  // namespace

  Projector::Projector(vector<PointObject> &point_objects, const PointObjectsCollider &collider, double max_width,
                       double max_height, double padding)
      : padding_(padding), max_height(max_height) {
    if (point_objects.empty()) {
      return;
    }

    size_t latitude_groups_count = CollideNonClosestPointObjects(
        point_objects, collider,
        [this](const PointObject &lhs, const PointObject &rhs) {
          return lhs.position.latitude < rhs.position.latitude;
        },
        [this](const PointObject *point_object, size_t group_id) {
          this->object_groups[point_object->id].latitude_group = group_id;
        });

    y_step =
        latitude_groups_count > 1 ? (max_height - 2 * padding_) / static_cast<double>(latitude_groups_count - 1) : 0;

    size_t longitude_groups_count = CollideNonClosestPointObjects(
        point_objects, collider,
        [this](const PointObject &lhs, const PointObject &rhs) {
          return lhs.position.longitude < rhs.position.longitude;
        },
        [this](const PointObject *point_object, size_t group_id) {
          this->object_groups[point_object->id].longitude_group = group_id;
        });

    x_step =
        longitude_groups_count > 1 ? (max_width - 2 * padding_) / static_cast<double>(longitude_groups_count - 1) : 0;
  }

  Svg::Point Projector::operator()(const string &id) const {
    const double y_zoom = static_cast<double>(object_groups.at(id).latitude_group);
    const double x_zoom = static_cast<double>(object_groups.at(id).longitude_group);
    return {
        .x = x_zoom * x_step + padding_,
        .y = max_height - padding_ - y_zoom * y_step,
    };
  }

}  // namespace Sphere
