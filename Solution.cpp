#include "Solution.h"

using namespace std;

vector<unique_ptr<Request>> ParseRequests(const TypeConverter& converter, istream& input) {
  auto count = ReadNumberOnLine<size_t>(input);
  vector<unique_ptr<Request>> requests;
  requests.reserve(count);
  for (size_t i = 0; i < count; i++) {
    string request_str;
    getline(input, request_str);
    requests.emplace_back(ParseRequest(converter, request_str));
  }
  return requests;
}

void ProcessModifyRequests(Database& db, vector<RequestHolder>& requests) {
  for (const auto& request_holder : requests) {
    auto& request = dynamic_cast<ModifyRequest&>(*request_holder);
    request.Process(db);
  }
}

vector<string> ProcessReadRequests(Database& db, vector<RequestHolder>& requests) {
  vector<string> responses;
  for (const auto& request_holder : requests) {
    auto& request = dynamic_cast<ReadRequest<string>&>(*request_holder);
    responses.push_back(request.Process(db));
  }
  return responses;
}


void PrintResponses(const vector<string>& responses, ostream& stream) {
  for (const auto& response : responses) {
    stream << response << '\n';
  }
}

void TestIntegrationGenerator(string inputText, string expectedText) {
  stringstream input(inputText);
  auto modifyRequests = ParseRequests(MODIFY_TYPES_CONVERTER, input);
  Database db;
  ProcessModifyRequests(db, modifyRequests);
  auto readRequests = ParseRequests(READ_TYPES_CONVERTER, input);
  auto responses = ProcessReadRequests(db, readRequests);
  stringstream output;
  PrintResponses(responses, output);
  ASSERT_EQUAL(output.str(), expectedText);
}

void TestIntegrationPartD() {
  TestIntegrationGenerator(
    (
      "{\n"
      "  \"base_requests\": [\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Marushkino\": 3900\n"
      "      },\n"
      "      \"longitude\": 37.20829,\n"
      "      \"name\": \"Tolstopaltsevo\",\n"
      "      \"latitude\": 55.611087\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Rasskazovka\": 9900\n"
      "      },\n"
      "      \"longitude\": 37.209755,\n"
      "      \"name\": \"Marushkino\",\n"
      "      \"latitude\": 55.595884\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Bus\",\n"
      "      \"name\": \"256\",\n"
      "      \"stops\": [\n"
      "        \"Biryulyovo Zapadnoye\",\n"
      "        \"Biryusinka\",\n"
      "        \"Universam\",\n"
      "        \"Biryulyovo Tovarnaya\",\n"
      "        \"Biryulyovo Passazhirskaya\",\n"
      "        \"Biryulyovo Zapadnoye\"\n"
      "      ],\n"
      "      \"is_roundtrip\": true\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Bus\",\n"
      "      \"name\": \"750\",\n"
      "      \"stops\": [\n"
      "        \"Tolstopaltsevo\",\n"
      "        \"Marushkino\",\n"
      "        \"Rasskazovka\"\n"
      "      ],\n"
      "      \"is_roundtrip\": false\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {},\n"
      "      \"longitude\": 37.333324,\n"
      "      \"name\": \"Rasskazovka\",\n"
      "      \"latitude\": 55.632761\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Rossoshanskaya ulitsa\": 7500,\n"
      "        \"Biryusinka\": 1800,\n"
      "        \"Universam\": 2400\n"
      "      },\n"
      "      \"longitude\": 37.6517,\n"
      "      \"name\": \"Biryulyovo Zapadnoye\",\n"
      "      \"latitude\": 55.574371\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Universam\": 750\n"
      "      },\n"
      "      \"longitude\": 37.64839,\n"
      "      \"name\": \"Biryusinka\",\n"
      "      \"latitude\": 55.581065\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Rossoshanskaya ulitsa\": 5600,\n"
      "        \"Biryulyovo Tovarnaya\": 900\n"
      "      },\n"
      "      \"longitude\": 37.645687,\n"
      "      \"name\": \"Universam\",\n"
      "      \"latitude\": 55.587655\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Biryulyovo Passazhirskaya\": 1300\n"
      "      },\n"
      "      \"longitude\": 37.653656,\n"
      "      \"name\": \"Biryulyovo Tovarnaya\",\n"
      "      \"latitude\": 55.592028\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {\n"
      "        \"Biryulyovo Zapadnoye\": 1200\n"
      "      },\n"
      "      \"longitude\": 37.659164,\n"
      "      \"name\": \"Biryulyovo Passazhirskaya\",\n"
      "      \"latitude\": 55.580999\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Bus\",\n"
      "      \"name\": \"828\",\n"
      "      \"stops\": [\n"
      "        \"Biryulyovo Zapadnoye\",\n"
      "        \"Universam\",\n"
      "        \"Rossoshanskaya ulitsa\",\n"
      "        \"Biryulyovo Zapadnoye\"\n"
      "      ],\n"
      "      \"is_roundtrip\": true\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {},\n"
      "      \"longitude\": 37.605757,\n"
      "      \"name\": \"Rossoshanskaya ulitsa\",\n"
      "      \"latitude\": 55.595579\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"road_distances\": {},\n"
      "      \"longitude\": 37.603831,\n"
      "      \"name\": \"Prazhskaya\",\n"
      "      \"latitude\": 55.611678\n"
      "    }\n"
      "  ],\n"
      "  \"stat_requests\": [\n"
      "    {\n"
      "      \"type\": \"Bus\",\n"
      "      \"name\": \"256\",\n"
      "      \"id\": 1965312327\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Bus\",\n"
      "      \"name\": \"750\",\n"
      "      \"id\": 519139350\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Bus\",\n"
      "      \"name\": \"751\",\n"
      "      \"id\": 194217464\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"name\": \"Samara\",\n"
      "      \"id\": 746888088\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"name\": \"Prazhskaya\",\n"
      "      \"id\": 65100610\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"Stop\",\n"
      "      \"name\": \"Biryulyovo Zapadnoye\",\n"
      "      \"id\": 1042838872\n"
      "    }\n"
      "  ]\n"
      "}"
    ),
    (
      "[\n"
      "  {\n"
      "    \"route_length\": 5950,\n"
      "    \"request_id\": 1965312327,\n"
      "    \"curvature\": 1.36124,\n"
      "    \"stop_count\": 6,\n"
      "    \"unique_stop_count\": 5\n"
      "  },\n"
      "  {\n"
      "    \"route_length\": 27600,\n"
      "    \"request_id\": 519139350,\n"
      "    \"curvature\": 1.31808,\n"
      "    \"stop_count\": 5,\n"
      "    \"unique_stop_count\": 3\n"
      "  },\n"
      "  {\n"
      "    \"request_id\": 194217464,\n"
      "    \"error_message\": \"not found\"\n"
      "  },\n"
      "  {\n"
      "    \"request_id\": 746888088,\n"
      "    \"error_message\": \"not found\"\n"
      "  },\n"
      "  {\n"
      "    \"buses\": [],\n"
      "    \"request_id\": 65100610\n"
      "  },\n"
      "  {\n"
      "    \"buses\": [\n"
      "      \"256\",\n"
      "      \"828\"\n"
      "    ],\n"
      "    \"request_id\": 1042838872\n"
      "  }\n"
      "]"
    )
  );
}
