#include "descriptions.h"

using namespace std;

namespace Descriptions {

  Stop Stop::ParseFrom(const Json::Dict& attrs) {
    Stop stop = {.name = attrs.at("name").AsString(),
                 .position =
                     {
                         .latitude = attrs.at("latitude").AsDouble(),
                         .longitude = attrs.at("longitude").AsDouble(),
                     },
                 .distances = {}};
    if (attrs.count("road_distances") > 0) {
      for (const auto& [neighbour_stop, distance_node] : attrs.at("road_distances").AsMap()) {
        stop.distances[neighbour_stop] = distance_node.AsInt();
      }
    }
    return stop;
  }

  static vector<string> ParseStops(const Json::Array& stop_nodes, bool is_roundtrip) {
    vector<string> stops;
    stops.reserve(stop_nodes.size());
    for (const Json::Node& stop_node : stop_nodes) {
      stops.push_back(stop_node.AsString());
    }
    if (is_roundtrip || stops.size() <= 1) {
      return stops;
    }
    stops.reserve(stops.size() * 2 - 1);  // end stop is not repeated
    for (size_t stop_idx = stops.size() - 1; stop_idx > 0; --stop_idx) {
      stops.push_back(stops[stop_idx - 1]);
    }
    return stops;
  }

  size_t ComputeStopsDistance(const Stop& lhs, const Stop& rhs) {
    if (auto it = lhs.distances.find(rhs.name); it != lhs.distances.end()) {
      return it->second;
    } else {
      return rhs.distances.at(lhs.name);
    }
  }

  Bus Bus::ParseFrom(const Json::Dict& attrs) {
    const auto& name = attrs.at("name").AsString();
    const auto& stops = attrs.at("stops").AsArray();
    if (stops.empty()) {
      return Bus{.name = name, .stops = {}, .endpoints = {}};
    } else {
      Bus bus{.name = name,
              .stops = ParseStops(stops, attrs.at("is_roundtrip").AsBool()),
              .endpoints = {stops.front().AsString(), stops.back().AsString()}};
      if (bus.endpoints.back() == bus.endpoints.front()) {
        bus.endpoints.pop_back();
      }
      return bus;
    }
  }

  void Bus::Serialize(TCProto::BusDescription& proto) const {
    proto.set_name(name);
    for (const string& stop : stops) {
      proto.add_stops(stop);
    }
    for (const string& stop : endpoints) {
      proto.add_endpoints(stop);
    }
  }

  Bus Bus::Deserialize(const TCProto::BusDescription& proto) {
    Bus bus;
    bus.name = proto.name();

    bus.stops.reserve(proto.stops_size());
    for (const auto& stop : proto.stops()) {
      bus.stops.push_back(stop);
    }

    bus.endpoints.reserve(proto.endpoints_size());
    for (const auto& stop : proto.endpoints()) {
      bus.endpoints.push_back(stop);
    }

    return bus;
  }

  vector<InputQuery> ReadDescriptions(const Json::Array& nodes) {
    vector<InputQuery> result;
    result.reserve(nodes.size());

    for (const Json::Node& node : nodes) {
      const auto& node_dict = node.AsMap();
      if (node_dict.at("type").AsString() == "Bus") {
        result.push_back(Bus::ParseFrom(node_dict));
      } else {
        result.push_back(Stop::ParseFrom(node_dict));
      }
    }

    return result;
  }

  static YellowPages::Phone ReadPhone(const Json::Dict& attrs) {
    YellowPages::Phone phone;

    phone.set_type(YellowPages::Phone_Type::Phone_Type_PHONE);
    if (attrs.count("type")) {
      if (attrs.at("type").AsString() == "FAX") {
        phone.set_type(YellowPages::Phone_Type::Phone_Type_FAX);
      }
    }

    if (attrs.count("country_code")) {
      *phone.mutable_country_code() = attrs.at("country_code").AsString();
    }

    if (attrs.count("local_code")) {
      *phone.mutable_local_code() = attrs.at("local_code").AsString();
    }

    if (attrs.count("number")) {
      *phone.mutable_number() = attrs.at("number").AsString();
    }

    if (attrs.count("extension")) {
      *phone.mutable_extension() = attrs.at("extension").AsString();
    }

    return phone;
  }

  static YellowPages::Name ReadName(const Json::Dict& attrs) {
    YellowPages::Name name;
    name.set_value(attrs.at("value").AsString());
    if (attrs.count("type")) {
      const auto& type = attrs.at("type").AsString();
      if (type == "MAIN") {
        name.set_type(YellowPages::Name_Type_MAIN);
      } else if (type == "SYNONYM") {
        name.set_type(YellowPages::Name_Type_SYNONYM);
      } else {
        name.set_type(YellowPages::Name_Type_SHORT);
      }
    }
    return name;
  }

  static YellowPages::NearbyStop ReadNearbyStop(const Json::Dict& attrs) {
    YellowPages::NearbyStop stop;
    stop.set_name(attrs.at("name").AsString());
    stop.set_meters(attrs.at("meters").AsInt());
    return stop;
  }

  static const std::string& GetMainCompanyName(const YellowPages::Company& company) {
    for (const auto& name : company.names()) {
      if (name.type() == YellowPages::Name_Type::Name_Type_MAIN) {
        return name.value();
      }
    }

    throw domain_error("name without appropriate types");
  }

  static YellowPages::WorkingTimeInterval ReadWorkingTimeInterval(const Json::Dict& attrs) {
    YellowPages::WorkingTimeInterval interval;
    if (attrs.count("day")) {
      YellowPages::WorkingTimeInterval::Day day;
      YellowPages::WorkingTimeInterval::Day_Parse(attrs.at("day").AsString(), &day);
      interval.set_day(move(day));
    }
    interval.set_minutes_from(attrs.at("minutes_from").AsInt());
    interval.set_minutes_to(attrs.at("minutes_to").AsInt());
    return interval;
  }

  static YellowPages::WorkingTime ReadWorkingTime(const Json::Dict& attrs) {
    YellowPages::WorkingTime working_time;

    if (!attrs.count("intervals")) {
      return working_time;
    }

    const auto& nodes = attrs.at("intervals").AsArray();
    if (nodes.empty()) {
      return working_time;
    }

    for(auto &node : nodes) {
      *working_time.add_intervals() = ReadWorkingTimeInterval(node.AsMap());
    }

    return working_time;
  }

  static std::string GetFullCompanyName(const YellowPages::Database& database, const YellowPages::Company& company) {
    const string& name = GetMainCompanyName(company);
    if (company.rubrics_size() > 0) {
      return database.rubrics().at(company.rubrics()[0]).name() + " " + name;
    }

    return name;
  }

  YellowPages::Database ReadYellowPages(const Json::Dict& attrs) {
    YellowPages::Database db;
    for (const auto& [id, rubric] : attrs.at("rubrics").AsMap()) {
      YellowPages::Rubric r;
      *r.mutable_name() = rubric.AsMap().at("name").AsString();
      (*db.mutable_rubrics())[stoi(id)] = move(r);
    }

    size_t id = 0;

    for (const auto& company_node : attrs.at("companies").AsArray()) {
      YellowPages::Company company;
      company.set_id(to_string(id++));
      const auto& dict = company_node.AsMap();

      {
        YellowPages::Address addr;
        const auto& attrs = dict.at("address").AsMap().at("coords").AsMap();
        (*addr.mutable_coords()).set_lat(stod(attrs.at("lat").AsString()));
        (*addr.mutable_coords()).set_lon(stod(attrs.at("lon").AsString()));
        (*company.mutable_address()) = move(addr);
      }

      if (dict.count("rubrics")) {
        for (const auto& rubric_id_node : dict.at("rubrics").AsArray()) {
          company.add_rubrics(rubric_id_node.AsInt());
        }
      }

      for (const auto& name_node : dict.at("names").AsArray()) {
        *company.add_names() = ReadName(name_node.AsMap());
      }

      (*company.mutable_cached_main_name()) = GetMainCompanyName(company);
      (*company.mutable_cached_full_name()) = GetFullCompanyName(db, company);

      if (dict.count("urls")) {
        for (const auto& url_node : dict.at("urls").AsArray()) {
          YellowPages::Url url;
          url.set_value(url_node.AsMap().at("value").AsString());
          *company.add_urls() = move(url);
        }
      }

      if (dict.count("phones")) {
        for (const auto& phone_node : dict.at("phones").AsArray()) {
          *company.add_phones() = ReadPhone(phone_node.AsMap());
        }
      }

      if (dict.count("nearby_stops")) {
        for (const auto& stop_node : dict.at("nearby_stops").AsArray()) {
          *company.add_nearby_stops() = ReadNearbyStop(stop_node.AsMap());
        }
      }

      if(dict.count("working_time")) {
        *company.mutable_working_time() = ReadWorkingTime(dict.at("working_time").AsMap());
      }

      *db.add_companies() = move(company);
    }
    return db;
  }
}  // namespace Descriptions
