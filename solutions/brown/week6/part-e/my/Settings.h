//
// Created by Дмитрий Коротин on 07.06.2020.
//

#ifndef C_PLUS_PLUS_BELTS_SETTINGS_H
#define C_PLUS_PLUS_BELTS_SETTINGS_H

#include <memory>
#include "Json.h"

class Settings {
private:
  size_t busVelocity;
  size_t busWaitTime;
public:
  static void InitializeFrom(const Json::Node& node);
  static size_t GetBusWaitTime();
  static size_t GetBusVelocity();
  static std::unique_ptr<Settings> instance;
};

#endif //C_PLUS_PLUS_BELTS_SETTINGS_H
