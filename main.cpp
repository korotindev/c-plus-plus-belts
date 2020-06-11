#include "Solution.h"

using namespace std;


int main() {
  std::istream::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.precision(DEFAULT_PRECISION);

  TestRunner tr;
//  commit tests before sending to grader
  RUN_TEST(tr, TestParsing);
  RUN_TEST(tr, TestIntegrationTest1);
  RUN_TEST(tr, TestIntegrationTest2);
  RUN_TEST(tr, TestIntegrationTest3);
  RUN_TEST(tr, TestIntegrationTest4);

  Json::Document document = Json::Load(cin);
  InitializeSettings(document, "routing_settings");
  auto modifyRequests = ParseSpecificRequests(MODIFY_TYPES_CONVERTER, document, "base_requests");
  auto readRequests = ParseSpecificRequests(READ_TYPES_CONVERTER, document, "stat_requests");
  Database db;
  db.BuildRouter();
  ProcessModifyRequests(db, modifyRequests);
  auto jsonDoc = ProcessReadRequests(db, readRequests);
  cout << jsonDoc;

  return 0;
}
