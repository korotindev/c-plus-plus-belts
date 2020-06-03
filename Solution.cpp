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

void TestParseRequests_withModifyConverter() {
  stringstream input("5\n"
                     "Stop Tolstopaltsevo 1: 55.611087, 37.20829\n"
                     "Stop Tolstopaltsevo 2: 55.611087, 37.20829, 9m to Tolstopaltsevo 1\n"
                     "Stop Tolstopaltsevo 3: 55.611087, 37.20829, 9m to Tolstopaltsevo 2, 1000000m to stop2, 15m to stop 3\n"
                     "Bus 256 2: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Zapadnoye\n"
                     "Bus 750 2: Tolstopaltsevo - Marushkino - Rasskazovka"
  );
  const vector<unique_ptr<Request>> requests = ParseRequests(MODIFY_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 5ul);
  {
    auto request = dynamic_cast<const EntertainStopRequest&>(*requests[0].get());
    ASSERT_EQUAL(request.stopName, "Tolstopaltsevo 1");
    ASSERT(abs(request.latitude - 55.611087) <= 0.0001);
    ASSERT(abs(request.longitude - 37.20829) <= 0.0001);
    ASSERT(request.distanceToOtherStops.empty());
  }
  {
    auto request = dynamic_cast<const EntertainStopRequest&>(*requests[1].get());
    ASSERT_EQUAL(request.stopName, "Tolstopaltsevo 2");
    ASSERT(abs(request.latitude - 55.611087) <= 0.0001);
    ASSERT(abs(request.longitude - 37.20829) <= 0.0001);
    vector<StopDistance> distanceToOtherStops = {{"Tolstopaltsevo 1", 9}};
    ASSERT_EQUAL(request.distanceToOtherStops, distanceToOtherStops);
  }
  {
    auto request = dynamic_cast<const EntertainStopRequest&>(*requests[2].get());
    ASSERT_EQUAL(request.stopName, "Tolstopaltsevo 3");
    ASSERT(abs(request.latitude - 55.611087) <= 0.0001);
    ASSERT(abs(request.longitude - 37.20829) <= 0.0001);
    vector<StopDistance> distanceToOtherStops = {{"Tolstopaltsevo 2", 9},
                                                 {"stop2",            1000000},
                                                 {"stop 3",           15}};
    ASSERT_EQUAL(request.distanceToOtherStops, distanceToOtherStops);
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest&>(*requests[3].get());
    ASSERT_EQUAL(request.busName, "256 2")
    const vector<string> stopsNames = {"Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Zapadnoye"};
    ASSERT_EQUAL(request.stopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest&>(*requests[4].get());
    ASSERT_EQUAL(request.busName, "750 2")
    const vector<string> stopsNames = {"Tolstopaltsevo", "Marushkino", "Rasskazovka", "Marushkino", "Tolstopaltsevo"};
    ASSERT_EQUAL(request.stopsNames, stopsNames)
  }
}

void TestParseRequests_withReadConverter() {
  stringstream input("3\n"
                     "Bus 2 56\n"
                     "Stop 7 50\n"
                     "Bus 751"
  );
  const vector<unique_ptr<Request>> requests = ParseRequests(READ_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 3ul);
  {
    auto request = dynamic_cast<const ReadBusRequest&>(*requests[0].get());
    ASSERT_EQUAL(request.busName, "2 56");
  }
  {
    auto request = dynamic_cast<const ReadStopRequest&>(*requests[1].get());
    ASSERT_EQUAL(request.stopName, "7 50");
  }
  {
    auto request = dynamic_cast<const ReadBusRequest&>(*requests[2].get());
    ASSERT_EQUAL(request.busName, "751");
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

void TestIntegrationPartC() {
  TestIntegrationGenerator(
    (
      "13\n"
      "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
      "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka\n"
      "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
      "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
      "Stop Rasskazovka: 55.632761, 37.333324\n"
      "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
      "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
      "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
      "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
      "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
      "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
      "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
      "Stop Prazhskaya: 55.611678, 37.603831\n"
      "6\n"
      "Bus 256\n"
      "Bus 750\n"
      "Bus 751\n"
      "Stop Samara\n"
      "Stop Prazhskaya\n"
      "Stop Biryulyovo Zapadnoye"
    ),
    (
      "Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature\n"
      "Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.31808 curvature\n"
      "Bus 751: not found\n"
      "Stop Samara: not found\n"
      "Stop Prazhskaya: no buses\n"
      "Stop Biryulyovo Zapadnoye: buses 256 828\n"
    )
  );
}
