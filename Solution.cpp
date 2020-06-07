#include "Solution.h"

using namespace std;

void InitializeSettings(const Json::Document& document, const string& key) {
  auto documentRootData = document.GetRoot().AsMap();
  auto settingsNode = documentRootData.at(key);
  Settings::InitializeFrom(settingsNode);
}

vector<unique_ptr<Request>> ParseSpecificRequests(TypeConverter converter, Json::Document& document, string key) {
  auto documentRootData = document.GetRoot().AsMap();
  auto& requests = documentRootData[key].AsArray();
  vector<unique_ptr<Request>> result;
  result.reserve(requests.size());
  for (size_t i = 0; i < requests.size(); i++) {
    result.emplace_back(ParseRequest(converter, requests[i].AsMap()));
  }
  return result;
}

void ProcessModifyRequests(Database& db, vector<RequestHolder>& requests) {
  for (const auto& request_holder : requests) {
    auto& request = dynamic_cast<ModifyRequest&>(*request_holder);
    request.Process(db);
  }
}

Json::Document ProcessReadRequests(Database& db, vector<RequestHolder>& requests) {
  vector<Json::Node> responses;
  responses.reserve(requests.size());
  for (const auto& request_holder : requests) {
    auto& request = dynamic_cast<ReadRequest<Json::Document>&>(*request_holder);
    responses.push_back(request.Process(db).GetRoot());
  }
  auto document = Json::Document(responses);
  return document;
}

void TestParsing() {
  ifstream input("../test_data/test_parsing.json");
  Json::Document document = Json::Load(input);
  InitializeSettings(document, "routing_settings");
  const auto modifyRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "base_requests");
  const auto readRequests = ParseSpecificRequests(READ_TYPES_CONVERTER, document, "stat_requests");
  {
    ASSERT_EQUAL(Settings::GetBusVelocity(), 40ul);
    ASSERT_EQUAL(Settings::GetBusWaitTime(), 6ul);
  }
  ASSERT_EQUAL(modifyRequests.size(), 3ul);
  {
    auto request = dynamic_cast<const EntertainStopRequest&>(*modifyRequests[0].get());
    ASSERT_EQUAL(request.stopName, "Tolstopaltsevo");
    ASSERT(abs(request.latitude - 55.611087) <= 0.0001);
    ASSERT(abs(request.longitude - 37.20829) <= 0.0001);
    vector<StopDistance> distanceToOtherStops = {{"Marushkino", 3900}};
    ASSERT_EQUAL(request.distanceToOtherStops, distanceToOtherStops);
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest&>(*modifyRequests[1].get());
    ASSERT_EQUAL(request.busName, "256")
    const vector<string> stopsNames = {"Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Zapadnoye"};
    ASSERT_EQUAL(request.stopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest&>(*modifyRequests[2].get());
    ASSERT_EQUAL(request.busName, "750")
    const vector<string> stopsNames = {"Tolstopaltsevo", "Marushkino", "Rasskazovka", "Marushkino", "Tolstopaltsevo"};
    ASSERT_EQUAL(request.stopsNames, stopsNames)
  }
  ASSERT_EQUAL(readRequests.size(), 3ul);
  {
    auto request = dynamic_cast<const ReadBusRequest&>(*readRequests[0].get());
    ASSERT_EQUAL(request.busName, "256");
    ASSERT_EQUAL(request.id, 1965312327ul);
  }
  {
    auto request = dynamic_cast<const ReadStopRequest&>(*readRequests[1].get());
    ASSERT_EQUAL(request.stopName, "Samara");
    ASSERT_EQUAL(request.id, 746888088ul);
  }
  {
    auto request = dynamic_cast<const ReadRouteRequest&>(*readRequests[2].get());
    ASSERT_EQUAL(request.from, "Biryulyovo Zapadnoye");
    ASSERT_EQUAL(request.to, "Universam");
    ASSERT_EQUAL(request.id, 746888089ul);
  }
}

void PrintResponses(const vector<string>& responses, ostream& stream) {
  for (const auto& response : responses) {
    stream << response << '\n';
  }
}

void TestIntegrationGenerator(const string& testDataFolderName) {
  auto input = ifstream("../test_data/" + testDataFolderName + "/input.json");
  auto expectedOutput = ifstream("../test_data/" + testDataFolderName + "/expected_output.json");
  Json::Document document = Json::Load(input);
  auto modifyRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "base_requests");
  auto readRequests = ParseSpecificRequests(READ_TYPES_CONVERTER, document, "stat_requests");
  Database db;
  ProcessModifyRequests(db, modifyRequests);
  auto jsonDoc = ProcessReadRequests(db, readRequests);
  stringstream output;
  output.precision(DEFAULT_PRECISION);
  output << jsonDoc;
  Json::Document resultDocument = Json::Load(output);
  Json::Document expectedDocument = Json::Load(expectedOutput);
  ASSERT_EQUAL(resultDocument, expectedDocument);
}

void TestIntegrationTest1() {
  TestIntegrationGenerator("test1");
}

void TestIntegrationTest2() {
  TestIntegrationGenerator("test2");
}

void TestIntegrationTest3() {
  TestIntegrationGenerator("test3");
}

void TestIntegrationTest4() {
  TestIntegrationGenerator("test4");
}