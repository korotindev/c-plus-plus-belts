#ifndef C_PLUS_PLUS_BELTS_READRESPONSE_H
#define C_PLUS_PLUS_BELTS_READRESPONSE_H

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

struct ReadResponse {
  virtual ~ReadResponse() = default;
  virtual void Print(std::ostream &output) = 0;
};

struct ReadStopResponse : ReadResponse {
  std::string stopName;
  std::vector<std::string> buses;
  explicit ReadStopResponse(std::string stopName);
};

struct ReadNoStopResponse : ReadStopResponse {
  using ReadStopResponse::ReadStopResponse;
  void Print(std::ostream &output) override;
};

struct ReadStopMetricsResponse : ReadStopResponse {
  using ReadStopResponse::ReadStopResponse;
  void Print(std::ostream &output) override;
};

struct ReadBusResponse : ReadResponse {
  std::string busName;
  explicit ReadBusResponse(std::string busName_);
};

struct ReadNoBusResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  void Print(std::ostream &output) override;
};

struct ReadBusMetricsResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  void Print(std::ostream &output) override;
  size_t stopsCount;
  size_t uniqueStopsCount;
  double routeDistance;
  double routeDistanceV2;
};

#endif  // C_PLUS_PLUS_BELTS_READRESPONSE_H
