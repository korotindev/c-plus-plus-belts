#include "map_stops_distributor.h"

using namespace std;

MapStopsDistributor::MapStopsDistributor(shared_ptr<const TransportInfo> transport_info) {
  unordered_map<size_t, bool> support_elements;

  auto mark_stop_as_support = [this, &support_elements](const TransportInfo::ConstStopPtr &stop_ptr) {
    support_elements[stop_ptr->id] = stop_ptr->id;
    distribution[stop_ptr->id] = stop_ptr->position;
  };

  for (const auto stop_ptr : transport_info->GetStopsRange()) {
    if (size_t size = stop_ptr->bus_names.size(); size > 1 || size == 0) {
      mark_stop_as_support(stop_ptr);
    }
  }

  auto consume_collected_idx = [&transport_info, &support_elements, this](const vector<string> &stops,
                                                                          const vector<size_t> &collected_idxs,
                                                                          size_t i, size_t j) {
    const auto first_stop = transport_info->GetStop(stops[i]);
    const auto last_stop = transport_info->GetStop(stops[j]);

    double lon_step = (last_stop->position.longitude - first_stop->position.longitude) / static_cast<double>(j - i);
    double lat_step = (last_stop->position.latitude - first_stop->position.latitude) / static_cast<double>(j - i);

    for (const size_t idx : collected_idxs) {
      const auto stop_ptr = transport_info->GetStop(stops[idx]);
      distribution[stop_ptr->id] =
          Sphere::Point{.latitude = first_stop->position.latitude + lat_step * static_cast<double>(idx - i),
                        .longitude = first_stop->position.longitude + lon_step * static_cast<double>(idx - i)};
    }
  };

  for (const auto bus_ptr : transport_info->GetBusesRange()) {
    const auto &stops = bus_ptr->stops;
    unordered_map<size_t, size_t> stops_stat; // TODO: Use this to find support stops in this for loop.
    size_t prev_support_id = 0;
    vector<size_t> collected;

    mark_stop_as_support(transport_info->GetStop(stops[0]));
    if (!bus_ptr->is_roundtrip) {
      mark_stop_as_support(transport_info->GetStop(stops[stops.size() / 2]));
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