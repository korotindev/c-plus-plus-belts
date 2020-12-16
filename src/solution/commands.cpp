#include "commands.h"

#include <fstream>

#include "requests.h"
#include "transport_catalog.h"

using namespace std;

string ReadFileData(const string& file_name) {
  ifstream file(file_name, ios::binary | ios::ate);
  const ifstream::pos_type end_pos = file.tellg();
  file.seekg(0, ios::beg);

  string data(end_pos, '\0');
  file.read(&data[0], end_pos);
  return data;
}

void ProcessRequests(istream& in, ostream& out) {
  const auto input_doc = Json::Load(in);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const string& file_name = input_map.at("serialization_settings").AsMap().at("file").AsString();
  const auto db = TransportCatalog::Deserialize(ReadFileData(file_name));

  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), out);
  out << endl;
}

void MakeBase(istream& in) {
  const auto input_doc = Json::Load(in);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            Descriptions::ReadYellowPages(input_map.at("yellow_pages").AsMap()),
                            input_map.at("routing_settings").AsMap(), input_map.at("render_settings").AsMap());

  const string& file_name = input_map.at("serialization_settings").AsMap().at("file").AsString();
  ofstream file(file_name);
  file << db.Serialize();
}