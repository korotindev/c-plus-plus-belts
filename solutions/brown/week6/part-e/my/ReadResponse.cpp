#include "ReadResponse.h"


using namespace std;

ReadBusResponse::ReadBusResponse(string busName, size_t requestId)
  : ReadResponse(requestId),
    busName(move(busName)) {}

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

ReadRouteResponseItem::ReadRouteResponseItem(ReadRouteResponseItemType type_, double time_)
  : type(type_),
    time(time_) {}

ReadRouteResponseWaitItem::ReadRouteResponseWaitItem(std::string_view stopName_, double time_)
  : ReadRouteResponseItem(ReadRouteResponseItemType::WaitType, time_),
    stopName(stopName_) {}

Json::Node ReadRouteResponseWaitItem::ToJson() const {
  return Json::Node(
    map<string, Json::Node>(
      {
        {"type",      string("Wait")},
        {"stop_name", string(stopName)},
        {"time",      time}
      })
  );
}

Json::Node ReadRouteResponseBusItem::ToJson() const {
  return Json::Node(
    map<string, Json::Node>(
      {
        {"type",       string("Bus")},
        {"bus",        string(busName)},
        {"span_count", spanCount},
        {"time",       time}
      })
  );
}

ReadRouteResponseBusItem::ReadRouteResponseBusItem(std::string_view busName_, double time_, size_t spanCount_)
  : ReadRouteResponseItem(ReadRouteResponseItemType::BusType, time_),
    busName(busName_),
    spanCount(spanCount_) {}

Json::Document ReadNoRouteResponse::ToJson() {
  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id",    requestId},
        {"error_message", string("not found")},
      }
    )
  );
}

Json::Document ReadRouteMetricsResponse::ToJson() {
  vector<Json::Node> resultItems;

  for (const auto& item : items) {
    resultItems.push_back(item->ToJson());
  }

  return Json::Document(
    map<string, Json::Node>(
      {
        {"request_id", requestId},
        {"total_time", totalTime},
        {"items",      resultItems},
      }
    )
  );
}
