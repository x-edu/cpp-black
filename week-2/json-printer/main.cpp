#include <cassert>
#include <cmath>
#include <deque>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <list>

#include "test_runner.h"

namespace Json {

class Node {
 public:
  using JsonArray = std::list<Node>;
  using JsonObject = std::list<std::pair<std::string, Node>>;
  using JsonNumber = int64_t;

  Node& SetString(const std::string_view value) {
    AsString() = value;
    return *this;
  }
  Node& Set(std::string key, Node node) {
    AsObject().emplace_back(std::move(key), std::move(node));
    return *this;
  }
  Node& Add(Node node) {
    AsArray().push_back(std::move(node));
    return *this;
  }
  Node& SetNumber(JsonNumber number) {
    AsNumber() = number;
    return *this;
  }
  Node& SetNull() {
    value_ = std::monostate{};
    return *this;
  }
  Node& SetBool(bool value) {
    AsBool() = value;
    return *this;
  }

  bool IsNull() const { return Is<std::monostate>(); }
  bool IsString() const { return Is<std::string>(); }
  std::string& AsString() { return AsSafe<std::string>(); }
  bool IsArray() const { return Is<JsonArray>(); }
  JsonArray& AsArray() { return AsSafe<JsonArray>(); }
  bool IsObject() const { return Is<JsonObject>(); }
  JsonObject& AsObject() { return AsSafe<JsonObject>(); }
  bool IsNumber() const { return Is<JsonNumber>(); }
  JsonNumber& AsNumber() {
    if (!IsNumber()) {
      value_ = JsonNumber{0};
    }
    return As<JsonNumber>();
  }
  bool IsBool() const { return Is<bool>(); }
  bool& AsBool() { return AsSafe<bool>(); }

 private:
  template <typename T>
  bool Is() const {
    return std::holds_alternative<T>(value_);
  }
  template <typename T>
  T& As() {
    return std::get<T>(value_);
  }
  template <typename T>
  T& AsSafe() {
    if (!Is<T>()) {
      value_ = T{};
    }
    return As<T>();
  }

 private:
  using Value = std::variant<std::monostate, bool, JsonNumber, std::string,
                             JsonArray, JsonObject>;
  Value value_ = std::monostate{};
};

using Array = Node::JsonArray;
using Object = Node::JsonObject;
using Number = Node::JsonNumber;
}  // namespace Json

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

void PrintJsonBool(std::ostream& out, bool val) {
  out << (val ? "true" : "false");
}

void PrintJsonNull(std::ostream& out) { out << "null"; }

void PrintNode(std::ostream& out, Json::Node& node);
void PrintJsonObject(std::ostream& out, Json::Object& object) {
  out << '{';
  int id = 0;
  for (auto& [key, node] : object) {
    if (id++ > 0) {
      out << ',';
    }
    PrintJsonString(out, key);
    out << ':';
    PrintNode(out, node);
  }
  out << '}';
}

void PrintJsonArray(std::ostream& out, Json::Array& array) {
  out << '[';
  int id = 0;
  for (auto& item : array) {
    if (id++ > 0) {
      out << ',';
    }
    PrintNode(out, item);
  }
  out << ']';
}

void PrintJsonNumber(std::ostream& out, int64_t number) { out << number; }

void PrintNode(std::ostream& out, Json::Node& node) {
  if (node.IsArray()) {
    return PrintJsonArray(out, node.AsArray());
  }
  if (node.IsString()) {
    return PrintJsonString(out, node.AsString());
  }
  if (node.IsBool()) {
    return PrintJsonBool(out, node.AsBool());
  }
  if (node.IsNull()) {
    return PrintJsonNull(out);
  }
  if (node.IsNumber()) {
    return PrintJsonNumber(out, node.AsNumber());
  }
  if (node.IsObject()) {
    return PrintJsonObject(out, node.AsObject());
  }
}

class EmptyContext {
 public:
  virtual ~EmptyContext() = default;
};

class ObjectContext;
class ArrayContext;
template <typename T>
class InnerArrayContext;
template <typename T>
class InnerObjectContext;

class ArrayContext : public EmptyContext {
 public:
  explicit ArrayContext(std::ostream& ostream) : ostream_(ostream) {}
  ArrayContext() = default;
  ~ArrayContext() override {
    if (ostream_.has_value()) {
      PrintJsonArray(ostream_.value(), array_);
    }
  }
  virtual ArrayContext& Number(int64_t number) {
    array_.push_back(Json::Node{}.SetNumber(number));
    return *this;
  }
  virtual ArrayContext& String(std::string_view string) {
    array_.push_back(Json::Node{}.SetString(string));
    return *this;
  }
  virtual ArrayContext& Boolean(bool value) {
    array_.push_back(Json::Node{}.SetBool(value));
    return *this;
  }
  virtual ArrayContext& Null() {
    array_.push_back(Json::Node{}.SetNull());
    return *this;
  }
  virtual EmptyContext& EndArray() { return *this; }
  virtual InnerArrayContext<ArrayContext> BeginArray();
  virtual InnerObjectContext<ArrayContext> BeginObject();

 private:
  std::optional<std::reference_wrapper<std::ostream>> ostream_;
  Json::Array array_;
};

template <typename T>
class InnerArrayContext final : public ArrayContext {
 public:
  InnerArrayContext(T& ret, Json::Array& array) : ret_(ret), array_(array) {}
  InnerArrayContext& Number(int64_t number) override {
    array_.push_back(Json::Node{}.SetNumber(number));
    return *this;
  }
  InnerArrayContext& String(std::string_view string) override {
    array_.push_back(Json::Node{}.SetString(string));
    return *this;
  }
  InnerArrayContext& Boolean(bool value) override {
    array_.push_back(Json::Node{}.SetBool(value));
    return *this;
  }
  InnerArrayContext& Null() override {
    array_.push_back(Json::Node{}.SetNull());
    return *this;
  }

  T& EndArray() override { return ret_; }

  InnerArrayContext<ArrayContext> BeginArray() override {
    array_.emplace_back();
    return InnerArrayContext<ArrayContext>(*this, array_.back().AsArray());
  }

  InnerObjectContext<ArrayContext> BeginObject() override;

 private:
  Json::Array& array_;
  T& ret_;
};

class ValueContext;

class ObjectContext : public EmptyContext {
 public:
  explicit ObjectContext(std::ostream& ostream) : ostream_(ostream) {}
  ObjectContext() = default;
  ~ObjectContext() override {
    if (ostream_.has_value()) {
      PrintJsonObject(ostream_.value(), object_);
    }
  }

  virtual ValueContext Key(std::string_view key);
  virtual EmptyContext& EndObject() { return *this; }

 private:
  std::optional<std::reference_wrapper<std::ostream>> ostream_;
  Json::Object object_;
};

class ValueContext {
 public:
  explicit ValueContext(ObjectContext& object_context, Json::Node& node)
      : object_context_(object_context), node_(node) {}

  ObjectContext& Number(int64_t number) {
    node_.SetNumber(number);
    return object_context_;
  }
  ObjectContext& String(std::string_view string) {
    node_.SetString(string);
    return object_context_;
  }
  ObjectContext& Boolean(bool value) {
    node_.SetBool(value);
    return object_context_;
  }
  ObjectContext& Null() {
    node_.SetNull();
    return object_context_;
  }
  InnerArrayContext<ObjectContext> BeginArray() {
    return InnerArrayContext<ObjectContext>{object_context_, node_.AsArray()};
  }
  InnerObjectContext<ObjectContext> BeginObject();

 private:
  Json::Node& node_;
  ObjectContext& object_context_;
};

ValueContext ObjectContext::Key(std::string_view key) {
  object_.emplace_back(std::string{key}, Json::Node{});
  return ValueContext(*this, object_.back().second);
}

template <typename T>
class InnerObjectContext final : public ObjectContext {
 public:
  InnerObjectContext(T& ret, Json::Object& object)
      : ret_(ret), object_(object) {}
  ValueContext Key(std::string_view key) override;
  T& EndObject() override { return ret_; }

 private:
  Json::Object& object_;
  T& ret_;
};

template <typename T>
InnerObjectContext<ArrayContext> InnerArrayContext<T>::BeginObject() {
  array_.emplace_back();
  return InnerObjectContext<ArrayContext>(*this, array_.back().AsObject());
}

InnerArrayContext<ArrayContext> ArrayContext::BeginArray() {
  array_.emplace_back();
  return InnerArrayContext<ArrayContext>(*this, array_.back().AsArray());
}

InnerObjectContext<ArrayContext> ArrayContext::BeginObject() {
  array_.emplace_back();
  return InnerObjectContext<ArrayContext>(*this, array_.back().AsObject());
}

template <typename T>
ValueContext InnerObjectContext<T>::Key(std::string_view key) {
  object_.emplace_back(std::string{key}, Json::Node{});
  return ValueContext(*this, object_.back().second);
}

InnerObjectContext<ObjectContext> ValueContext::BeginObject() {
  return InnerObjectContext<ObjectContext>{object_context_, node_.AsObject()};
}

ArrayContext PrintJsonArray(std::ostream& out) { return ArrayContext(out); }

ObjectContext PrintJsonObject(std::ostream& out) { return ObjectContext(out); }

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

  {
    PrintJsonString(output, "Hello, \"world\"");
  }

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

  {
    PrintJsonArray(output)
        .Null()
        .String("Hello")
        .Number(123)
        .Boolean(false);
  }

  ASSERT_EQUAL(output.str(), R"([null,"Hello",123,false])");
}

void TestInnerArray() {
  std::ostringstream output;

  {
    PrintJsonArray(output)
        .String("Hello")
        .BeginArray()
        .String("World");
  }

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
        .Key("foo");  // закрытие объекта в таком состоянии допишет null в качестве значения
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
