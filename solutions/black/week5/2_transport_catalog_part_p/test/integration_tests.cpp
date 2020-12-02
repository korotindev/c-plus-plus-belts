#include "integration_tests.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "json.h"
#include "processes.h"
#include "test_runner.h"

using namespace std;

void TestIntegration(const string &test_data_folder_name) {
  {
    ifstream input(test_data_folder_name + "/make_base.json");
    MakeBase(input);
  }

  ifstream input(test_data_folder_name + "/process_requests.json");
  stringstream output;
  ProcessRequests(input, output);
  Json::Document result_doc = Json::Load(output);
  auto expected_doc_input = ifstream(test_data_folder_name + "/expected_output.json");
  Json::Document expected_doc = Json::Load(expected_doc_input);

  ASSERT_EQUAL(result_doc, expected_doc);
}