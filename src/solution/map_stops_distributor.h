#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "sphere.h"
#include "transport_info.h"

class MapStopsDistributor {
 public:
  explicit MapStopsDistributor(std::shared_ptr<const TransportInfo> transport_info);

  Sphere::Point operator()(const size_t id) const;

 private:
  std::shared_ptr<TransportInfo> transport_info;
  std::unordered_map<size_t, Sphere::Point> distribution;
};
