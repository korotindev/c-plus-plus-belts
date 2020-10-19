#pragma once

#include <unordered_map>
#include <vector>

#include "company.pb.h"
#include "name.pb.h"
#include "phone.pb.h"
#include "provider.pb.h"
#include "signal.pb.h"
#include "url.pb.h"

namespace YellowPages {
  using Signals = std::vector<Signal>;
  using Providers = std::unordered_map<uint64_t, Provider>;

  // Объединяем данные из сигналов об одной и той же организации
  Company Merge(const Signals& signals, const Providers& providers);

}  // namespace YellowPages
