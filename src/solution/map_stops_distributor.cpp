#include "map_stops_distributor.h"

using namespace std;

MapStopsDistributor::MapStopsDistributor(shared_ptr<const TransportInfo> transport_info) {
  unordered_map<size_t, bool> support_elements;

  auto mark_stop_as_support = [this, &support_elements](const TransportInfo::ConstStopPtr &stop_ptr) {
    support_elements[stop_ptr->id] = true;
    distribution[stop_ptr->id] = stop_ptr->position;
  };

  {
    unordered_map<size_t, bool> visited_buses;
    for (const auto stop_ptr : transport_info->GetStopsRange()) {
      if (size_t size = stop_ptr->bus_names.size(); size != 1) {
        mark_stop_as_support(stop_ptr);
      } else {
        const auto bus_ptr = transport_info->GetBus(*stop_ptr->bus_names.begin());
        if (visited_buses[bus_ptr->id]) {
          continue;
        }
        unordered_map<size_t, size_t> stops_stat;
        for (const auto &route_stop_name : bus_ptr->stops) {
          const auto route_stop_ptr = transport_info->GetStop(route_stop_name);
          const auto route_stop_visits_count = ++stops_stat[route_stop_ptr->id];
          if (route_stop_visits_count > 2) {
            mark_stop_as_support(route_stop_ptr);
          }
        }

        visited_buses[bus_ptr->id] = true;
      }
    }
  }

  auto consume_collected_idx = [&transport_info, &support_elements, this](const vector<string> &stops,
                                                                          const vector<size_t> &collected_idxs,
                                                                          size_t i, size_t j) {
    const auto first_stop = transport_info->GetStop(stops[i]);
    const auto first_stop_position = distribution.at(first_stop->id);
    const auto last_stop = transport_info->GetStop(stops[j]);
    const auto last_stop_position = distribution.at(last_stop->id);

    double lon_step = (last_stop_position.longitude - first_stop_position.longitude) / static_cast<double>(j - i);
    double lat_step = (last_stop_position.latitude - first_stop_position.latitude) / static_cast<double>(j - i);

    for (const size_t idx : collected_idxs) {
      const auto stop_ptr = transport_info->GetStop(stops[idx]);
      distribution[stop_ptr->id] =
          Sphere::Point{.latitude = first_stop_position.latitude + lat_step * static_cast<double>(idx - i),
                        .longitude = first_stop_position.longitude + lon_step * static_cast<double>(idx - i)};
    }
  };

  for (const auto bus_ptr : transport_info->GetBusesRange()) {
    const auto &stops = bus_ptr->stops;
    size_t prev_support_id = 0;
    vector<size_t> collected;

    for (const auto &stop_name : bus_ptr->endpoints) {
      mark_stop_as_support(transport_info->GetStop(stop_name));
    }

    for (size_t i = 1; i < stops.size(); i++) {
      const auto &stop_name = stops[i];
      const auto stop_ptr = transport_info->GetStop(stop_name);
      const bool is_support = support_elements[stop_ptr->id];

      if (is_support) {
        consume_collected_idx(bus_ptr->stops, collected, prev_support_id, i);
        prev_support_id = i;
        collected.clear();
      } else {
        collected.push_back(i);
      }
    }
    if (!collected.empty()) {
      consume_collected_idx(bus_ptr->stops, collected, prev_support_id, 0);
    }
  }
}

Sphere::Point MapStopsDistributor::operator()(const size_t id) const { return distribution.at(id); }