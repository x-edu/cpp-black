#include <cassert>
#include <cmath>
#include <deque>
#include <list>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "test_runner.h"

void PrintJsonString(std::ostream& out, std::string_view str) {
  out << '"';
  for (const char c : str) {
    if (c == '"' || c == '\\') {
      out << '\\';
    }
    out << c;
  }
  out << '"';
}

template <typename T>
class PrintContext {
 protected:
  enum class State {
    Initial,
    Intermediate,
    Terminal,
  };

 protected:
  PrintContext(std::ostream& ostream, T& ret)
      : ostream_(ostream), ret_(ret), state_(State::Initial) {}
  virtual ~PrintContext() = default;

  void PrintNull() {
    BeforeValuePrint();
    ostream_ << "null";
  }
  void PrintNumber(int64_t number) {
    BeforeValuePrint();
    ostream_ << number;
  }
  void PrintString(const std::string_view str) {
    BeforeValuePrint();
    PrintJsonString(ostream_, str);
  }
  void PrintBoolean(bool value) {
    BeforeValuePrint();
    ostream_ << (value ? "true" : "false");
  }
  void PrintSeparator() { ostream_ << ','; }
  virtual void BeforeValuePrint() {
    if (state_ != State::Initial) {
      PrintSeparator();
    }
    if (state_ == State::Initial) {
      state_ = State::Intermediate;
    }
  }

  void Terminate() { state_ = State::Terminal; }
  [[nodiscard]] bool IsTerminated() const { return state_ == State::Terminal; }
  [[nodiscard]] bool IsInitial() const { return state_ == State::Initial; }
  std::ostream& Stream() { return ostream_; }
  T& Ret() { return ret_; }

 private:
  std::ostream& ostream_;
  T& ret_;
  State state_;
};

struct EmptyContextType {
} EmptyContext;

template <typename T>
class ArrayContext;

template <typename T>
class ObjectKeyContext;

template <typename T>
class ObjectValueContext;

template <typename T>
class ObjectContext;

template <typename T>
class ArrayContext final : public PrintContext<T> {
 public:
  ArrayContext(std::ostream& ostream, T& ret) : PrintContext<T>(ostream, ret) {
    ostream << '[';
  }

  ~ArrayContext() override { EndArray(); }

  ArrayContext& Number(int64_t value) {
    this->PrintNumber(value);
    return *this;
  }

  ArrayContext& Null() {
    this->PrintNull();
    return *this;
  }

  ArrayContext& String(const std::string_view str) {
    this->PrintString(str);
    return *this;
  }

  ArrayContext& Boolean(bool value) {
    this->PrintBoolean(value);
    return *this;
  }

  ArrayContext<ArrayContext<T>> BeginArray() {
    this->BeforeValuePrint();
    return ArrayContext<ArrayContext<T>>(this->Stream(), *this);
  }

  T& EndArray() {
    if (!this->IsTerminated()) {
      this->Stream() << ']';
      this->Terminate();
    }
    return this->Ret();
  }

  ObjectKeyContext<ArrayContext<T>> BeginObject() {
    this->BeforeValuePrint();
    return ObjectKeyContext<ArrayContext<T>>(this->Stream(), *this);
  }
};

template <typename T>
class ObjectKeyContext : public PrintContext<T> {
 public:
  using Self = ObjectKeyContext<T>;
  ObjectKeyContext(std::ostream& ostream, T& ret)
      : PrintContext<T>(ostream, ret) {
    ostream << '{';
  }

  ~ObjectKeyContext() override { EndObject(); }

  ObjectValueContext<ObjectKeyContext<T>> Key(const std::string_view key) {
    this->PrintString(key);
    return ObjectValueContext<Self>(this->Stream() << ':', *this);
  }

  T& EndObject() {
    if (!this->IsTerminated()) {
      this->Stream() << '}';
      this->Terminate();
    }
    return this->Ret();
  }
};

template <typename T>
class ObjectValueContext : public PrintContext<T> {
 public:
  ObjectValueContext(std::ostream& ostream, T& ret)
      : PrintContext<T>(ostream, ret) {}

  ~ObjectValueContext() {
    if (this->IsInitial()) {
      this->PrintNull();
    }
  }

  T& Number(int64_t value) {
    this->PrintNumber(value);
    return this->Ret();
  }

  T& Null() {
    this->PrintNull();
    return this->Ret();
  }

  T& String(const std::string_view str) {
    this->PrintString(str);
    return this->Ret();
  }

  T& Boolean(bool value) {
    this->PrintBoolean(value);
    return this->Ret();
  }

  ArrayContext<T> BeginArray() {
    this->BeforeValuePrint();
    return ArrayContext<T>(this->Stream(), this->Ret());
  }

  ObjectKeyContext<T> BeginObject() {
    this->BeforeValuePrint();
    return ObjectKeyContext<T>(this->Stream(), this->Ret());
  }
};

ArrayContext<EmptyContextType> PrintJsonArray(std::ostream& out) {
  return ArrayContext<EmptyContextType>(out, EmptyContext);
}

ObjectKeyContext<EmptyContextType> PrintJsonObject(std::ostream& out) {
  return ObjectKeyContext<EmptyContextType>(out, EmptyContext);
}

void TestArray() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.Number(5).Number(6).BeginArray().Number(7).EndArray().Number(8).String(
        "bingo!");
  }

  ASSERT_EQUAL(output.str(), R"([5,6,[7],8,"bingo!"])");
}

void TestObject() {
  std::ostringstream output;

  {
    auto json = PrintJsonObject(output);
    json.Key("id1")
        .Number(1234)
        .Key("id2")
        .Boolean(false)
        .Key("")
        .Null()
        .Key("\"")
        .String("\\");
  }

  ASSERT_EQUAL(output.str(), R"({"id1":1234,"id2":false,"":null,"\"":"\\"})");
}

void TestAutoClose() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.BeginArray().BeginObject();
  }

  ASSERT_EQUAL(output.str(), R"([[{}]])");
}

void TestPrintJsonString() {
  std::ostringstream output;

  { PrintJsonString(output, "Hello, \"world\""); }

  ASSERT_EQUAL(output.str(), R"("Hello, \"world\"")");
}

void TestExplicitClose() {
  std::ostringstream output;

  {
    PrintJsonArray(output)
        .Null()
        .String("Hello")
        .Number(123)
        .Boolean(false)
        .EndArray();
  }

  ASSERT_EQUAL(output.str(), R"([null,"Hello",123,false])");
}

void TestImplicitClose() {
  std::ostringstream output;

  { PrintJsonArray(output).Null().String("Hello").Number(123).Boolean(false); }

  ASSERT_EQUAL(output.str(), R"([null,"Hello",123,false])");
}

void TestInnerArray() {
  std::ostringstream output;

  { PrintJsonArray(output).String("Hello").BeginArray().String("World"); }

  ASSERT_EQUAL(output.str(), R"(["Hello",["World"]])");
}

void TestComplexObject() {
  std::ostringstream output;

  {
    PrintJsonObject(output)
        .Key("foo")
        .BeginArray()
        .String("Hello")
        .EndArray()
        .Key("foo")  // повторяющиеся ключи допускаются
        .BeginObject()
        .Key("foo");  // закрытие объекта в таком состоянии допишет null в
                      // качестве значения
  }

  ASSERT_EQUAL(output.str(), R"({"foo":["Hello"],"foo":{"foo":null}})");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestArray);
  RUN_TEST(tr, TestObject);
  RUN_TEST(tr, TestAutoClose);
  RUN_TEST(tr, TestExplicitClose);
  RUN_TEST(tr, TestPrintJsonString);
  RUN_TEST(tr, TestImplicitClose);
  RUN_TEST(tr, TestInnerArray);
  RUN_TEST(tr, TestComplexObject);

  //  PrintJsonObject(std::cout).String("foo");  // ошибка компиляции
  //
  //  PrintJsonObject(std::cout).Key("foo").Key("bar");  // ошибка компиляции
  //
  //  PrintJsonObject(std::cout).EndArray();  // ошибка компиляции
  //
  //  PrintJsonArray(std::cout)
  //      .Key("foo")
  //      .BeginArray()
  //      .EndArray()
  //      .EndArray();  // ошибка компиляции
  //
  //  PrintJsonArray(std::cout)
  //      .EndArray()
  //      .BeginArray();  // ошибка компиляции  (JSON допускает только одно
  //                      // верхнеуровневое значение)
  //
  //  PrintJsonObject(std::cout)
  //      .EndObject()
  //      .BeginObject();  // ошибка компиляции  (JSON допускает только одно
  //                       // верхнеуровневое значение)

  return 0;
}
