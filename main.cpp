#include "Solution.h"

using namespace std;


int main() {
  TestRunner tr;
  RUN_TEST(tr, TestIntegrationPartD);

  auto modifyRequests = ParseRequests(MODIFY_TYPES_CONVERTER, cin);
  auto readRequests = ParseRequests(READ_TYPES_CONVERTER, cin);
  Database db;
  ProcessModifyRequests(db, modifyRequests);
  auto responses = ProcessReadRequests(db, readRequests);
  PrintResponses(responses);

  return 0;
}
