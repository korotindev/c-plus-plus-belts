#pragma once

#include "iterator_range.h"

#include <string_view>
#include <sstream>
#include <vector>
using namespace std;

template <typename Container>
string Join(char c, const Container& cont) {
  ostringstream os;
  size_t size = cont.size() - 1;
  for (const auto& item : Head(cont, size)) {
    os << item << c;
  }
  os << *rbegin(cont);
  return os.str();
}

string_view Strip(string_view s);
vector<string_view> SplitBy(string_view s, char sep);
