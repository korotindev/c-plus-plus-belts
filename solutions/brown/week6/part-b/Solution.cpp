#include "Solution.h"

using namespace std;

vector<unique_ptr<Request>> ParseRequests(const TypeConverter &converter, istream &input) {
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

void ProcessModifyRequests(Database &db, vector<RequestHolder> &requests) {
  for (const auto &request_holder : requests) {
    auto &request = dynamic_cast<ModifyRequest &>(*request_holder);
    request.Process(db);
  }
}

vector<string> ProcessReadRequests(Database &db, vector<RequestHolder> &requests) {
  vector<string> responses;
  for (const auto &request_holder : requests) {
    auto &request = dynamic_cast<ReadRequest<string> &>(*request_holder);
    responses.push_back(request.Process(db));
  }
  return responses;
}

void PrintResponses(const vector<string> &responses, ostream &stream) {
  for (const auto &response : responses) {
    stream << response << '\n';
  }
}

void TestParseRequests_withModifyConverter() {
  stringstream input("3\n"
                     "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                     "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo "
                     "Passazhirskaya > Biryulyovo Zapadnoye\n"
                     "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka");
  const vector<unique_ptr<Request>> requests = ParseRequests(MODIFY_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 3ul);
  {
    auto request = dynamic_cast<const EntertainStopRequest &>(*requests[0].get());
    ASSERT_EQUAL(request.StopName, "Tolstopaltsevo");
    ASSERT(abs(request.Latitude - 55.611087) <= 0.0001);
    ASSERT(abs(request.Longitude - 37.20829) <= 0.0001);
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest &>(*requests[1].get());
    ASSERT_EQUAL(request.BusName, "256")
    const vector<string> stopsNames = {
        "Biryulyovo Zapadnoye",      "Biryusinka",          "Universam", "Biryulyovo Tovarnaya",
        "Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest &>(*requests[2].get());
    ASSERT_EQUAL(request.BusName, "750")
    const vector<string> stopsNames = {"Tolstopaltsevo", "Marushkino", "Rasskazovka", "Marushkino", "Tolstopaltsevo"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
}

void TestParseRequests_withModifyConverter2() {
  stringstream input("4\n"
                     "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo "
                     "Passazhirskaya > Biryulyovo Zapadnoye\n"
                     "Bus lw5PH5: qwe > eee > qwe\n"
                     "Bus lw5PH6: qwe - eee\n"
                     "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo "
                     "Passazhirskaya > Biryulyovo Zapadnoye\n");
  const vector<unique_ptr<Request>> requests = ParseRequests(MODIFY_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 4ul);
  {
    auto request = dynamic_cast<const EntertainBusRequest &>(*requests[0].get());
    ASSERT_EQUAL(request.BusName, "256")
    const vector<string> stopsNames = {
        "Biryulyovo Zapadnoye",      "Biryusinka",          "Universam", "Biryulyovo Tovarnaya",
        "Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest &>(*requests[1].get());
    ASSERT_EQUAL(request.BusName, "lw5PH5")
    const vector<string> stopsNames = {"qwe", "eee", "qwe"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest &>(*requests[2].get());
    ASSERT_EQUAL(request.BusName, "lw5PH6")
    const vector<string> stopsNames = {"qwe", "eee", "qwe"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest &>(*requests[3].get());
    ASSERT_EQUAL(request.BusName, "256")
    const vector<string> stopsNames = {
        "Biryulyovo Zapadnoye",      "Biryusinka",          "Universam", "Biryulyovo Tovarnaya",
        "Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
}

void TestParseRequests_withReadConverter() {
  stringstream input("3\n"
                     "Bus 2 56\n"
                     "Stop 7 50\n"
                     "Bus 751");
  const vector<unique_ptr<Request>> requests = ParseRequests(READ_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 3ul);
  {
    auto request = dynamic_cast<const ReadBusRequest &>(*requests[0].get());
    ASSERT_EQUAL(request.BusName, "2 56");
  }
  {
    auto request = dynamic_cast<const ReadStopRequest &>(*requests[1].get());
    ASSERT_EQUAL(request.StopName, "7 50");
  }
  {
    auto request = dynamic_cast<const ReadBusRequest &>(*requests[2].get());
    ASSERT_EQUAL(request.BusName, "751");
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

void TestIntegrationPartA() {
  TestIntegrationGenerator(("15\n"
                            "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                            "Stop Marushkino: 55.595884, 37.209755\n"
                            "Stop qwe: 55.611087, 37.20829\n"
                            "Stop eee: 55.595884, 37.209755\n"
                            "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > "
                            "Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                            "Bus lw5PH5 second: qwe > eee > qwe\n"
                            "Bus lw5PH6: qwe - eee\n"
                            "Bus lw5PH7: \n"
                            "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                            "Stop Rasskazovka: 55.632761, 37.333324\n"
                            "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
                            "Stop Biryusinka: 55.581065, 37.64839\n"
                            "Stop Universam: 55.587655, 37.645687\n"
                            "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
                            "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
                            "7\n"
                            "Bus 256\n"
                            "Bus 750\n"
                            "Bus 752\n"
                            "Bus lw5PH5 second\n"
                            "Bus lw5PH6\n"
                            "Bus lw5PH7\n"
                            "Bus 751 1"),
                           ("Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length\n"
                            "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n"
                            "Bus 752: not found\n"
                            "Bus lw5PH5 second: 3 stops on route, 2 unique stops, 3386 route length\n"
                            "Bus lw5PH6: 3 stops on route, 2 unique stops, 3386 route length\n"
                            "Bus lw5PH7: 0 stops on route, 0 unique stops, 0 route length\n"
                            "Bus 751 1: not found\n"));
}

void TestIntegrationPartB() {
  TestIntegrationGenerator(("4\n"
                            "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > "
                            "Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                            "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
                            "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
                            "Stop Prazhskaya: 55.611678, 37.603831\n"
                            "3\n"
                            "Stop Samara\n"
                            "Stop Prazhskaya\n"
                            "Stop Biryulyovo Zapadnoye"),
                           ("Stop Samara: not found\n"
                            "Stop Prazhskaya: no buses\n"
                            "Stop Biryulyovo Zapadnoye: buses 256 828\n"));

  TestIntegrationGenerator(("13\n"
                            "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                            "Stop Marushkino: 55.595884, 37.209755\n"
                            "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > "
                            "Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                            "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                            "Stop Rasskazovka: 55.632761, 37.333324\n"
                            "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
                            "Stop Biryusinka: 55.581065, 37.64839\n"
                            "Stop Universam: 55.587655, 37.645687\n"
                            "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
                            "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
                            "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
                            "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
                            "Stop Prazhskaya: 55.611678, 37.603831\n"
                            "6\n"
                            "Bus 256\n"
                            "Bus 750\n"
                            "Bus 751\n"
                            "Stop Samara\n"
                            "Stop Prazhskaya\n"
                            "Stop Biryulyovo Zapadnoye"),
                           ("Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length\n"
                            "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n"
                            "Bus 751: not found\n"
                            "Stop Samara: not found\n"
                            "Stop Prazhskaya: no buses\n"
                            "Stop Biryulyovo Zapadnoye: buses 256 828\n"));
}
