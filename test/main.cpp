#include <functional>
#include <string>
#include <unordered_map>

#include "integration_tests.h"
#include "test_svg.h"
#include "test_runner.h"

using namespace std;

int main(int argc, char *argv[]) {
  TestRunner tr;
   
  TestSvg::Run(tr);

  if (argc > 1) {
    string test_folder = argv[1];
    // TestIntegration(test_folder, true);
    auto testor = bind(TestIntegration, test_folder, false);
    tr.RunTest(testor, "test from folder: " + test_folder);
  }
  return 0;
}