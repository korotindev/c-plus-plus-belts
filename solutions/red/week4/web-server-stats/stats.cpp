#include "stats.h"

Stats::Stats() {
  methodStats = {
      {"GET", 0}, {"POST", 0}, {"PUT", 0}, {"DELETE", 0}, {unkUpper, 0},
  };
  uriStats = {
      {"/", 0}, {"/order", 0}, {"/basket", 0}, {"/product", 0}, {"/help", 0}, {unkLower, 0},
  };
}

void Stats::AddMethod(string_view method) {
  if (method == "GET" || method == "POST" || method == "PUT" || method == "DELETE") {
    methodStats[method]++;
  } else {
    methodStats[unkUpper]++;
  }
}

void Stats::AddUri(string_view uri) {
  if (uri == "/" || uri == "/order" || uri == "/product" || uri == "/basket" || uri == "/help") {
    uriStats[uri]++;
  } else {
    uriStats[unkLower]++;
  }
}

const map<string_view, int> &Stats::GetMethodStats() const { return methodStats; }

const map<string_view, int> &Stats::GetUriStats() const { return uriStats; }

HttpRequest ParseRequest(string_view line) {
  HttpRequest httpRequest;
  line.remove_prefix(line.find_first_not_of(' '));
  auto firstSpace = line.find(' ');
  auto secondSpace = line.find(' ', firstSpace + 1);
  auto secondWordStart = firstSpace + 1;

  httpRequest.method = line.substr(0, firstSpace);
  httpRequest.uri = line.substr(secondWordStart, secondSpace - secondWordStart);
  httpRequest.protocol = line.substr(secondSpace + 1);
  return httpRequest;
}