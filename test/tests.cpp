#include "tests.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"
#include "test_runner.h"

using namespace std;

void TestIntegration(const string &testDataFolderName) {
  auto input = ifstream(testDataFolderName + "/input.json");
  auto expectedOutput = ifstream(testDataFolderName + "/expected_output.json");

  const auto input_doc = Json::Load(input);
  const auto &input_map = input_doc.GetRoot().AsMap();
  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(), input_map.at("render_settings").AsMap());
  stringstream output;
  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), output);
  output << endl;

  cout << output.str();

  Json::Document resultDocument = Json::Load(output);
  Json::Document expectedDocument = Json::Load(expectedOutput);
  ASSERT_EQUAL(resultDocument, expectedDocument);
}