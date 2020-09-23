#include "tests.h"

using namespace std;

void TestIntegrationGenerator(const string &testDataFolderName) {
  auto input = ifstream("../test_data/" + testDataFolderName + "/input.json");
  const auto input_doc = Json::Load(input);

  const auto& input_map = input_doc.GetRoot().AsMap();
  const TransportCatalog db(
    Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
    input_map.at("routing_settings").AsMap()
  );
  stringstream output;
  Json::PrintValue(
    Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
    output
  );
  output << endl;

  Json::Document resultDocument = Json::Load(output);
  auto expectedOutput = ifstream("../test_data/" + testDataFolderName + "/expected_output.json");
  Json::Document expectedDocument = Json::Load(expectedOutput);
  ASSERT_EQUAL(resultDocument, expectedDocument);
}

void TestIntegrationTest1() { TestIntegrationGenerator("test1"); }

void TestIntegrationTest2() { TestIntegrationGenerator("test2"); }

void TestIntegrationTest3() { TestIntegrationGenerator("test3"); }

void TestIntegrationTest4() { TestIntegrationGenerator("test4"); }