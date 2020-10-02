#pragma once

#include <map>
#include <sstream>
#include <string_view>

#include "http_request.h"
using namespace std;

class Stats {
 public:
  Stats();

  void AddMethod(string_view method);
  void AddUri(string_view uri);

  [[nodiscard]] const map<string_view, int> &GetMethodStats() const;
  [[nodiscard]] const map<string_view, int> &GetUriStats() const;

 private:
  map<string_view, int> methodStats;
  map<string_view, int> uriStats;
  string_view unkLower = "unknown";
  string_view unkUpper = "UNKNOWN";
};

HttpRequest ParseRequest(string_view line);