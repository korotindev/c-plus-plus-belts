#include "Database.h"

using namespace std;

void Database::EntertainStop(Stop stop) {
  stopsStorage.Add(move(stop));
}

void Database::EntertainBus(Bus bus) {
  for (auto& stopName : bus.stopsNames) {
    stopsStorage.AddBusToStop(stopName, bus.name);
  }
  busStorage.Add(move(bus));
}

unique_ptr<ReadBusResponse> Database::ReadBus(const std::string& busName, const size_t requestId) {
  if (!busStorage.Exist(busName)) {
    return make_unique<ReadNoBusResponse>(busName, requestId);
  }

  const auto& stops = busStorage.GetStops(busName);
  auto response = make_unique<ReadBusMetricsResponse>(busName, requestId);

  response->stopsCount = stops.size();
  response->uniqueStopsCount = busStorage.GetUniqueStopsCount(busName);
  response->routeDistance = 0.0;
  response->routeDistanceV2 = 0.0;
  for (size_t i = 1; i < stops.size(); i++) {
    response->routeDistance += stopsStorage.GetDistance(stops[i - 1], stops[i]);
    response->routeDistanceV2 += stopsStorage.GetDistanceV2(stops[i - 1], stops[i]);
  }
  return response;
}

std::unique_ptr<ReadStopResponse> Database::ReadStop(const string& stopName, const size_t requestId) {
  if (!stopsStorage.Exist(stopName)) {
    return make_unique<ReadNoStopResponse>(stopName, requestId);
  }

  auto response = make_unique<ReadStopMetricsResponse>(stopName, requestId);
  const auto& sortedBuses = stopsStorage.GetBuses(stopName);
  response->buses = vector<string>(sortedBuses.begin(), sortedBuses.end());
  return response;
}

void Database::ReadRoute(const size_t, const std::string& from, const std::string& to) {
  stopsStorage.ReadRoute(from, to);

  // При построении маршрута надо не забыть проверить интересный кейс:
  //   Если роутер говорит, что путь должен быть проложен из А в Б,
  //   то возможно мы должны попробовать найти автобус из Б в А,
  //   т.к. роутер мог сделать разворот на той же автобусной линии
  //   в таком случае надо подождать на остановке следующего автобуса и прыгнуть на него

  // В целом надо ехать на автобусном маршруте по текущему пути до тех пор, пока можно,
  //   потом - выбирать следующий автобус и ехать на нём до тех пор пока можно.

  // Сейчас граф построен просто по карте дорог, надо ещё попробовать научить алгоритм поиска быть несколько жадным
  //   и не пересаживаться лишний раз, но тогда надо понять как работает алгоритм и попробовать адаптировать веса рёбер.
  // Сама по себе идея кажется не очень умной, т.к. для разных стартовых точек веса одних и тех же рёбер должны быть разными.

  // Итого: на данный момент роутер уменьшает количество пройденного растояния, но не количество пересадок.
  // Задача - придумать алгоритм уменьшения пересадок, ну или забить хер и всегда пытаться ехать по прошлому автобусу.
}

void Database::BuildRouter() {
  stopsStorage.BuildRouter();
}
