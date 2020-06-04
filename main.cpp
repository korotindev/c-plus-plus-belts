#include "Solution.h"

using namespace std;


int main() {
  TestRunner tr;
  RUN_TEST(tr, TestParsing);
  RUN_TEST(tr, TestIntegrationPartD);

  Json::Document document = Json::Load(cin);
  auto modifyRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "base_requests");
  auto readRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "stat_requests");
  Database db;
  ProcessModifyRequests(db, modifyRequests);
  auto responses = ProcessReadRequests(db, readRequests);
  PrintResponses(responses);

  return 0;
}
