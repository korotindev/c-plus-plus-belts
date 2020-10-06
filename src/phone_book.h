#pragma once

#include "iterator_range.h"
#include "contact.pb.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iosfwd>

struct Date {
  int year, month, day;
};

struct Contact {
  std::string name;
  std::optional<Date> birthday;
  std::vector<std::string> phones;
};

class PhoneBook {
  std::vector<Contact> contacts_;
public:
  using ContactRange = IteratorRange<std::vector<Contact>::const_iterator>;
  explicit PhoneBook(std::vector<Contact> contacts) : contacts_(std::move(contacts)) {}

  ContactRange FindByNamePrefix(std::string_view) const {
    // FIXME
    return IteratorRange(contacts_.cbegin(), contacts_.cend());
  }

  void SaveTo(std::ostream& ) const {
    
  }
};

PhoneBook DeserializePhoneBook(std::istream&) {
  return PhoneBook({});
}
