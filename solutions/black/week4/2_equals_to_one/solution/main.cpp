#include "test_runner.h"

// EqualsToOneOf(x, "apple", "orange")‚ (x == "apple" || x == "orange")

template <typename T, typename ...Args>
bool EqualsToOneOf(const T &t, const Args &...args) {
  return ((t == args) || ...);
}

void Test() {
  auto x = "pear";
  ASSERT(EqualsToOneOf(x, "pear"));
  ASSERT(!EqualsToOneOf(x, "apple"));
  ASSERT(EqualsToOneOf(x, "apple", "pear"));
  ASSERT(!EqualsToOneOf(x, "apple", "banana"));
  ASSERT(EqualsToOneOf(x, "apple", "banana", "pear"));
  ASSERT(!EqualsToOneOf(x, "apple", "banana", "peach"));
  ASSERT(EqualsToOneOf(x, "apple", "banana", "pear", "orange"));
  ASSERT(!EqualsToOneOf(x, "apple", "banana", "peach", "orange"));
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, Test);
  return 0;
}