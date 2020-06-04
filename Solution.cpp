#include "Solution.h"

using namespace std;

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

vector<string> ProcessReadRequests(Database& db, vector<RequestHolder>& requests) {
  vector<string> responses;
  for (const auto& request_holder : requests) {
    auto& request = dynamic_cast<ReadRequest<string>&>(*request_holder);
    responses.push_back(request.Process(db));
  }
  return responses;
}

void TestParsing() {
  ifstream input("../test_data/test_parsing.json");
  Json::Document document = Json::Load(input);
  const auto modifyRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "base_requests");
  const auto readRequests = ParseSpecificRequests(READ_TYPES_CONVERTER, document, "stat_requests");
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
  ASSERT_EQUAL(readRequests.size(), 2ul);
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
}

void PrintResponses(const vector<string>& responses, ostream& stream) {
  for (const auto& response : responses) {
    stream << response << '\n';
  }
}

void TestIntegrationGenerator(istream& input, ifstream& expectedOutput) {
  Json::Document document = Json::Load(input);
  auto modifyRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "base_requests");
  auto readRequests = ParseSpecificRequests(READ_TYPES_CONVERTER, document, "stat_requests");
  Database db;
  ProcessModifyRequests(db, modifyRequests);
  auto responses = ProcessReadRequests(db, readRequests);
  stringstream output;
  output.precision(7);
  PrintResponses(responses, output);
  Json::Document resultDocument = Json::Load(output);
  Json::Document expectedDocument = Json::Load(expectedOutput);
  ASSERT_EQUAL(resultDocument, expectedDocument);
}

void TestIntegrationPartD() {
  auto input = ifstream("../test_data/test_integration_part_d.json");
  auto expectedOutput = ifstream("../test_data/test_integration_part_d_expected_output.json");
  TestIntegrationGenerator(input,expectedOutput);
}
