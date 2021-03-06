#ifndef C_PLUS_PLUS_BELTS_REQUEST_H
#define C_PLUS_PLUS_BELTS_REQUEST_H

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Coordinate.h"
#include "Database.h"
#include "Json.h"

struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
  enum class Type {
    EntertainStop,
    EntertainBus,
    ReadBus,
    ReadStop,
  };

  Request(Type type) : type(type) {}

  static RequestHolder Create(Type type);
  virtual void ParseFrom(std::map<std::string, Json::Node> &requestData) = 0;
  virtual ~Request() = default;
  const Type type;
};

using TypeConverter = std::unordered_map<std::string_view, Request::Type>;
const TypeConverter MODIFY_TYPES_CONVERTER = {
    {"Bus", Request::Type::EntertainBus},
    {"Stop", Request::Type::EntertainStop},
};
const TypeConverter READ_TYPES_CONVERTER = {
    {"Bus", Request::Type::ReadBus},
    {"Stop", Request::Type::ReadStop},
};

std::optional<Request::Type> ConvertRequestTypeFromString(const TypeConverter &converter, std::string_view type_str);

RequestHolder ParseRequest(const TypeConverter &converter, std::map<std::string, Json::Node> requestData);

template <typename ResultType>
struct ReadRequest : Request {
  using Request::Request;
  size_t id;
  virtual ResultType Process(Database &db) = 0;
};

struct ModifyRequest : Request {
  using Request::Request;
  virtual void Process(Database &db) = 0;
};

struct EntertainStopRequest : ModifyRequest {
  EntertainStopRequest() : ModifyRequest(Type::EntertainStop) {}

  void ParseFrom(std::map<std::string, Json::Node> &requestData) override;
  void Process(Database &db) override;
  std::string stopName;
  double latitude;
  double longitude;
  std::vector<StopDistance> distanceToOtherStops;
};

struct EntertainBusRequest : ModifyRequest {
  EntertainBusRequest() : ModifyRequest(Type::EntertainBus) {}

  void ParseFrom(std::map<std::string, Json::Node> &requestData) override;
  void Process(Database &db) override;
  std::string busName;
  std::vector<std::string> stopsNames;
};

struct ReadBusRequest : ReadRequest<Json::Document> {
  ReadBusRequest() : ReadRequest(Type::ReadBus) {}

  void ParseFrom(std::map<std::string, Json::Node> &requestData) override;
  Json::Document Process(Database &db) override;
  std::string busName;
};

struct ReadStopRequest : ReadRequest<Json::Document> {
  ReadStopRequest() : ReadRequest(Type::ReadStop) {}

  void ParseFrom(std::map<std::string, Json::Node> &requestData) override;
  Json::Document Process(Database &db) override;
  std::string stopName;
};

#endif  // C_PLUS_PLUS_BELTS_REQUEST_H
