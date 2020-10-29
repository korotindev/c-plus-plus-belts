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

// namespace {
//   unordered_map<size_t, bool> DetermineSupportElements(shared_ptr<const TransportInfo> transport_info) {
//     unordered_map<size_t, bool> support_elements;

//     for(const auto stop_ptr : transport_info->GetStopsRange()) {
//         cout << stop_ptr->name;
//     }

//     return support_elements;
//   }
// }

MapStopsDistributor::MapStopsDistributor(shared_ptr<const TransportInfo>) {
  
}

Sphere::Point MapStopsDistributor::operator()(const size_t id) const {
  return distribution.at(id);
}