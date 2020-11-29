#include "integration_tests.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "test_runner.h"
#include "transport_catalog.h"
#include "utils.h"

using namespace std;

void FindAndPrintSvg(const Json::Document &doc, filesystem::path &dir, string filename_part) {
  for (const auto &answer : doc.GetRoot().AsArray()) {
    if (answer.IsMap()) {
      const auto &answer_map = answer.AsMap();
      if (answer_map.count("map")) {
        const string &rendered_map = answer_map.at("map").AsString();
        string filename = to_string(answer_map.at("request_id").AsInt()) + "_" + filename_part + ".svg";
        ofstream out(dir / filename);
        out << rendered_map;
      }
    }
  }
}

void RunMain(const Json::Document &input_doc, ostream &out) {
  const auto &input_map = input_doc.GetRoot().AsMap();
  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(), input_map.at("render_settings").AsMap());
  out.precision(17);
  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), out);
  out << endl;
}

void TestIntegration(const string &test_data_folder_name) {
  auto input = ifstream(test_data_folder_name + "/input.json");
  const auto input_doc = Json::Load(input);

  filesystem::path dir = filesystem::current_path() / ".svg_test_results";
  filesystem::remove_all(dir);
  filesystem::create_directory(dir);

  stringstream output;
  RunMain(input_doc, output);
  auto result_doc = Json::Load(output);
  FindAndPrintSvg(result_doc, dir, "result");

  auto expected_doc_input = ifstream(test_data_folder_name + "/expected_output.json");
  Json::Document expected_doc = Json::Load(expected_doc_input);
  FindAndPrintSvg(expected_doc, dir, "expected");
  ASSERT_EQUAL(result_doc, expected_doc);
}
