#include "integration_tests.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "map_renderer.h"
#include "requests.h"
#include "sphere.h"
#include "test_runner.h"
#include "transport_catalog.h"
#include "utils.h"

using namespace std;

void FindAndPrintSvg(const Json::Document &doc, string filename) {
  for (const auto &answer : doc.GetRoot().AsArray()) {
    if (answer.IsMap()) {
      const auto &answer_map = answer.AsMap();
      if (answer_map.count("map")) {
        const string &rendered_map = answer_map.at("map").AsString();
        ofstream out(filename);
        out << rendered_map;
        break;
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

  stringstream output;
  RunMain(input_doc, output);
  auto result_doc = Json::Load(output);
  FindAndPrintSvg(result_doc, "test_result.svg");

  auto expected_doc_input = ifstream(test_data_folder_name + "/expected_output.json");
  Json::Document expected_doc = Json::Load(expected_doc_input);
  FindAndPrintSvg(expected_doc, "test_expected.svg");
  ASSERT_EQUAL(result_doc, expected_doc);
}
