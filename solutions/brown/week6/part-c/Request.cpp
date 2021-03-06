#include "Request.h"

#include "CustomUtils.h"

using namespace std;

void EntertainStopRequest::ParseFrom(string_view input) {
  stopName = ReadToken(input, ": ");
  latitude = ConvertToDouble(ReadToken(input, ", "));
  longitude = ConvertToDouble(ReadToken(input, ", "));
  auto token = ReadToken(input, ", ");
  while (!token.empty()) {
    auto distanceToTargetStop = ConvertToDouble(ReadToken(token, "m to "));
    distanceToOtherStops.push_back(StopDistance{string(token), distanceToTargetStop});
    token = ReadToken(input, ", ");
  }
};

void EntertainStopRequest::Process(Database &db) {
  db.EntertainStop(Stop(move(stopName), Coordinate{latitude, longitude}, move(distanceToOtherStops)));
};

void EntertainBusRequest::ParseFrom(string_view input) {
  busName = ReadToken(input, ": ");
  bool cyclic = input.find("-") == string_view::npos;
  string delimiter = cyclic ? " > " : " - ";
  while (!input.empty()) {
    stopsNames.emplace_back(ReadToken(input, delimiter));
  }

  if (!cyclic && !stopsNames.empty()) {
    size_t storedSize = stopsNames.size();
    for (size_t i = storedSize - 1; i > 0; i--) {
      stopsNames.emplace_back(stopsNames[i - 1]);
    }
  }
};

void EntertainBusRequest::Process(Database &db) {
  Bus bus(move(busName), move(stopsNames));
  db.EntertainBus(move(bus));
};

void ReadBusRequest::ParseFrom(string_view input) { busName = ReadToken(input, "\n"); };

string ReadBusRequest::Process(Database &db) {
  stringstream output;
  output.precision(DEFAULT_PRECISION);
  auto responseHolder = db.ReadBus(busName);
  responseHolder->Print(output);
  return output.str();
};

RequestHolder Request::Create(Request::Type type) {
  switch (type) {
    case Request::Type::EntertainBus:
      return make_unique<EntertainBusRequest>();
    case Request::Type::EntertainStop:
      return make_unique<EntertainStopRequest>();
    case Request::Type::ReadBus:
      return make_unique<ReadBusRequest>();
    case Request::Type::ReadStop:
      return make_unique<ReadStopRequest>();
    default:
      return nullptr;
  }
}

optional<Request::Type> ConvertRequestTypeFromString(const TypeConverter &converter, string_view type_str) {
  if (const auto it = converter.find(type_str); it != converter.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}

RequestHolder ParseRequest(const TypeConverter &converter, string_view request_str) {
  // cerr << request_str << '\n';
  const auto request_type = ConvertRequestTypeFromString(converter, ReadToken(request_str));
  if (!request_type) {
    return nullptr;
  }
  RequestHolder request = Request::Create(*request_type);
  if (request) {
    request->ParseFrom(request_str);
  };
  return request;
}

void ReadStopRequest::ParseFrom(std::string_view input) { stopName = ReadToken(input, "\n"); }

std::string ReadStopRequest::Process(Database &db) {
  stringstream output;
  output.precision(DEFAULT_PRECISION);
  auto responseHolder = db.ReadStop(stopName);
  responseHolder->Print(output);
  return output.str();
}