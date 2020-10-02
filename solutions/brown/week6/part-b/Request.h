#ifndef C_PLUS_PLUS_BELTS_REQUEST_H
#define C_PLUS_PLUS_BELTS_REQUEST_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Coordinate.h"
#include "Database.h"

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
  virtual void ParseFrom(std::string_view input) = 0;
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

RequestHolder ParseRequest(const TypeConverter &converter, std::string_view request_str);

template <typename ResultType>
struct ReadRequest : Request {
  using Request::Request;
  virtual ResultType Process(Database &db) = 0;
};

struct ModifyRequest : Request {
  using Request::Request;
  virtual void Process(Database &db) = 0;
};

struct EntertainStopRequest : ModifyRequest {
  EntertainStopRequest() : ModifyRequest(Type::EntertainStop) {}

  void ParseFrom(std::string_view input) override;
  void Process(Database &db) override;
  std::string StopName;
  double Latitude;
  double Longitude;
};

struct EntertainBusRequest : ModifyRequest {
  EntertainBusRequest() : ModifyRequest(Type::EntertainBus) {}

  void ParseFrom(std::string_view input) override;
  void Process(Database &db) override;
  std::string BusName;
  std::vector<std::string> StopsNames;
};

struct ReadBusRequest : ReadRequest<std::string> {
  ReadBusRequest() : ReadRequest(Type::ReadBus) {}

  void ParseFrom(std::string_view input) override;
  std::string Process(Database &db) override;
  std::string BusName;
};

struct ReadStopRequest : ReadRequest<std::string> {
  ReadStopRequest() : ReadRequest(Type::ReadStop) {}

  void ParseFrom(std::string_view input) override;
  std::string Process(Database &db) override;
  std::string StopName;
};

#endif  // C_PLUS_PLUS_BELTS_REQUEST_H
