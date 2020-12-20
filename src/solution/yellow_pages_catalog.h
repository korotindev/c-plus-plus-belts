#pragma once 
#include <memory>
#include <unordered_map>

#include "database.pb.h"
#include "filters.h"

class YellowPagesCatalog {
 public:
  YellowPagesCatalog(YellowPages::Database yellow_pages);

  // FIXME not safe for concurency, think about possible refs invalidation!!
  std::vector<const YellowPages::Company*> FindCompanies(const CompaniesFilter &filter) const;

  void Serialize(YellowPages::Database &proto) const ;
  static std::unique_ptr<YellowPagesCatalog> Deserialize(YellowPages::Database proto);

 private:
  YellowPagesCatalog() = default;
  std::unordered_map<uint64_t, YellowPages::Rubric> rubrics_;
  std::unordered_map<std::string, uint64_t> reversed_rubrics_index_;
  std::vector<YellowPages::Company> companies_;
};
