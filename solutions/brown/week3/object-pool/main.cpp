#include <algorithm>
#include <iostream>
#include <list>
#include <set>
#include <stdexcept>
#include <string>

#include "test_runner.h"

using namespace std;

template <class T>
class ObjectPool {
 public:
  T *Allocate() {
    if (pool.empty()) {
      pool.emplace_back(new T);
    }
    auto [iter, ok] = allocated.insert(pool.front());
    pool.pop_front();
    return *iter;
  }

  T *TryAllocate() {
    if (pool.empty()) {
      return nullptr;
    }
    return Allocate();
  }

  void Deallocate(T *object) {
    if (allocated.find(object) == allocated.end()) {
      throw invalid_argument("");
    }
    allocated.erase(object);
    pool.push_back(object);
  }

  ~ObjectPool() {
    for (auto x : pool) {
      delete x;
    }

    for (auto x : allocated) {
      delete x;
    }
  }

 private:
  list<T *> pool;
  set<T *> allocated;
};

void TestObjectPool() {
  ObjectPool<string> pool;

  auto p1 = pool.Allocate();
  auto p2 = pool.Allocate();
  auto p3 = pool.Allocate();

  *p1 = "first";
  *p2 = "second";
  *p3 = "third";

  pool.Deallocate(p2);
  ASSERT_EQUAL(*pool.Allocate(), "second");

  pool.Deallocate(p3);
  pool.Deallocate(p1);
  ASSERT_EQUAL(*pool.Allocate(), "third");
  ASSERT_EQUAL(*pool.Allocate(), "first");

  pool.Deallocate(p1);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestObjectPool);
  return 0;
}