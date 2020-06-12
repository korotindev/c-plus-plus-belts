#include "Solution.h"

using namespace std;


int main() {
  std::istream::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.precision(DEFAULT_PRECISION);

  TestRunner tr;
//  commit tests before sending to grader
//  RUN_TEST(tr, TestParsing);
//  RUN_TEST(tr, TestIntegrationTest1);
//  RUN_TEST(tr, TestIntegrationTest2);
//  RUN_TEST(tr, TestIntegrationTest3);
//  RUN_TEST(tr, TestIntegrationTest4);

  cout << GeneralProcess(cin);

  return 0;
}
