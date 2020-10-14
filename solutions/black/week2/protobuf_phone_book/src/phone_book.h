#pragma once

#include <algorithm>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "contact.pb.h"
#include "iterator_range.h"

struct Date {
  int year, month, day;
  PhoneBookSerialize::Date Serialize() const;
  static Date Deserealize(PhoneBookSerialize::Date &serialized_date);
};

struct Contact {
  std::string name;
  std::optional<Date> birthday;
  std::vector<std::string> phones;
  PhoneBookSerialize::Contact Serialize() const;
  static Contact Deserealize(PhoneBookSerialize::Contact &serialized_contact);
};

bool operator<(const Contact &lhs, const Contact &rhs);

class PhoneBook {
  std::vector<Contact> contacts_;

 public:
  using ContactRange = IteratorRange<std::vector<Contact>::const_iterator>;

  explicit PhoneBook(std::vector<Contact> contacts);

  ContactRange FindByNamePrefix(std::string_view) const;

  void SaveTo(std::ostream &out) const;
};

PhoneBook DeserializePhoneBook(std::istream &in);
