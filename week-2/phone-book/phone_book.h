#pragma once

#include <algorithm>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "iterator_range.h"

struct Date {
  int year, month, day;
};

struct Contact {
  std::string name;
  std::optional<Date> birthday;
  std::vector<std::string> phones;
};

class PhoneBook {
 public:
  explicit PhoneBook(std::vector<Contact> contacts)
      : contacts_(std::move(contacts)) {
    std::sort(contacts_.begin(), contacts_.end(),
              [](const Contact& lhs, const Contact& rhs) {
                return lhs.name < rhs.name;
              });
  }

  using ContactRange = IteratorRange<std::vector<Contact>::const_iterator>;

  ContactRange FindByNamePrefix(std::string_view name_prefix_view) const;

  void SaveTo(std::ostream& output) const;

 private:
  std::vector<Contact> contacts_;
};

PhoneBook DeserializePhoneBook(std::istream& input);
