#include <functional>
#include <string>

#include "test_runner.h"

using namespace std;

template <typename T>
class LazyValue {
 public:
  explicit LazyValue(function<T()> init) : init(move(init)) {}

  bool HasValue() const { return static_cast<bool>(cache); }

  const T &Get() const {
    if (!cache) {
      cache = init();
    }
    return *cache;
  }

 private:
  mutable optional<T> cache;
  std::function<T()> init;
};

void UseExample() {
  const string big_string = "Giant amounts of memory";

  LazyValue<string> lazy_string([&big_string] { return big_string; });

  ASSERT(!lazy_string.HasValue());
  ASSERT_EQUAL(lazy_string.Get(), big_string);
  ASSERT(lazy_string.HasValue());
  ASSERT_EQUAL(lazy_string.Get(), big_string);
}

void TestInitializerIsntCalled() {
  bool called = false;

  LazyValue<int> lazy_int([&called] {
    called = true;
    return 0;
  });
  ASSERT(!called);
  lazy_int.Get();
  ASSERT(called);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, UseExample);
  RUN_TEST(tr, TestInitializerIsntCalled);
  return 0;
}