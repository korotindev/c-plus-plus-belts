#include <memory>
#include <unordered_map>

#include "database.pb.h"


class YellowPagesCatalog {
 public:
  YellowPagesCatalog(YellowPages::Database yellow_pages);

  // FIXME not safe for concurency, think about possible refs invalidation!!
  std::vector<const YellowPages::Company*> Filter(const std::vector<std::string>& names, const std::vector<std::string>& rubrics,
                                  const std::vector<std::string>& urls,
                                  const std::vector<YellowPages::Phone>& phones) const;

  void Serialize(YellowPages::Database &proto) const ;
  static std::unique_ptr<YellowPagesCatalog> Deserialize(YellowPages::Database proto);

 private:
  YellowPagesCatalog() = default;
  std::unordered_map<uint64_t, YellowPages::Rubric> rubrics_;
  std::unordered_map<std::string, uint64_t> reversed_rubrics_index_;
  std::vector<YellowPages::Company> companies_;
};

