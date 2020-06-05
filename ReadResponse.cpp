#include "ReadResponse.h"


using namespace std;

ReadBusResponse::ReadBusResponse(string busName, size_t requestId)
  : ReadResponse(requestId),
    busName(move(busName)) {}

void ReadNoBusResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": not found";
}

Json::Document ReadNoBusResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id",    requestId},
        {"error_message", string("not found")},
      }
    )
  );
}

void ReadBusMetricsResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": "
         << stopsCount << " stops on route" << ", "
         << uniqueStopsCount << " unique stops" << ", "
         << routeDistanceV2 << " route length" << ", "
         << routeDistanceV2 / routeDistance << " curvature";
}

Json::Document ReadBusMetricsResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id",        requestId},
        {"stop_count",        stopsCount},
        {"unique_stop_count", uniqueStopsCount},
        {"route_length",      routeDistanceV2},
        {"curvature",         routeDistanceV2 / routeDistance},
      }
    )
  );
}

ReadStopResponse::ReadStopResponse(string stopName, size_t requestId)
  : ReadResponse(requestId),
    stopName(std::move(stopName)) {}

void ReadNoStopResponse::Print(ostream& output) {
  output << "Stop " << stopName << ": not found";
}

Json::Document ReadNoStopResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id",    requestId},
        {"error_message", string("not found")},
      }
    )
  );
}

void ReadStopMetricsResponse::Print(ostream& output) {
  output << "Stop " << stopName << ":";

  if (buses.empty()) {
    output << " no buses";
  } else {
    output << " buses";
    for (const auto& bus : buses) {
      output << " " << bus;
    }
  }
}

Json::Document ReadStopMetricsResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"buses",      vector<Json::Node>(buses.begin(), buses.end())},
        {"request_id", requestId}
      }
    )
  );
}
