#ifndef C_PLUS_PLUS_BELTS_READRESPONSE_H
#define C_PLUS_PLUS_BELTS_READRESPONSE_H

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <utility>
#include "Json.h"

struct ReadResponse {
  size_t requestId;
  ReadResponse(size_t requestId) : requestId(requestId) {};
  virtual ~ReadResponse() = default;
  virtual Json::Document ToJson() = 0;
};

struct ReadStopResponse : ReadResponse {
  std::string stopName;
  std::vector<std::string> buses;
  explicit ReadStopResponse(std::string stopName, size_t requestId);
};

struct ReadNoStopResponse : ReadStopResponse {
  using ReadStopResponse::ReadStopResponse;
  Json::Document ToJson() override;
};

struct ReadStopMetricsResponse : ReadStopResponse {
  using ReadStopResponse::ReadStopResponse;
  Json::Document ToJson() override;
};

struct ReadBusResponse : ReadResponse {
  std::string busName;
  explicit ReadBusResponse(std::string busName, size_t requestId);
};

struct ReadNoBusResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  Json::Document ToJson() override;
};

struct ReadBusMetricsResponse : ReadBusResponse {
  using ReadBusResponse::ReadBusResponse;
  Json::Document ToJson() override;
  size_t stopsCount;
  size_t uniqueStopsCount;
  double routeDistance;
  double routeDistanceV2;
};

enum ReadRouteResponseItemType { WaitType, BusType };

struct ReadRouteResponseItem {
  ReadRouteResponseItemType type;
  double time;
  ReadRouteResponseItem(ReadRouteResponseItemType type, double time);
  virtual Json::Node ToJson() const = 0;
  virtual ~ReadRouteResponseItem() = default;
};

struct ReadRouteResponseWaitItem : public ReadRouteResponseItem {
  std::string_view stopName;
  Json::Node ToJson() const override;
  ReadRouteResponseWaitItem(std::string_view stopName, double time);
};
struct ReadRouteResponseBusItem : public ReadRouteResponseItem {
  std::string_view busName;
  size_t spanCount;
  Json::Node ToJson() const override;
  ReadRouteResponseBusItem(std::string_view busName, double time, size_t spanCount);
};

struct ReadRouteResponse : ReadResponse {
  using ReadResponse::ReadResponse;
};

struct ReadRouteMetricsResponse : ReadRouteResponse {
  using ReadRouteResponse::ReadRouteResponse;
  double totalTime;
  std::vector<std::unique_ptr<ReadRouteResponseItem>> items;
  Json::Document ToJson() override;
};

struct ReadNoRouteResponse : ReadRouteResponse {
  using ReadRouteResponse::ReadRouteResponse;
  Json::Document ToJson() override;
};


#endif //C_PLUS_PLUS_BELTS_READRESPONSE_H
