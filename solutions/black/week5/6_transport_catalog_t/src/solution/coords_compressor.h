#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "sphere.h"

class CoordsCompressor {
 public:
  CoordsCompressor(const std::unordered_map<std::string, Sphere::Point>& stops_coords);

  void FillIndices(const std::unordered_map<double, std::vector<double>>& neighbour_lats,
                   const std::unordered_map<double, std::vector<double>>& neighbour_lons);

  void FillTargets(double max_width, double max_height, double padding);

  double MapLat(double value) const;
  double MapLon(double value) const;

 private:
  struct CoordInfo {
    double source;
    size_t idx = 0;
    double target = 0;

    bool operator<(const CoordInfo& other) const;
  };

  std::vector<CoordInfo> lats_;
  std::vector<CoordInfo> lons_;

  void FillCoordIndices(std::vector<CoordInfo>& coords,
                        const std::unordered_map<double, std::vector<double>>& neighbour_values);

  static const CoordInfo& Find(const std::vector<CoordInfo>& sorted_values, double value,
                               std::optional<std::vector<CoordInfo>::const_iterator> end_it = std::nullopt);

  static size_t FindMaxIdx(const std::vector<CoordInfo>& coords);

  size_t FindMaxLatIdx() const;
  size_t FindMaxLonIdx() const;
};