#ifndef C_PLUS_PLUS_BELTS_READRESPONSE_H
#define C_PLUS_PLUS_BELTS_READRESPONSE_H

#include "Json.h"
#include <cmath>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct ReadResponse {
  size_t requestId;
  ReadResponse(size_t requestId) : requestId(requestId){};
  virtual ~ReadResponse() = default;
  virtual void Print(std::ostream &output) = 0;
  virtual Json::Document ToJson() = 0;
};

struct ReadStopResponse : ReadResponse {
  std::string stopName;
  std::vector<std::string> buses;
  explicit ReadStopResponse(std::string stopName, size_t requestId);
};

struct ReadNoStopResponse : ReadStopResponse {
  using ReadStopResponse::ReadStopResponse;
  void Print(std::ostream &output) override;
  Json::Document ToJson() override;
};

struct ReadStopMetricsResponse : ReadStopResponse {
  using ReadStopResponse::ReadStopResponse;
  void Print(std::ostream &output) override;
  Json::Document ToJson() override;
};

struct ReadBusResponse : ReadResponse {
  std::string busName;
  explicit ReadBusResponse(std::string busName, size_t requestId);
};

struct ReadNoBusResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  void Print(std::ostream &output) override;
  Json::Document ToJson() override;
};

struct ReadBusMetricsResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  void Print(std::ostream &output) override;
  Json::Document ToJson() override;
  size_t stopsCount;
  size_t uniqueStopsCount;
  double routeDistance;
  double routeDistanceV2;
};

#endif // C_PLUS_PLUS_BELTS_READRESPONSE_H
