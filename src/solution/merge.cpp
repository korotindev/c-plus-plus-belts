#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <google/protobuf/util/message_differencer.h>

#include "yellow_pages.h"

namespace YellowPages {

using namespace std;

template <typename Message>
bool operator==(const Message &m1, const Message &m2) {
  return google::protobuf::util::MessageDifferencer::Equals(m1, m2);
}

template <typename Message>
bool operator<(const Message &m1, const Message &m2) {
  return m1.SerializeAsString() < m2.SerializeAsString();
}

template <typename RepeatedMessageArr>
void DeduplicateMessages(RepeatedMessageArr &arr) {
  sort(arr.begin(), arr.end());
  auto last = unique(arr.begin(), arr.end());
  arr.erase(last, arr.end());
}

Company Merge(const Signals &signals, const Providers &providers) {
  if (signals.empty()) {
    return {};
  }
  Company company;

  unordered_map<string, uint32_t> last_priorities;

  for (const auto &signal : signals) {
    if (!signal.has_company()) {
      continue;
    }

    const auto provider_priority = providers.at(signal.provider_id()).priority();
    const auto &company_signal = signal.company();

    if (company_signal.has_address() && provider_priority >= last_priorities["address"]) {
      if (provider_priority > last_priorities["address"]) {
        company.mutable_address()->Clear();
        last_priorities["address"] = provider_priority;
      }

      company.mutable_address()->CopyFrom(company_signal.address());
    }

    if (company_signal.has_working_time() && provider_priority >= last_priorities["working_time"]) {
      if (provider_priority > last_priorities["working_time"]) {
        company.mutable_working_time()->Clear();
        last_priorities["working_time"] = provider_priority;
      }

      company.mutable_working_time()->CopyFrom(company_signal.working_time());
    }

    if (company_signal.urls_size() > 0 && provider_priority >= last_priorities["urls"]) {
      if (provider_priority > last_priorities["urls"]) {
        company.mutable_urls()->Clear();
        last_priorities["urls"] = provider_priority;
      }

      auto &urls = *company.mutable_urls();
      urls.MergeFrom(company_signal.urls());
      DeduplicateMessages(urls);
    }

    if (company_signal.phones_size() > 0 && provider_priority >= last_priorities["phones"]) {
      if (provider_priority > last_priorities["phones"]) {
        company.mutable_phones()->Clear();
        last_priorities["phones"] = provider_priority;
      }

      auto &phones = *company.mutable_phones();
      phones.MergeFrom(company_signal.phones());
      DeduplicateMessages(phones);
    }

    if (company_signal.names_size() > 0 && provider_priority >= last_priorities["names"]) {
      if (provider_priority > last_priorities["names"]) {
        company.mutable_names()->Clear();
        last_priorities["names"] = provider_priority;
      }

      auto &names = *company.mutable_names();
      names.MergeFrom(company_signal.names());
      DeduplicateMessages(names);
    }
  }

  return company;
}

}  // namespace YellowPages
