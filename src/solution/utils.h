#pragma once

#include <iterator>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename T>
class ConstSharedPtrsVectorIterator {
 public:
  typedef std::random_access_iterator_tag iterator_category;
  typedef std::shared_ptr<const T> value_type;
  typedef ptrdiff_t difference_type;
  typedef std::shared_ptr<const T>* pointer;
  typedef std::shared_ptr<const T>& reference;
  using underlaying_vector = std::vector<std::shared_ptr<T>>;

  explicit ConstSharedPtrsVectorIterator(typename underlaying_vector::const_iterator it) : it(it) {}
  bool operator!=(const ConstSharedPtrsVectorIterator<T>& other) { return it != other.it; }
  value_type operator*() { return *it; }
  void operator++() { it++; }

 private:
  underlaying_vector::const_iterator it;
};

template <typename It>
class Range {
 public:
  using ValueType = typename std::iterator_traits<It>::value_type;

  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }

 private:
  It begin_;
  It end_;
};

template <typename C>
auto AsRange(const C& container) {
  return Range{std::begin(container), std::end(container)};
}

template <typename T>
Range<ConstSharedPtrsVectorIterator<T>> AsRange(const std::vector<std::shared_ptr<T>>& container) {
  return Range{ConstSharedPtrsVectorIterator<T>(begin(container)), ConstSharedPtrsVectorIterator<T>(end(container))};
}

template <typename It>
size_t ComputeUniqueItemsCount(Range<It> range) {
  return std::unordered_set<typename Range<It>::ValueType>{range.begin(), range.end()}.size();
}

template <typename K, typename V>
V GetSharedValue(const std::unordered_map<K, V>& map, const K& key) {
  if (auto it = map.find(key); it != end(map)) {
    return it->second;
  } else {
    return nullptr;
  }
}

template <typename K, typename V>
V GetSharedValue(std::unordered_map<K, V>& map, const K& key) {
  if (auto it = map.find(key); it != end(map)) {
    return it->second;
  } else {
    return nullptr;
  }
}

std::string_view Strip(std::string_view line);

bool IsZero(double x);
