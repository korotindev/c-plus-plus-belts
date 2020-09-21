#include "test_runner.h"

#include <cstddef> // нужно для nullptr_t

using namespace std;

// Реализуйте шаблон класса UniquePtr
template <typename T>
class UniquePtr {
private:
  T *__data_ptr = nullptr;

public:
  UniquePtr() : __data_ptr(nullptr) {}

  UniquePtr(T *ptr) : __data_ptr(ptr) {}

  UniquePtr(const UniquePtr &) = delete;

  UniquePtr(UniquePtr &&other) { this->Reset(other.Release()); }

  UniquePtr &operator=(const UniquePtr &) = delete;

  UniquePtr &operator=(nullptr_t) {
    this->Reset(nullptr);
    return *this;
  }

  UniquePtr &operator=(UniquePtr &&other) {
    this->Reset(other.Release());
    return *this;
  }

  ~UniquePtr() { this->Reset(nullptr); }

  T &operator*() const { return *this->__data_ptr; }

  T *operator->() const { return this->__data_ptr; }

  T *Release() {
    auto tmp_ptr = this->__data_ptr;
    this->__data_ptr = nullptr;
    return tmp_ptr;
  }

  void Reset(T *ptr) {
    if (this->__data_ptr) {
      delete this->__data_ptr;
    }
    this->__data_ptr = ptr;
  }

  void Swap(UniquePtr &other) { swap(this->__data_ptr, other.__data_ptr); }

  T *Get() const { return this->__data_ptr; }
};

struct Item {
  static int counter;
  int value;

  Item(int v = 0) : value(v) { ++counter; }

  Item(const Item &other) : value(other.value) { ++counter; }

  ~Item() { --counter; }
};

int Item::counter = 0;

void TestLifetime() {
  Item::counter = 0;
  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    ptr.Reset(new Item);
    ASSERT_EQUAL(Item::counter, 1);
  }
  ASSERT_EQUAL(Item::counter, 0);

  {
    UniquePtr<Item> ptr(new Item);
    ASSERT_EQUAL(Item::counter, 1);

    auto rawPtr = ptr.Release();
    ASSERT_EQUAL(Item::counter, 1);

    delete rawPtr;
    ASSERT_EQUAL(Item::counter, 0);
  }
  ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
  UniquePtr<Item> ptr(new Item(42));
  ASSERT_EQUAL(ptr.Get()->value, 42);
  ASSERT_EQUAL((*ptr).value, 42);
  ASSERT_EQUAL(ptr->value, 42);
}

void TestDoubleFree() {
  {
    Item::counter = 0;
    UniquePtr<Item> ptr(new Item(42));
    ASSERT_EQUAL(Item::counter, 1);
    {
      UniquePtr<Item> tmp = move(ptr);
      ASSERT_EQUAL(Item::counter, 1);
      TestGetters();
      ASSERT_EQUAL(Item::counter, 1);
      ptr = move(tmp);
      ASSERT_EQUAL(Item::counter, 1);
    }
    ASSERT_EQUAL(Item::counter, 1);
  }
  ASSERT_EQUAL(Item::counter, 0);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestLifetime);
  RUN_TEST(tr, TestGetters);
  RUN_TEST(tr, TestDoubleFree);
}