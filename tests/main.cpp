#include "tests.h"
#include "utils/test_runner.h"

#include <functional>
#include <string>
#include <unordered_map>

using namespace std;

const static unordered_map<string, function<void()>> tests = {
    {"1", TestIntegrationTest1},
    {"2", TestIntegrationTest2},
    {"3", TestIntegrationTest3},
    {"4", TestIntegrationTest4},
};

int main(int argc, char *argv[]) {
  TestRunner tr;

  if (argc == 1) {
    for (auto &[name, test] : tests) {
        tr.RunTest(test, "test " + name);
    }
  } else {
    if (auto it = tests.find(argv[1]); it != tests.end()) {
      tr.RunTest(it->second, "test " + it->first);
    } else {
      cerr << "Test not found";
      return 1;
    }
  }
  return 0;
}


  // template <>
  // void PrintValue<Document>(const Document& doc, std::ostream& output) {
  //   PrintNode(doc.GetRoot(), output);
  // }

  // void PrintNode(const Json::Node& node, ostream& output) {
  //   visit([&output](const auto& value) { PrintValue(value, output); },
  //         node.GetBase());
  // }

  // std::ostream &operator<<(std::ostream &output, const Document &rhs) {
  //   PrintValue(rhs, output);
  //   return output;
  // }

  // bool operator==(const Document &lhs, const Document &rhs) {
  //   stringstream output_lhs, output_rhs;
  //   PrintValue(lhs, output_lhs);
  //   PrintValue(rhs, output_rhs);
  //   return output_lhs.str() == output_rhs.str();
  // }