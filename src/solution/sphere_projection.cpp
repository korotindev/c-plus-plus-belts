#include "sphere_projection.h"

using namespace std;

namespace Sphere {

  namespace {
    bool HasShortPath(const Descriptions::Stop *stop_ptr, vector<const Descriptions::Stop *> group) {
      return any_of(group.begin(), group.end(), [stop_ptr](const Descriptions::Stop *group_elem) {
        return IsStopsCollide(*stop_ptr, *group_elem);
      });
    }

    template <typename Saver>
    void SaveCollidedGroup(const vector<const Descriptions::Stop *> &group, size_t id, Saver &&saver) {
      for (auto stop : group) {
        saver(stop, id);
      }
    }

    template <typename SortComparator, typename Saver>
    size_t CollideNonClosestStops(vector<const Descriptions::Stop *> &stops, SortComparator &&sort_comparator,
                                  Saver &&saver) {
      sort(stops.begin(), stops.end(), sort_comparator);

      size_t group_idx = 0;
      vector<const Descriptions::Stop *> group;
      group.push_back(stops[0]);

      for (size_t i = 1; i < stops.size(); i++) {
        const Descriptions::Stop *stop = stops[i];
        if (HasShortPath(stop, group)) {
          SaveCollidedGroup(group, group_idx++, saver);
          group.clear();
        }
        group.push_back(stop);
      }

      SaveCollidedGroup(group, group_idx++, saver);

      return group_idx;
    }
  }  // namespace

  Projector::Projector(const Descriptions::StopsDict &stops_dict, double max_width, double max_height, double padding)
      : padding_(padding), max_height(max_height) {
    if (stops_dict.empty()) {
      return;
    }

    vector<const Descriptions::Stop *> stops;
    stops.reserve(stops_dict.size());
    for (const auto &[_, stop_ptr] : stops_dict) {
      stops.push_back(stop_ptr);
    }

    size_t longitude_groups_count = CollideNonClosestStops(
        stops,
        [this](const Descriptions::Stop *lhs, const Descriptions::Stop *rhs) {
          return lhs->position.longitude < rhs->position.longitude;
        },
        [this](const Descriptions::Stop *stop, size_t group_id) {
          this->longitude_to_group_id[stop->position.longitude] = group_id;
        });

    x_step =
        longitude_groups_count > 1 ? (max_width - 2 * padding_) / static_cast<double>(longitude_groups_count - 1) : 0;

    size_t latitude_groups_count = CollideNonClosestStops(
        stops,
        [this](const Descriptions::Stop *lhs, const Descriptions::Stop *rhs) {
          return lhs->position.latitude < rhs->position.latitude;
        },
        [this](const Descriptions::Stop *stop, size_t group_id) {
          this->latitude_to_group_id[stop->position.latitude] = group_id;
        });

    y_step =
        latitude_groups_count > 1 ? (max_height - 2 * padding_) / static_cast<double>(latitude_groups_count - 1) : 0;
  }

  Svg::Point Projector::operator()(Point point) const {
    const double x_zoom = static_cast<double>(longitude_to_group_id.at(point.longitude));
    const double y_zoom = static_cast<double>(latitude_to_group_id.at(point.latitude));
    return {x_zoom * x_step + padding_, max_height - padding_ - y_zoom * y_step};
  }

}  // namespace Sphere
