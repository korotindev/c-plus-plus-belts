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
                                   const Json::Dict& render_settings_json) {
  auto stops_end =
      partition(begin(data), end(data), [](const auto& item) { return holds_alternative<Descriptions::Stop>(item); });

  for (const auto& item : Range{begin(data), stops_end}) {
    const auto& stop = get<Descriptions::Stop>(item);
    transport_info_->AddStop(stop);
  }

  for (const auto& item : Range{stops_end, end(data)}) {
    const auto& bus = get<Descriptions::Bus>(item);
    transport_info_->AddBus(bus);
  }

  router_ = make_unique<TransportRouter>(transport_info_, routing_settings_json);

  map_ = MapRenderer(transport_info_, render_settings_json).Render();
}

const TransportInfo::Stop* TransportCatalog::GetStop(const string& name) const {
  return transport_info_->GetStop(name);
}

const TransportInfo::Bus* TransportCatalog::GetBus(const string& name) const { return transport_info_->GetBus(name); }

optional<TransportRouter::RouteInfo> TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const {
  return router_->FindRoute(stop_from, stop_to);
}

string TransportCatalog::RenderMap() const {
  ostringstream out;
  out.precision(6);
  map_.Render(out);
  return out.str();
}
