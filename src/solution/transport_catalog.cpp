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

// Svg::Document TransportCatalog::BuildMap(shared_ptr<TransportInfo> transport_info_ptr,
//                                          const Json::Dict& render_settings_json) {
//   if (stops_dict.empty()) {
//     return {};
//   }

//   auto stops_collider = [this, &buses_dict](const Descriptions::Stop* stop_ptr,
//                                             const vector<const Descriptions::Stop*>& group) {
//     const auto& bus_names = this->GetStop(stop_ptr->name)->bus_names;

//     for (const auto other_stop_ptr : group) {
//       bool has_short_path_forward = stop_ptr->distances.count(other_stop_ptr->name) > 0;
//       bool has_short_path_backward = other_stop_ptr->distances.count(stop_ptr->name) > 0;

//       if (has_short_path_forward || has_short_path_backward) {
//         const auto& other_stop_buses = this->GetStop(other_stop_ptr->name)->bus_names;
//         for (const auto& bus_name : bus_names) {
//           if (auto it = other_stop_buses.find(bus_name); it != other_stop_buses.cend()) {
//             const auto& stops = buses_dict.at(bus_name)->stops;
//             for (size_t i = 1; i < stops.size(); ++i) {
//               if ((stops[i] == stop_ptr->name && stops[i - 1] == other_stop_ptr->name) ||
//                   (stops[i] == other_stop_ptr->name && stops[i - 1] == stop_ptr->name)) {
//                 return true;
//               }
//             }
//           }
//         }
//       }
//     }
//     return false;
//   };
// }
