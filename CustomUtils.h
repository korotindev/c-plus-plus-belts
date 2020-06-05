#ifndef C_PLUS_PLUS_BELTS_CUSTOMUTILS_H
#define C_PLUS_PLUS_BELTS_CUSTOMUTILS_H

#include <string_view>
#include <optional>
#include <string>
#include <sstream>
#include <iostream>

template<typename It>
class Range {
public:
  Range(It begin, It end) : begin_(begin), end_(end) {}

  It begin() const { return begin_; }

  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

std::pair<std::string_view, std::optional<std::string_view>>
SplitTwoStrict(std::string_view s, std::string_view delimiter = " ");

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimiter = " ");

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ");

double ConvertToDouble(std::string_view str);


template<typename Number>
Number ReadNumberOnLine(std::istream& stream) {
  Number number;
  stream >> number;
  std::string dummy;
  getline(stream, dummy);
  return number;
}

static const size_t DEFAULT_PRECISION = 7;

#endif //C_PLUS_PLUS_BELTS_CUSTOMUTILS_H
