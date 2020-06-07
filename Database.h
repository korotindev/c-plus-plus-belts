#ifndef C_PLUS_PLUS_BELTS_DATABASE_H
#define C_PLUS_PLUS_BELTS_DATABASE_H

#include "ReadResponse.h"
#include "Coordinate.h"
#include "Router.h"
#include "StopsStorage.h"
#include "BusStorage.h"

class Database {
  StopsStorage stopsStorage;
  BusStorage busStorage;

public:
  void EntertainStop(Stop stop);
  void EntertainBus(Bus bus);
  std::unique_ptr<ReadBusResponse> ReadBus(const std::string& busName, const size_t requestId);
  std::unique_ptr<ReadStopResponse> ReadStop(const std::string& stopName, const size_t requestId);
};

#endif //C_PLUS_PLUS_BELTS_DATABASE_H
