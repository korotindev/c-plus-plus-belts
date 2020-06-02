#include "test_runner.h"
#include "profile.h"
#include <iostream>
#include <sstream>
#include <iostream>
#include "Request.h"
#include "CustomUtils.h"

using namespace std;

vector<unique_ptr<Request>> ParseRequests(const TypeConverter &converter, istream& input) {
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

void TestParseRequests_withModifyConverter() {
  stringstream input("3\n"
                     "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                     "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                     "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"
  );
  const vector<unique_ptr<Request>> requests = ParseRequests(MODIFY_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 3ul);
  {
    auto request = dynamic_cast<const EntertainStopRequest&>(*requests[0].get());
    ASSERT_EQUAL(request.StopName, "Tolstopaltsevo");
    ASSERT(abs(request.Latitude - 55.611087) <= 0.0001);
    ASSERT(abs(request.Longitude - 37.20829) <= 0.0001);
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest&>(*requests[1].get());
    ASSERT_EQUAL(request.BusName, "256")
    const vector<string> stopsNames = {"Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Tovarnaya",
                                       "Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
  {
    auto request = dynamic_cast<const EntertainBusRequest&>(*requests[2].get());
    ASSERT_EQUAL(request.BusName, "750")
    const vector<string> stopsNames = {"Tolstopaltsevo", "Marushkino", "Rasskazovka", "Marushkino", "Tolstopaltsevo"};
    ASSERT_EQUAL(request.StopsNames, stopsNames)
  }
}

void TestParseRequests_withReadConverter() {
  stringstream input("3\n"
                     "Bus 256\n"
                     "Bus 750\n"
                     "Bus 751"
  );
  const vector<unique_ptr<Request>> requests = ParseRequests(READ_TYPES_CONVERTER, input);
  ASSERT_EQUAL(requests.size(), 3ul);
  {
    auto request = dynamic_cast<const ReadBusRequest&>(*requests[0].get());
    ASSERT_EQUAL(request.BusName, "256");
  }
  {
    auto request = dynamic_cast<const ReadBusRequest&>(*requests[1].get());
    ASSERT_EQUAL(request.BusName, "750");
  }
  {
    auto request = dynamic_cast<const ReadBusRequest&>(*requests[2].get());
    ASSERT_EQUAL(request.BusName, "751");
  }
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestParseRequests_withModifyConverter);
  RUN_TEST(tr, TestParseRequests_withReadConverter);
  return 0;
}
