#include <cassert>
#include <cmath>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>

#include "test_runner.h"

void PrintJsonString(std::ostream& out, std::string_view str) {
  out << '"';
  for (auto c : str) {
    if (c == '"' || c == '\\') {
      out << '\\';
    }
    out << c;
  }
  out << '"';
}

void PrintJsonNumber(std::ostream& out, int64_t num) { out << num; }
void PrintJsonNull(std::ostream& out) { out << "null"; }

void PrintJsonBool(std::ostream& out, bool flag) { out << std::boolalpha << flag; }

class EmptyContext {};

template <class Owner>
class ObjectContext;

template <class Owner>
class ArrayContext {
  Owner* owner_;
  std::ostream& out_;
  bool empty_;
  bool closed_;

  void RenderComma() {
    if (!empty_) {
      out_ << ',';
    }
    empty_ = false;
  }

 public:
  explicit ArrayContext(Owner* owner_ptr, std::ostream& out)
      : owner_(owner_ptr), out_(out), empty_(true), closed_(false) {
    out_ << '[';
  }
  explicit ArrayContext(std::ostream& out) : ArrayContext<Owner>(nullptr, out) {}

  ~ArrayContext() {
    if (!closed_) {
      out_ << ']';
    }
  }

  ArrayContext& Number(int64_t num) {
    RenderComma();
    PrintJsonNumber(out_, num);
    return *this;
  }

  ArrayContext& Null() {
    RenderComma();
    PrintJsonNull(out_);
    return *this;
  }

  ArrayContext& Boolean(bool flag) {
    RenderComma();
    PrintJsonBool(out_, flag);
    return *this;
  }

  ArrayContext& String(std::string_view str) {
    RenderComma();
    PrintJsonString(out_, str);
    return *this;
  }

  ArrayContext<ArrayContext<Owner>> BeginArray() {
    RenderComma();
    return ArrayContext<ArrayContext<Owner>>(this, out_);
  }

  Owner& EndArray() {
    closed_ = true;
    out_ << ']';
    return *owner_;
  }

  ObjectContext<ArrayContext<Owner>> BeginObject() {
    RenderComma();
    return ObjectContext<ArrayContext<Owner>>(this, out_);
  }
};

template <class Owner>
class ValueObjectContext {
  Owner* owner_;
  std::ostream& out_;
  bool closed_;

 public:
  explicit ValueObjectContext(Owner* owner_ptr, std::ostream& out) : owner_(owner_ptr), out_(out), closed_(false) {
    out_ << ':';
  }
  explicit ValueObjectContext(std::ostream& out) : ValueObjectContext<Owner>(nullptr, out) {}
  ~ValueObjectContext() {
    if (!closed_) {
      PrintJsonNull(out_);
    }
  }

  Owner& Number(int64_t num) {
    PrintJsonNumber(out_, num);
    closed_ = true;
    return *owner_;
  }

  Owner& Null() {
    PrintJsonNull(out_);
    closed_ = true;
    return *owner_;
  }

  Owner& String(std::string_view str) {
    PrintJsonString(out_, str);
    closed_ = true;
    return *owner_;
  }

  Owner& Boolean(bool flag) {
    PrintJsonBool(out_, flag);
    closed_ = true;
    return *owner_;
  }

  ObjectContext<Owner> BeginObject() {
    closed_ = true;
    return ObjectContext<Owner>(this->owner_, out_);
  }
  ArrayContext<Owner> BeginArray() {
    closed_ = true;
    return ArrayContext<Owner>(this->owner_, out_);
  }
};

template <class Owner>
class ObjectContext {
  Owner* owner_;
  std::ostream& out_;
  bool empty_;
  bool closed_;

  void RenderComma() {
    if (!empty_) {
      out_ << ',';
    }
    empty_ = false;
  }

 public:
  explicit ObjectContext(Owner* owner_ptr, std::ostream& out)
      : owner_(owner_ptr), out_(out), empty_(true), closed_(false) {
    out_ << '{';
  }
  explicit ObjectContext(std::ostream& out) : ObjectContext<Owner>(nullptr, out) {}
  ~ObjectContext() {
    if (!closed_) {
      out_ << '}';
    }
  }

  ValueObjectContext<ObjectContext<Owner>> Key(std::string_view str) {
    RenderComma();
    PrintJsonString(out_, str);
    return ValueObjectContext<ObjectContext<Owner>>(this, out_);
  }

  Owner& EndObject() {
    closed_ = true;
    out_ << '}';
    return *owner_;
  }
};

ArrayContext<EmptyContext> PrintJsonArray(std::ostream& out) { return ArrayContext<EmptyContext>(out); }
ObjectContext<EmptyContext> PrintJsonObject(std::ostream& out) { return ObjectContext<EmptyContext>(out); }

void TestArray() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.Number(5).Number(6).BeginArray().Number(7).EndArray().Number(8).String("bingo!").Null();
  }

  ASSERT_EQUAL(output.str(), R"([5,6,[7],8,"bingo!",null])");
}

void TestObject() {
  std::ostringstream output;

  {
    auto json = PrintJsonObject(output);
    json.Key("id1").Number(1234).Key("id2").Boolean(false).Key("").Null().Key("\"").String("\\");
  }

  ASSERT_EQUAL(output.str(), R"({"id1":1234,"id2":false,"":null,"\"":"\\"})");
}

void TestAutoClose() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.BeginArray().BeginObject().Key("arr");
  }

  ASSERT_EQUAL(output.str(), R"([[{"arr":null}]])");
}

void TestAutoClose2() {
  std::ostringstream output;

  PrintJsonObject(output);
  PrintJsonArray(output);

  ASSERT_EQUAL(output.str(), R"({}[])");
}

void TestDeep() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.BeginArray()
        .BeginObject()
        .Key("arr")
        .BeginObject()
        .Key("qwe")
        .BeginArray()
        .Number(1)
        .EndArray()
        .Key("a")
        .String("w");
  }

  ASSERT_EQUAL(output.str(), R"([[{"arr":{"qwe":[1],"a":"w"}}]])");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestArray);
  RUN_TEST(tr, TestObject);
  RUN_TEST(tr, TestAutoClose);
  RUN_TEST(tr, TestAutoClose2);
  RUN_TEST(tr, TestDeep);

  return 0;
}