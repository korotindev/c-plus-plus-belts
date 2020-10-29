// #pragma once

// #include <memory>
// #include <string>
// #include <unordered_map>

// #include "sphere.h"
// #include "transport_info.h"

// class MapStopsDistributor {
//  public:
//   explicit MapStopsDistributor(std::shared_ptr<const TransportInfo> transport_info);

//   Sphere::Point operator()(size_t id) const;

//  private:
//   std::shared_ptr<TransportInfo> transport_info;
//   std::unordered_map<size_t, Sphere::Point> distribution;
// };

#include "map_stops_distributor.h"

using namespace std;

namespace {
  unordered_map<size_t, bool> DetermineSupportElements(shared_ptr<const TransportInfo> transport_info) {
    unordered_map<size_t, bool> support_elements;

    for (const auto stop_ptr : transport_info->GetStopsRange()) {
      if (size_t size = stop_ptr->bus_names.size(); size > 1 || size == 0) {
        support_elements[stop_ptr->id] = stop_ptr->id;
      } else {
      }
    }

    return support_elements;
  }
}  // namespace

MapStopsDistributor::MapStopsDistributor(shared_ptr<const TransportInfo> transport_info) {
  unordered_map<size_t, bool> support_elements = DetermineSupportElements(transport_info);
  for (const auto &[id, is_support] : support_elements) {
    if (is_support) {
      distribution[id] = transport_info->GetStop(id)->position;
    }
  }
}

Sphere::Point MapStopsDistributor::operator()(const size_t id) const { return distribution.at(id); }