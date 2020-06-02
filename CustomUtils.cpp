#include "CustomUtils.h"

using namespace std;

std::pair<std::string_view, std::optional<std::string_view>>
SplitTwoStrict(std::string_view s, std::string_view delimiter) {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, std::nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimiter) {
  const auto[lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
  return {lhs, rhs_opt.value_or("")};
}

std::string_view ReadToken(std::string_view& s, std::string_view delimiter) {
  const auto[lhs, rhs] = SplitTwo(s, delimiter);
  s = rhs;
  return lhs;
}

double ConvertToDouble(std::string_view str) {
  // use std::from_chars when available to git rid of string copy
  size_t pos;
  const double result = stod(std::string(str), &pos);
  if (pos != str.length()) {
    std::stringstream error;
    error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
    throw std::invalid_argument(error.str());
  }
  return result;
}