#include "ReadResponse.h"

using namespace std;

ReadBusResponse::ReadBusResponse(string busName_) : busName(busName_) {}

void ReadNoBusResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": not found";
}

void ReadBusMetricsResponse::Print(std::ostream& output) {
  output << "Bus " << busName << ": "
         << stopsCount << " stops on route" << ", "
         << uniqueStopsCount << " unique stops" << ", "
         << routeDistance << " route length";
}

ReadStopResponse::ReadStopResponse(string stopName) : stopName(std::move(stopName)) {}

void ReadNoStopResponse::Print(ostream& output) {
  output << "Stop " << stopName << ": not found";
}

void ReadStopMetricsResponse::Print(ostream& output) {
  output << "Stop " << stopName << ":";

  if (buses.empty()) {
    output << " no buses";
  } else {
    output << " buses";
    for(const auto& bus : buses) {
      output << " " << bus;
    }
  }
}
