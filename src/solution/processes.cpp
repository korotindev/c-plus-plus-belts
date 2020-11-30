#include "processes.h"

#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "transport_catalog.h"

using namespace std;

void MakeBase(istream &input) {
  auto doc = Json::Load(input);
  const auto &input_map = doc.GetRoot().AsMap();

  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(), input_map.at("render_settings").AsMap());

  SerializeTransportCatalog(db, input_map.at("serialization_settings").AsMap());
}

void ProcessRequests(istream &input, ostream &output) {
  auto doc = Json::Load(input);
  const auto &input_map = doc.GetRoot().AsMap();

  TransportCatalog db = ParseTransportCatalog(input_map.at("serialization_settings").AsMap());

  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), output);
}