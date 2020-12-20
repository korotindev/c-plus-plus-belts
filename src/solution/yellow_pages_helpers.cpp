#include "yellow_pages_helpers.h"

using namespace std;


const std::string &GetMainCompanyName(const YellowPages::Company& company) {
  const auto& names = company.names();
  return find_if(names.begin(), names.end(),
                 [](const YellowPages::Name& name) { return name.type() == YellowPages::Name_Type::Name_Type_MAIN; })
      ->value();
}

std::string GetFullCompanyName(const YellowPages::Database& database, const YellowPages::Company& company) {
  const string &name = GetMainCompanyName(company);
  if (company.rubrics_size() > 0) {
    return database.rubrics().at(company.rubrics()[0]).name() + name;
  }

  return name;
}