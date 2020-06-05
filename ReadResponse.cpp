#include "ReadResponse.h"

using namespace std;

ReadBusResponse::ReadBusResponse(string busName_) : busName(busName_) {}

void ReadNoBusResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": not found";
}

Json::Document ReadNoBusResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id",    static_cast<double>(this->requestId)},
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
        {"request_id", Json::Node(static_cast<double>(this->requestId))},
        {"stop_count", Json::Node(static_cast<double>(this->stopsCount))},
        {"unique_stop_count", Json::Node(static_cast<double>(this->uniqueStopsCount))},
        {"route_length", Json::Node(routeDistanceV2)},
        {"curvature", Json::Node(routeDistanceV2 / routeDistance)},
      }
    )
  );
}

ReadStopResponse::ReadStopResponse(string stopName) : stopName(std::move(stopName)) {}

void ReadNoStopResponse::Print(ostream& output) {
  output << "Stop " << stopName << ": not found";
}

Json::Document ReadNoStopResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id",    static_cast<double>(this->requestId)},
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
        {"request_id", Json::Node(static_cast<double>(this->requestId))}
      }
    )
  );
}
