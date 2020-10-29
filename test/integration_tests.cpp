#include "integration_tests.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "test_runner.h"
#include "transport_catalog.h"
#include "utils.h"
#include "map_renderer.h"

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

void TestIntegration(const string &testDataFolderName) {
  auto input = ifstream(testDataFolderName + "/input.json");
  auto expectedOutput = ifstream(testDataFolderName + "/expected_output.json");

  const auto input_doc = Json::Load(input);
  const auto &input_map = input_doc.GetRoot().AsMap();
  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(), input_map.at("render_settings").AsMap(), make_unique<DefaultMapRenderer>());
  stringstream output;
  output.precision(17);
  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), output);
  output << endl;

  Json::Document resultDocument = Json::Load(output);
  Json::Document expectedDocument = Json::Load(expectedOutput);
  FindAndPrintSvg(resultDocument, "test_result.svg");
  FindAndPrintSvg(expectedDocument, "test_expected.svg");
  ASSERT_EQUAL(resultDocument, expectedDocument);
}