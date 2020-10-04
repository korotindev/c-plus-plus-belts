#include "Request.h"

#include "CustomUtils.h"

using namespace std;

void EntertainStopRequest::ParseFrom(map<string, Json::Node> &requestData) {
  stopName = requestData["name"].AsString();
  latitude = requestData["latitude"].AsNumber();
  longitude = requestData["longitude"].AsNumber();
  auto &roadDistances = requestData["road_distances"].AsMap();
  for (auto &[name, distanceNode] : roadDistances) {
    distanceToOtherStops.push_back(StopDistance{name, static_cast<double>(distanceNode.AsNumber())});
  }
};

void EntertainStopRequest::Process(Database &db) {
  db.EntertainStop(Stop(move(stopName), Coordinate{latitude, longitude}, move(distanceToOtherStops)));
};

void EntertainBusRequest::ParseFrom(map<string, Json::Node> &requestData) {
  busName = requestData["name"].AsString();
  bool isRoundTrip = requestData["is_roundtrip"].AsBool();
  auto &stopsNodes = requestData["stops"].AsArray();

  for (auto &stopNode : stopsNodes) {
    stopsNames.emplace_back(stopNode.AsString());
  }

  if (!isRoundTrip && !stopsNames.empty()) {
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

void ReadBusRequest::ParseFrom(map<string, Json::Node> &requestData) {
  busName = requestData["name"].AsString();
  id = static_cast<size_t>(requestData["id"].AsNumber());
};

Json::Document ReadBusRequest::Process(Database &db) {
  auto responseHolder = db.ReadBus(busName, id);
  auto document = responseHolder->ToJson();
  return document;
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

RequestHolder ParseRequest(const TypeConverter &converter, map<string, Json::Node> requestData) {
  const auto rawType = requestData["type"].AsString();
  const auto request_type = ConvertRequestTypeFromString(converter, rawType);
  if (!request_type) {
    return nullptr;
  }
  RequestHolder request = Request::Create(*request_type);
  if (request) {
    request->ParseFrom(requestData);
  };
  return request;
}

void ReadStopRequest::ParseFrom(map<string, Json::Node> &requestData) {
  stopName = requestData["name"].AsString();
  id = static_cast<size_t>(requestData["id"].AsNumber());
}

Json::Document ReadStopRequest::Process(Database &db) {
  auto responseHolder = db.ReadStop(stopName, id);
  auto document = responseHolder->ToJson();
  return document;
}