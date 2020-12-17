#include "coords_compressor.h"

#include <algorithm>

using namespace std;

CoordsCompressor::CoordsCompressor(const unordered_map<string, Sphere::Point>& stops_coords) {
  for (const auto& [_, coord] : stops_coords) {
    lats_.push_back({coord.latitude});
    lons_.push_back({coord.longitude});
  }

  sort(begin(lats_), end(lats_));
  sort(begin(lons_), end(lons_));
}

void CoordsCompressor::FillIndices(const unordered_map<double, vector<double>>& neighbour_lats,
                                   const unordered_map<double, vector<double>>& neighbour_lons) {
  FillCoordIndices(lats_, neighbour_lats);
  FillCoordIndices(lons_, neighbour_lons);
}

void CoordsCompressor::FillTargets(double max_width, double max_height, double padding) {
  if (lats_.empty() || lons_.empty()) {
    return;
  }

  const size_t max_lat_idx = FindMaxLatIdx();
  const double y_step = max_lat_idx ? (max_height - 2 * padding) / max_lat_idx : 0;

  const size_t max_lon_idx = FindMaxLonIdx();
  const double x_step = max_lon_idx ? (max_width - 2 * padding) / max_lon_idx : 0;

  for (auto& [_, idx, value] : lats_) {
    value = max_height - padding - idx * y_step;
  }
  for (auto& [_, idx, value] : lons_) {
    value = idx * x_step + padding;
  }
}

double CoordsCompressor::MapLat(double value) const { return Find(lats_, value).target; }

double CoordsCompressor::MapLon(double value) const { return Find(lons_, value).target; }

bool CoordsCompressor::CoordInfo::operator<(const CoordInfo& other) const { return source < other.source; }

void CoordsCompressor::FillCoordIndices(vector<CoordInfo>& coords,
                                        const unordered_map<double, vector<double>>& neighbour_values) {
  for (auto coord_it = begin(coords); coord_it != end(coords); ++coord_it) {
    const auto neighbours_it = neighbour_values.find(coord_it->source);
    if (neighbours_it == neighbour_values.end()) {
      coord_it->idx = 0;
      continue;
    }
    const auto& neighbours = neighbours_it->second;
    optional<size_t> max_neighbour_idx;
    for (const double value : neighbours) {
      const size_t idx = Find(coords, value, coord_it).idx;
      if (idx > max_neighbour_idx) {
        max_neighbour_idx = idx;
      }
    }
    coord_it->idx = *max_neighbour_idx + 1;
  }
}

const CoordsCompressor::CoordInfo& CoordsCompressor::Find(
    const vector<CoordInfo>& sorted_values, double value,
    optional<vector<CoordInfo>::const_iterator> end_it) {
  return *lower_bound(begin(sorted_values), end_it.value_or(end(sorted_values)), CoordInfo{value});
}

size_t CoordsCompressor::FindMaxIdx(const vector<CoordsCompressor::CoordInfo>& coords) {
  return max_element(begin(coords), end(coords),
                     [](const CoordsCompressor::CoordInfo& lhs, const CoordsCompressor::CoordInfo& rhs) {
                       return lhs.idx < rhs.idx;
                     })
      ->idx;
}

size_t CoordsCompressor::FindMaxLatIdx() const { return FindMaxIdx(lats_); }

size_t CoordsCompressor::FindMaxLonIdx() const { return FindMaxIdx(lons_); }
