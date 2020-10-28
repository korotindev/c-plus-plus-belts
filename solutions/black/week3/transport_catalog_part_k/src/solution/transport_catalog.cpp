#include "transport_catalog.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "map_renderer.h"
#include "utils.h"

using namespace std;

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery> data, const Json::Dict& routing_settings_json,
                                   const Json::Dict& render_settings_json)
    : transport_info_(make_shared<TransportInfo>()) {
  auto stops_end =
      partition(begin(data), end(data), [](const auto& item) { return holds_alternative<Descriptions::Stop>(item); });

  sort(begin(data), stops_end,
       [](auto lhs, auto rhs) { return get<Descriptions::Stop>(lhs).name < get<Descriptions::Stop>(rhs).name; });

  for (auto& item : Range{begin(data), stops_end}) {
    auto& stop = get<Descriptions::Stop>(item);
    transport_info_->AddStop(move(stop));
  }

  sort(stops_end, end(data),
       [](auto lhs, auto rhs) { return get<Descriptions::Bus>(lhs).name < get<Descriptions::Bus>(rhs).name; });

  for (auto& item : Range{stops_end, end(data)}) {
    auto& bus = get<Descriptions::Bus>(item);
    transport_info_->AddBus(move(bus));
  }

  router_ = make_unique<TransportRouter>(transport_info_, routing_settings_json);

  map_ = MapRenderer(transport_info_, render_settings_json).Render();
}

shared_ptr<const TransportInfo::Stop> TransportCatalog::GetStop(const string& name) const {
  return transport_info_->GetStop(name);
}

shared_ptr<const TransportInfo::Bus> TransportCatalog::GetBus(const string& name) const {
  return transport_info_->GetBus(name);
}

optional<TransportRouter::RouteInfo> TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const {
  return router_->FindRoute(stop_from, stop_to);
}

string TransportCatalog::RenderMap() const {
  ostringstream out;
  out.precision(6);
  map_.Render(out);
  return out.str();
}
