#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <random>
#include <set>
#include <utility>
#include <vector>

#include "test_runner.h"

using namespace std;

template <typename T>
class PriorityCollection {
 public:
  using Id = int;
  using Priority = int;

  Id Add(T object) {
    const int id = objects.size();
    objects.push_back({move(object)});
    sorted_objects.insert({0, id});
    return id;
  }

  template <typename ObjInputIt, typename IdOutputIt>
  void Add(ObjInputIt range_begin, ObjInputIt range_end, IdOutputIt ids_begin) {
    for (; range_begin != range_end; range_begin++) {
      *ids_begin = Add(move(*range_begin));
      ids_begin++;
    }
  }

  bool IsValid(Id id) const { return 0 <= id && id < objects.size() && objects[id].priority != NONE_PRIORITY; }

  const T &Get(Id id) const { return objects[id].object; }

  void Promote(Id id) {
    auto &object = objects[id];
    sorted_objects.erase({object.priority, id});
    object.priority++;
    sorted_objects.insert({object.priority, id});
  }

  pair<const T &, int> GetMax() const {
    const auto objectIterator = prev(sorted_objects.end());
    const auto &object = objects[objectIterator->second].object;
    return {object, objectIterator->first};
  }

  pair<T, int> PopMax() {
    auto objectIterator = prev(sorted_objects.end());
    auto &objectItem = objects[objectIterator->second];
    sorted_objects.erase(objectIterator);
    const auto priority = objectItem.priority;
    objectItem.priority = NONE_PRIORITY;
    return {move(objectItem.object), priority};
  }

 private:
  struct ObjectItem {
    T object;
    Priority priority = 0;
  };
  static const Priority NONE_PRIORITY = -1;
  vector<ObjectItem> objects;
  set<pair<Priority, Id>> sorted_objects;
};

class StringNonCopyable : public string {
 public:
  using string::string;  // Позволяет использовать конструкторы строки
  StringNonCopyable(const StringNonCopyable &) = delete;

  StringNonCopyable(StringNonCopyable &&) = default;

  StringNonCopyable &operator=(const StringNonCopyable &) = delete;

  StringNonCopyable &operator=(StringNonCopyable &&) = default;
};

void TestNoCopy() {
  PriorityCollection<StringNonCopyable> strings;
  const auto white = strings.Add("white");
  const auto yellow_id = strings.Add("yellow");
  const auto red_id = strings.Add("red");

  strings.Promote(yellow_id);
  for (int i = 0; i < 2; ++i) {
    strings.Promote(red_id);
  }
  strings.Promote(yellow_id);
  {
    const auto item = strings.PopMax();
    ASSERT_EQUAL(item.first, "red");
    ASSERT_EQUAL(item.second, 2);
  }
  {
    const auto item = strings.PopMax();
    ASSERT_EQUAL(item.first, "yellow");
    ASSERT_EQUAL(item.second, 2);
  }
  {
    const auto item = strings.PopMax();
    ASSERT_EQUAL(item.first, "white");
    ASSERT_EQUAL(item.second, 0);
  }
}

void TestManyAddsDuration() {
  PriorityCollection<string> qwe;
  for (int i = 0; i < 1000000; i++) {
    qwe.Add(to_string(i));
  }
}

void TestManyAddsWithPopsDuration() {
  PriorityCollection<StringNonCopyable> strings;
  for (int i = 0; i < 1000000 / 2; i++) {
    StringNonCopyable s("white");
    strings.Add(move(s));
    strings.PopMax();
  }
}

void TestManyPromoteDuration() {
  PriorityCollection<StringNonCopyable> strings;

  const auto white_id = strings.Add("white");
  const auto yellow_id = strings.Add("yellow");
  const auto red_id = strings.Add("red");

  for (int i = 0; i < 1'000'000 / 3; i++) {
    strings.Promote(white_id);
    strings.Promote(yellow_id);
    strings.Promote(red_id);
  }
}

void TestGetMaxReturnsLastAdded() {
  PriorityCollection<string> strings;
  const auto white_id = strings.Add("white");
  const auto yellow_id = strings.Add("yellow");
  const auto red_id = strings.Add("red");

  strings.Promote(red_id);
  strings.Promote(yellow_id);
  strings.Promote(white_id);

  auto redMaxElem = strings.PopMax();
  ASSERT_EQUAL(redMaxElem.first, "red");
  auto yellowMaxElem = strings.PopMax();
  ASSERT_EQUAL(yellowMaxElem.first, "yellow");
  auto whiteMaxElem = strings.PopMax();
  ASSERT_EQUAL(whiteMaxElem.first, "white");
}

void TestPopAndGetMaxEquality() {
  PriorityCollection<string> strings;
  const auto white_id = strings.Add("white");
  const auto yellow_id = strings.Add("yellow");
  const auto red_id = strings.Add("red");

  strings.Promote(red_id);
  strings.Promote(yellow_id);
  strings.Promote(white_id);

  auto maxElem = strings.GetMax();
  ASSERT_EQUAL(maxElem.first, "red");
  ASSERT_EQUAL(maxElem.second, 1);
  auto poppedMaxElem = strings.PopMax();
  ASSERT_EQUAL(poppedMaxElem.first, "red");
  ASSERT_EQUAL(poppedMaxElem.second, 1);
}

void TestRangedAdd() {
  PriorityCollection<StringNonCopyable> strings;

  StringNonCopyable s1("white");
  StringNonCopyable s2("yellow");
  StringNonCopyable s3("red");

  vector<StringNonCopyable> v;
  v.push_back(move(s1));
  v.push_back(move(s2));
  v.push_back(move(s3));

  vector<PriorityCollection<StringNonCopyable>::Id> ids;
  ids.reserve(3);
  strings.Add(v.begin(), v.end(), ids.begin());
  const auto white_id = ids[0];
  const auto yellow_id = ids[1];
  const auto red_id = ids[2];

  strings.Promote(red_id);
  strings.Promote(yellow_id);
  strings.Promote(white_id);

  auto maxElem = strings.GetMax();
  ASSERT_EQUAL(maxElem.first, "red");
  ASSERT_EQUAL(maxElem.second, 1);
  auto poppedMaxElem = strings.PopMax();
  ASSERT_EQUAL(poppedMaxElem.first, "red");
  ASSERT_EQUAL(poppedMaxElem.second, 1);
}

void TestIsValid() {
  PriorityCollection<string> strings;
  const auto white_id = strings.Add("white");
  const auto yellow_id = strings.Add("yellow");
  const auto red_id = strings.Add("red");

  ASSERT(strings.IsValid(white_id));
  ASSERT(strings.IsValid(yellow_id));
  ASSERT(strings.IsValid(red_id));
  strings.Promote(red_id);
  strings.Promote(yellow_id);
  strings.Promote(white_id);
  ASSERT(strings.IsValid(white_id));
  ASSERT(strings.IsValid(yellow_id));
  ASSERT(strings.IsValid(red_id));
  ASSERT(!strings.IsValid(1000));
}

void TestGet() {
  PriorityCollection<StringNonCopyable> strings;

  StringNonCopyable s1("white");
  StringNonCopyable s2("yellow");
  StringNonCopyable s3("red");

  vector<StringNonCopyable> v;
  v.push_back(move(s1));
  v.push_back(move(s2));
  v.push_back(move(s3));

  vector<PriorityCollection<StringNonCopyable>::Id> ids;
  ids.reserve(3);
  strings.Add(v.begin(), v.end(), ids.begin());

  auto maxElem = strings.GetMax();
  ASSERT_EQUAL(strings.Get(ids[0]), "white");
  ASSERT_EQUAL(strings.Get(ids[1]), "yellow");
  ASSERT_EQUAL(strings.Get(ids[2]), "red");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestPopAndGetMaxEquality);
  RUN_TEST(tr, TestNoCopy);
  RUN_TEST(tr, TestManyAddsDuration);
  RUN_TEST(tr, TestManyAddsWithPopsDuration);
  RUN_TEST(tr, TestManyPromoteDuration);
  RUN_TEST(tr, TestGetMaxReturnsLastAdded);
  RUN_TEST(tr, TestRangedAdd);
  RUN_TEST(tr, TestIsValid);
  RUN_TEST(tr, TestGet);
  return 0;
}