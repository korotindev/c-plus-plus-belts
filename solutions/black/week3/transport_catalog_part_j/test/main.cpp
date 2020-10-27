#include <functional>
#include <string>
#include <unordered_map>

#include "tests.h"
#include "test_runner.h"

using namespace std;

int main(int argc, char *argv[]) {
  TestRunner tr;

  if (argc == 1) {
    cout << "Provide test folder path";
    return 1;
  } else {
    string test_folder = argv[1];
    auto testor = bind(TestIntegration, test_folder);
    tr.RunTest(testor, "test from folder: " + test_folder);
  }
  return 0;
}