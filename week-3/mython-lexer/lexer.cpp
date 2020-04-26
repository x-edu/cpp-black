#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>

using namespace std;

namespace Parse {

bool operator==(const Token& lhs, const Token& rhs) {
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream& operator<<(std::ostream& os, const Token& rhs) {
  using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
  if (rhs.Is<type>()) return os << #type;

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}

namespace {

TokenType::String ReadString(const char quote, std::istream& in) {
  TokenType::String stringToken{};
  for (char symbol = in.get(); symbol != quote; symbol = in.get()) {
    stringToken.value.push_back(symbol);
  }
  return stringToken;
}

inline bool IsIdFirstChar(const char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

inline bool IsDigit(const char c) { return '0' <= c && c <= '9'; }

inline bool IsIdChar(const char c) { return IsIdFirstChar(c) || IsDigit(c); }

TokenType::Id ReadId(const char first_char, std::istream& in) {
  TokenType::Id id{};
  id.value.push_back(first_char);
  while (!in.eof() && IsIdChar(in.peek())) {
    id.value.push_back(in.get());
  }
  return id;
}

const std::map<std::string_view, Token> kKeywords = {
    {"class", TokenType::Class{}}, {"return", TokenType::Return{}},
    {"if", TokenType::If{}},       {"else", TokenType::Else{}},
    {"def", TokenType::Def{}},     {"print", TokenType::Print{}},
    {"or", TokenType::Or{}},       {"None", TokenType::None{}},
    {"and", TokenType::And{}},     {"not", TokenType::Not{}},
    {"True", TokenType::True{}},   {"False", TokenType::False{}},
};

inline bool IsKeyword(const std::string_view s) { return kKeywords.count(s); }

TokenType::Number ReadNumber(const char first_char, std::istream& in) {
  TokenType::Number number{};
  number.value = first_char - '0';
  while (!in.eof() && IsDigit(in.peek())) {
    number.value = number.value * 10 + in.get() - '0';
  }
  return number;
}

}  // namespace

Token Lexer::NextToken() {
  if (!queue_.empty()) {
    current_token_ = queue_.front();
    queue_.pop();
    return current_token_;
  }
  if (input_.eof()) {
    if (indent_ > 0) {
      current_token_ = TokenType::Dedent{};
      indent_ -= 2;
      return current_token_;
    }
    if (current_token_.Is<TokenType::Newline>() ||
        current_token_.Is<TokenType::Dedent>()) {
      added_new_line_at_the_end_ = true;
    }
    if (!added_new_line_at_the_end_) {
      added_new_line_at_the_end_ = true;
      return current_token_ = TokenType::Newline{};
    }
    current_token_ = TokenType::Eof{};
    return current_token_;
  }
  if (current_token_.Is<TokenType::Newline>()) {
    while (!input_.eof() && input_.peek() == '\n') {
      input_.get();
    }
  }
  if (input_.eof()) {
    return NextToken();
  }
  if (current_token_.Is<TokenType::Newline>()) {
    int spaces = 0;
    for (; !input_.eof() && input_.peek() == ' '; input_.get()) {
      ++spaces;
    }
    if (spaces > indent_) {
      current_token_ = TokenType::Indent{};
      indent_ = spaces;
      return current_token_;
    } else if (spaces < indent_) {
      current_token_ = TokenType::Dedent{};
      int delta = indent_ - spaces - 2;
      for (int i = 0; i < delta; i += 2) {
        queue_.push(TokenType::Dedent{});
      }
      indent_ = spaces;
      return current_token_;
    }
  }
  while (!input_.eof() && input_.peek() == ' ') {
    input_.get();
  }
  if (input_.eof()) {
    return NextToken();
  }
  char token = input_.get();
  if (!input_.eof()) {
    if (token == '"' || token == '\'') {
      current_token_ = ReadString(token, input_);
    } else if (IsIdFirstChar(token)) {
      auto id = ReadId(token, input_);
      if (IsKeyword(id.value)) {
        current_token_ = kKeywords.at(id.value);
      } else {
        current_token_ = std::move(id);
      }
    } else if (IsDigit(token)) {
      current_token_ = ReadNumber(token, input_);
    } else {
      current_token_ = TokenType::Char{token};
      switch (token) {
        case '\n':
          current_token_ = TokenType::Newline{};
          break;
        case '!':
          if (input_.peek() == '=') {
            input_.get();
            current_token_ = TokenType::NotEq{};
          }
          break;
        case '=':
          if (input_.peek() == '=') {
            input_.get();
            current_token_ = TokenType::Eq{};
          }
          break;
        case '<':
          if (input_.peek() == '=') {
            input_.get();
            current_token_ = TokenType::LessOrEq{};
          }
          break;
        case '>':
          if (input_.peek() == '=') {
            input_.get();
            current_token_ = TokenType::GreaterOrEq{};
          }
          break;
        default:
          break;
      }
    }
  }
  return current_token_;
}

} /* namespace Parse */
