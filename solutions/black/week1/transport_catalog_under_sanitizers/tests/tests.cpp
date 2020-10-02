#include "tests.h"
#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"
#include "utils/test_runner.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void TestIntegrationGenerator(const string &testDataFolderName) {
  auto input = ifstream("samples/" + testDataFolderName + "/input.json");
  auto expectedOutput = ifstream("samples/" + testDataFolderName + "/expected_output.json");

  const auto input_doc = Json::Load(input);
  const auto &input_map = input_doc.GetRoot().AsMap();
  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap());
  stringstream output;
  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), output);
  output << endl;

  Json::Document resultDocument = Json::Load(output);
  Json::Document expectedDocument = Json::Load(expectedOutput);
  ASSERT_EQUAL(resultDocument, expectedDocument);
}

void TestIntegrationTest1() { TestIntegrationGenerator("test1"); }

void TestIntegrationTest2() { TestIntegrationGenerator("test2"); }

void TestIntegrationTest3() { TestIntegrationGenerator("test3"); }

void TestIntegrationTest4() { TestIntegrationGenerator("test4"); }