#include "phone_book.h"

using namespace std;

PhoneBookSerialize::Date Date::Serialize() const {
  PhoneBookSerialize::Date date;
  date.set_day(day);
  date.set_month(month);
  date.set_year(year);
  return date;
}

Date Date::Deserealize(PhoneBookSerialize::Date &serialized_date) {
  return Date{.year = serialized_date.year(), .month = serialized_date.month(), .day = serialized_date.day()};
}

Contact Contact::Deserealize(PhoneBookSerialize::Contact &serialized_contact) {
  Contact contact;
  contact.name = move(*serialized_contact.mutable_name());
  if (serialized_contact.has_birthday()) {
    contact.birthday = Date::Deserealize(*serialized_contact.mutable_birthday());
  }

  if (serialized_contact.phone_number_size() > 0) {
    auto &container = *serialized_contact.mutable_phone_number();
    contact.phones.assign(move_iterator(container.begin()), move_iterator(container.end()));
  }
  return contact;
}

bool operator<(const Contact &lhs, const Contact &rhs) { return lhs.name < rhs.name; }

PhoneBook::PhoneBook(vector<Contact> contacts) : contacts_(move(contacts)) { sort(contacts_.begin(), contacts_.end()); }

PhoneBook::ContactRange PhoneBook::FindByNamePrefix(string_view str) const {
  if (str.empty()) {
    return ContactRange(contacts_.cbegin(), contacts_.cend());
  }

  Contact tmp;

  tmp.name = string(str);

  auto p = equal_range(contacts_.cbegin(), contacts_.cend(), tmp, [str](const Contact &lhs, const Contact &rhs) {
    string_view lhs_sv = lhs.name;
    if (lhs_sv.size() > str.size()) {
      lhs_sv.remove_suffix(lhs_sv.size() - str.size());
    }
    string_view rhs_sv = rhs.name;
    if (rhs_sv.size() > str.size()) {
      rhs_sv.remove_suffix(rhs_sv.size() - str.size());
    }
    return lhs_sv < rhs_sv;
  });

  return ContactRange(p.first, p.second);
}

PhoneBookSerialize::Contact Contact::Serialize() const {
  PhoneBookSerialize::Contact contact;
  contact.set_name(name);
  if (birthday) {
    *contact.mutable_birthday() = birthday->Serialize();
  }
  for (const auto &number : phones) {
    contact.add_phone_number(number);
  }
  return contact;
}

void PhoneBook::SaveTo(ostream &out) const {
  PhoneBookSerialize::ContactList list;

  for (auto &contact : contacts_) {
    *list.add_contact() = contact.Serialize();
  }

  list.SerializeToOstream(&out);
}

PhoneBook DeserializePhoneBook(istream &in) {
  PhoneBookSerialize::ContactList list;
  list.ParseFromIstream(&in);
  vector<Contact> contacts;
  for (auto &serialized_contact : *list.mutable_contact()) {
    contacts.push_back(Contact::Deserealize(serialized_contact));
  }
  return PhoneBook(move(contacts));
}