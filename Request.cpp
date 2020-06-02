//
// Created by Дмитрий Коротин on 01.06.2020.
//

#include "Request.h"
#include "CustomUtils.h"

using namespace std;

void EntertainStopRequest::ParseFrom(string_view input) {
  StopName = ReadToken(input, ": ");
  Latitude = ConvertToDouble(ReadToken(input, ", "));
  Longitude = ConvertToDouble(input);
};

void EntertainStopRequest::Process(Database& db) {
  db.EntertainStop(Stop(move(StopName), Coordinate{Latitude, Longitude}));
};

void EntertainBusRequest::ParseFrom(string_view input) {
  BusName = ReadToken(input, ": ");
  bool cyclic = input.find("-") == string_view::npos;
  string delimiter = cyclic ? " > " : " - ";
  while (!input.empty()) {
    StopsNames.emplace_back(ReadToken(input, delimiter));
  }

  if (!cyclic) {
    int storedSize = StopsNames.size();
    for (int i = storedSize - 2; i >= 0; i--) {
      StopsNames.emplace_back(StopsNames[i]);
    }
  }
};

void EntertainBusRequest::Process(Database& db) {
  Bus bus(move(BusName), move(StopsNames));
  db.EntertainBus(move(bus));
};

void ReadBusRequest::ParseFrom(string_view input) {
  BusName = ReadToken(input);
};

string ReadBusRequest::Process(Database& db) {
  stringstream output;
  output.precision(6);
  output << db.ReadBus(BusName);
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
    default:
      return nullptr;
  }
}

optional<Request::Type> ConvertRequestTypeFromString(const TypeConverter& converter, string_view type_str) {
  if (const auto it = converter.find(type_str);
    it != converter.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}

RequestHolder ParseRequest(const TypeConverter& converter, string_view request_str) {
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