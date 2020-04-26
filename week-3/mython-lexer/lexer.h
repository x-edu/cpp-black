#pragma once

#include <iosfwd>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

class TestRunner;

namespace Parse {

namespace TokenType {
struct Number {
  int value;
};

struct Id {
  std::string value;
};

struct Char {
  char value;
};

struct String {
  std::string value;
};

struct Class {};
struct Return {};
struct If {};
struct Else {};
struct Def {};
struct Newline {};
struct Print {};
struct Indent {};
struct Dedent {};
struct Eof {};
struct And {};
struct Or {};
struct Not {};
struct Eq {};
struct NotEq {};
struct LessOrEq {};
struct GreaterOrEq {};
struct None {};
struct True {};
struct False {};
}  // namespace TokenType

using TokenBase =
    std::variant<TokenType::Number, TokenType::Id, TokenType::Char,
                 TokenType::String, TokenType::Class, TokenType::Return,
                 TokenType::If, TokenType::Else, TokenType::Def,
                 TokenType::Newline, TokenType::Print, TokenType::Indent,
                 TokenType::Dedent, TokenType::And, TokenType::Or,
                 TokenType::Not, TokenType::Eq, TokenType::NotEq,
                 TokenType::LessOrEq, TokenType::GreaterOrEq, TokenType::None,
                 TokenType::True, TokenType::False, TokenType::Eof>;

//По сравнению с условием задачи мы добавили в тип Token несколько
//удобных методов, которые делают код короче. Например,
//
// token.Is<TokenType::Id>()
//
//гораздо короче, чем
//
// std::holds_alternative<TokenType::Id>(token).
struct Token : TokenBase {
  using TokenBase::TokenBase;

  template <typename T>
  bool Is() const {
    return std::holds_alternative<T>(*this);
  }

  template <typename T>
  const T& As() const {
    return std::get<T>(*this);
  }

  template <typename T>
  const T* TryAs() const {
    return std::get_if<T>(this);
  }
};

bool operator==(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& rhs);

class LexerError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class Lexer {
 public:
  explicit Lexer(std::istream& input) : input_(input) {
    NextToken();
  }

  const Token& CurrentToken() const { return current_token_; }
  Token NextToken();

  template <typename T>
  const T& Expect() const {
    if (current_token_.Is<T>()) {
      return current_token_.As<T>();
    }
    throw LexerError{"expectation failure"};
  }

  template <typename T, typename U>
  void Expect(const U& value) const {
    if (current_token_.Is<T>() && current_token_.As<T>().value == value) {
      return;
    }
    throw LexerError{"expectation failure"};
  }

  template <typename T>
  const T& ExpectNext() {
    NextToken();
    return Expect<T>();
  }

  template <typename T, typename U>
  void ExpectNext(const U& value) {
    NextToken();
    return Expect<T>(value);
  }

 private:
  std::istream& input_;
  int indent_ = 0;
  std::queue<Token> queue_;
  bool added_new_line_at_the_end_ = false;
  Token current_token_ = TokenType::Newline{};
};

void RunLexerTests(TestRunner& test_runner);

} /* namespace Parse */
