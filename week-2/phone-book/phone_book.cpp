#include "phone_book.h"

#include "contact.pb.h"

PhoneBook DeserializePhoneBook(std::istream& input) {
  PhoneBookSerialize::ContactList contact_list;
  contact_list.ParseFromIstream(&input);
  std::vector<Contact> contacts;
  contacts.reserve(contact_list.contact_size());
  for (const auto& c : contact_list.contact()) {
    Contact contact;
    contact.name = c.name();
    if (c.has_birthday()) {
      contact.birthday = Date{.year = c.birthday().year(),
                              .month = c.birthday().month(),
                              .day = c.birthday().day()};
    }
    contact.phones.reserve(c.phone_number_size());
    for (const auto& phone : c.phone_number()) {
      contact.phones.push_back(phone);
    }
    contacts.push_back(contact);
  }
  return PhoneBook{std::move(contacts)};
}

void PhoneBook::SaveTo(std::ostream& output) const {
  PhoneBookSerialize::ContactList contact_list;
  for (const auto& contact : contacts_) {
    auto& c = *contact_list.add_contact();
    c.set_name(contact.name);
    if (contact.birthday.has_value()) {
      auto& bday = *c.mutable_birthday();
      bday.set_day(contact.birthday->day);
      bday.set_month(contact.birthday->month);
      bday.set_year(contact.birthday->year);
    }
    for (const auto& phone : contact.phones) {
      c.add_phone_number(phone);
    }
  }
  contact_list.SerializeToOstream(&output);
}
PhoneBook::ContactRange PhoneBook::FindByNamePrefix(
    std::string_view name_prefix_view) const {
  std::string name_prefix{name_prefix_view};
  if (name_prefix.empty()) {
    return IteratorRange(contacts_.cbegin(), contacts_.cend());
  }
  constexpr auto comp = [](const Contact& contact, const std::string& v) {
    return contact.name < v;
  };
  const auto left =
      std::lower_bound(contacts_.cbegin(), contacts_.cend(), name_prefix, comp);
  name_prefix.back()++;
  const auto right =
      std::lower_bound(left, contacts_.cend(), name_prefix, comp);
  return IteratorRange(left, right);
}
