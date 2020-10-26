#include <functional>
#include <string>
#include <unordered_map>

#include "integration_tests.h"
#include "rendering_tests.h"
#include "test_runner.h"

using namespace std;

int main(int argc, char *argv[]) {
  TestRunner tr;
  RenderingTests::TestProjector(tr);

  if (argc > 1) {
    string test_folder = argv[1];
    auto testor = bind(TestIntegration, test_folder);
    tr.RunTest(testor, "test from folder: " + test_folder);
  }
  return 0;
}