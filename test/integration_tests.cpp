#include "integration_tests.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "commands.h"
#include "json.h"
#include "test_runner.h"

using namespace std;

void DrawResults(const Json::Document &doc, filesystem::path &dir, string filename_part) {
  for (const auto &answer : doc.GetRoot().AsArray()) {
    if (answer.IsMap()) {
      const auto &answer_map = answer.AsMap();
      if (answer_map.count("map")) {
        const string &rendered_map = answer_map.at("map").AsString();
        string filename = to_string(answer_map.at("request_id").AsInt()) + "_" + filename_part + ".svg";
        ofstream out(dir / filename);
        out << rendered_map;
      }
    }
  }
}

void TestIntegration(const string &test_data_folder_name, bool draw_results = false) {
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

  if (draw_results) {
    filesystem::path dir = filesystem::current_path() / ".svg_test_results";
    filesystem::remove_all(dir);
    filesystem::create_directory(dir);
    DrawResults(result_doc, dir, "result");
    DrawResults(expected_doc, dir, "expected");
  }

  ASSERT_EQUAL(result_doc, expected_doc);
}
