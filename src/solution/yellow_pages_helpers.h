#pragma once

#include <string>

#include "database.pb.h"
#include "sphere.h"

const std::string &GetMainCompanyName(const YellowPages::Company& company);
std::string GetFullCompanyName(const YellowPages::Database& database, const YellowPages::Company& company);