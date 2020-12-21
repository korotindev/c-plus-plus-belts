#include "yellow_pages_catalog.h"
using namespace std;

static bool MatchNames(const YellowPages::Company& company, const vector<string>& names) {
  if (names.empty()) {
    return true;
  }
  // think about unordered_map_of_names but where should I build it?
  for (const auto& name : company.names()) {
    for (const auto& other : names) {
      if (name.value() == other) {
        return true;
      }
    }
  }
  return false;
}

static bool MatchRubrics(const YellowPages::Company& company, const vector<uint64_t>& rubrics_ids) {
  if (rubrics_ids.empty()) {
    return true;
  }
  // think about unordered_map_of_names but where should I build it?
  for (const auto& rubric_id : company.rubrics()) {
    for (const auto& other : rubrics_ids) {
      if (rubric_id == other) {
        return true;
      }
    }
  }
  return false;
}

static bool MatchUrls(const YellowPages::Company& company, const vector<string>& urls) {
  if (urls.empty()) {
    return true;
  }
  // think about unordered_map_of_names but where should I build it?
  for (const auto& url : company.urls()) {
    for (const auto& other : urls) {
      if (url.value() == other) {
        return true;
      }
    }
  }
  return false;
}

static bool MatchPhone(const YellowPages::Phone& object, const YellowPages::Phone& phone_template) {
  if (!phone_template.extension().empty() && phone_template.extension() != object.extension()) {
    return false;
  }
  if (phone_template.type() != YellowPages::Phone_Type::Phone_Type_UNKNOWN && phone_template.type() != object.type()) {
    return false;
  }
  if (!phone_template.country_code().empty() && phone_template.country_code() != object.country_code()) {
    return false;
  }
  if ((!phone_template.local_code().empty() || !phone_template.country_code().empty()) &&
      phone_template.local_code() != object.local_code()) {
    return false;
  }
  return phone_template.number() == object.number();
}

static bool MatchPhones(const YellowPages::Company& company, const vector<YellowPages::Phone>& phones) {
  if (phones.empty()) {
    return true;
  }
  // think about unordered_map_of_names but where should I build it?
  for (const auto& phone : company.phones()) {
    for (const auto& other : phones) {
      if (MatchPhone(phone, other)) {
        return true;
      }
    }
  }
  return false;
}

vector<const YellowPages::Company*> YellowPagesCatalog::FindCompanies(const CompaniesFilter& filter) const {
  vector<const YellowPages::Company*> items;
  vector<uint64_t> rubrics_ids;

  for (auto& str : filter.rubrics) {
    rubrics_ids.push_back(reversed_rubrics_index_.at(str));
  }
  for (const auto& company : companies_) {
    bool ok = MatchNames(company, filter.names);
    ok = ok && MatchRubrics(company, rubrics_ids);
    ok = ok && MatchUrls(company, filter.urls);
    ok = ok && MatchPhones(company, filter.phones);
    if (ok) {
      items.push_back(&company);
    }
  }
  return items;
}

YellowPagesCatalog::YellowPagesCatalog(YellowPages::Database proto) {
  companies_.reserve(proto.companies_size());
  for (auto& company : *proto.mutable_companies()) {
    companies_.push_back(move(company));
  }

  for (auto& map_pair : *proto.mutable_rubrics()) {
    reversed_rubrics_index_[map_pair.second.name()] = map_pair.first;
    rubrics_[map_pair.first] = move(map_pair.second);
  }
}

void YellowPagesCatalog::Serialize(YellowPages::Database& proto) const {
  for (const auto& company : companies_) {
    *proto.add_companies() = company;
  }

  for (const auto& [id, rubric] : rubrics_) {
    (*proto.mutable_rubrics())[id] = rubric;
  }
}

std::unique_ptr<YellowPagesCatalog> YellowPagesCatalog::Deserialize(YellowPages::Database proto) {
  return make_unique<YellowPagesCatalog>(move(proto));
}