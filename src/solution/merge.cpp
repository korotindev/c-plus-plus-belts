#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>
#include <google/protobuf/util/message_differencer.h>

#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

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

void ProcessSingularField(Company &company, const Company &sig, const uint32_t priority, const string &field_name,
                          unordered_map<string, uint32_t> &last_priorities) {
  const google::protobuf::Descriptor *descriptor = Company::GetDescriptor();
  const google::protobuf::FieldDescriptor *field = descriptor->FindFieldByName(field_name);
  const google::protobuf::Reflection *reflection = Company::GetReflection();

  if (reflection->HasField(sig, field) && priority >= last_priorities[field_name]) {
    auto mutable_company_field_message = reflection->MutableMessage(&company, field);
    const auto &signal_field_message = reflection->GetMessage(sig, field);

    if (priority > last_priorities[field_name]) {
      mutable_company_field_message->Clear();
      last_priorities[field_name] = priority;
    }

    mutable_company_field_message->CopyFrom(signal_field_message);
  }
}

class CompanySignalMerger {
  using Field = const google::protobuf::FieldDescriptor *;

  Company company;
  unordered_map<string, uint32_t> last_priorities;

 public:
  Company &&TakeResult() { return move(company); }

  void AddCompanySignal(const Company &dirty, uint32_t priority) {
    for (const auto &field_name : {"address", "working_time"}) {
      ProcessSingularField(dirty, priority, field_name, last_priorities[field_name]);
    }

    ProcessRepeatedField<Name>(dirty, priority, "names", last_priorities["names"]);
    ProcessRepeatedField<Phone>(dirty, priority, "phones", last_priorities["phones"]);
    ProcessRepeatedField<Url>(dirty, priority, "urls", last_priorities["urls"]);
  }

 private:
  void ProcessSingularField(const Company &sig, const uint32_t priority, const string &field_name,
                            uint32_t &last_priority) {
    auto reflection = Company::GetReflection();
    auto descriptor = Company::GetDescriptor();
    auto field = descriptor->FindFieldByName(field_name);

    if (reflection->HasField(sig, field) && priority >= last_priority) {
      if (priority > last_priority) {
        reflection->ClearField(&company, field);
        last_priority = priority;
      }

      const auto &signal_field_message = reflection->GetMessage(sig, field);
      auto mutable_company_field_message = reflection->MutableMessage(&company, field);

      mutable_company_field_message->CopyFrom(signal_field_message);
    }
  }

  template <class PB>
  void ProcessRepeatedField(const Company &sig, const uint32_t priority, const string &field_name,
                            uint32_t &last_priority) {
    auto reflection = Company::GetReflection();
    auto descriptor = Company::GetDescriptor();
    auto field = descriptor->FindFieldByName(field_name);

    if (reflection->FieldSize(sig, field) > 0 && priority >= last_priority) {
      if (priority > last_priority) {
        reflection->ClearField(&company, field);
        last_priority = priority;
      }

      using namespace google::protobuf;

      const auto &signal_repeated_field_message = reflection->GetRepeatedPtrField<PB>(sig, field);
      auto mutable_company_repeated_field_message = reflection->MutableRepeatedPtrField<PB>(&company, field);

      mutable_company_repeated_field_message->MergeFrom(signal_repeated_field_message);
      DeduplicateMessages(*mutable_company_repeated_field_message);
    }
  }
};

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

    ProcessSingularField(company, company_signal, provider_priority, "address", last_priorities);
    ProcessSingularField(company, company_signal, provider_priority, "working_time", last_priorities);

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
