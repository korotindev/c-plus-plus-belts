//
// Created by Дмитрий Коротин on 07.06.2020.
//

#include "Settings.h"

using namespace std;

unique_ptr<Settings> Settings::instance = nullptr;

size_t Settings::GetBusWaitTime() { return instance->busWaitTime; }

size_t Settings::GetBusVelocity() { return instance->busVelocity; }

void Settings::InitializeFrom(const Json::Node &node) {
  const auto &values = node.AsMap();
  instance = make_unique<Settings>();

  instance->busVelocity = static_cast<size_t>(values.at("bus_velocity").AsNumber());
  instance->busWaitTime = static_cast<size_t>(values.at("bus_wait_time").AsNumber());
}