#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>

template <typename T>
struct RawMemory {
  T* buf = nullptr;
  size_t cp = 0;

  RawMemory() = default;

  RawMemory(size_t n) {
    buf = static_cast<T*>(operator new(n * sizeof(T)));
    cp = n;
  }

  RawMemory(const RawMemory&) = delete;

  RawMemory(RawMemory&& other) { Swap(other); }

  void Swap(RawMemory& other) {
    std::swap(buf, other.buf);
    std::swap(cp, other.cp);
  }

  ~RawMemory() { operator delete(buf); }

  RawMemory& operator=(const RawMemory&) = delete;

  RawMemory& operator=(RawMemory&& other) {
    Swap(other);
    return *this;
  };

  T& operator[](size_t i) { return buf[i]; }

  const T& operator[](size_t i) const { return buf[i]; }
};

template <typename T>
void p(const RawMemory<T>& data, size_t n) {
  for (size_t i = 0; i != n; i++) {
    std::cout << data[i] << ' ';
  }
  std::cout << std::endl;
}

template <typename T>
class Vector {
  RawMemory<T> data;
  size_t sz = 0;

 public:
  Vector() = default;

  Vector(size_t n) : data(n) {
    std::uninitialized_value_construct_n(data.buf, n);
    sz = n;
  }

  Vector(const Vector& other) : data(other.sz) {
    std::uninitialized_copy_n(other.data.buf, other.sz, data.buf);
    sz = other.sz;
  }

  Vector(Vector&& other) { Swap(other); }

  ~Vector() { std::destroy_n(data.buf, sz); }

  Vector& operator=(const Vector& other) {
    if (data.cp < other.data.cp) {
      Vector tmp(other);
      Swap(tmp);
    } else {
      for (size_t i = 0; i < sz && i < other.sz; i++) {
        data[i] = other[i];
      }
      if (sz < other.sz) {
        std::uninitialized_copy_n(other.data.buf + sz, other.sz - sz, data.buf + sz);
      } else if (sz > other.sz) {
        std::destroy_n(data.buf + other.sz, sz - other.sz);
      }
      sz = other.sz;
    }
    return *this;
  }

  Vector& operator=(Vector&& other) noexcept {
    Swap(other);
    return *this;
  }

  void Swap(Vector& other) {
    data.Swap(other.data);
    std::swap(sz, other.sz);
  }

  void Reserve(size_t n) {
    if (n > data.cp) {
      RawMemory<T> data2(n);
      std::uninitialized_move_n(data.buf, sz, data2.buf);
      std::destroy_n(data.buf, sz);
      data = std::move(data2);
    }
  }
  void Resize(size_t n) {
    Reserve(n);
    if (sz < n) {
      std::uninitialized_value_construct_n(data.buf + sz, n - sz);
    } else if (sz > n) {
      std::destroy_n(data.buf + n, sz - n);
    }
    sz = n;
  }

  void PushBack(const T& elem) {
    if (sz == data.cp) {
      Reserve(sz == 0 ? 1 : sz * 2);
    }
    new (data.buf + sz) T(elem);
    ++sz;
  }

  void PushBack(T&& elem) {
    if (sz == data.cp) {
      Reserve(sz == 0 ? 1 : sz * 2);
    }
    new (data.buf + sz) T(std::move(elem));
    ++sz;
  }

  template <typename... Args>
  T& EmplaceBack(Args&&... args) {
    if (sz == data.cp) {
      Reserve(sz == 0 ? 1 : sz * 2);
    }
    auto obj_ptr = new (data.buf + sz) T(std::forward<Args>(args)...);
    ++sz;
    return *obj_ptr;
  }

  void PopBack() {
    std::destroy_at(data.buf + sz - 1);
    sz--;
  }

  size_t Size() const noexcept { return sz; }

  size_t Capacity() const noexcept { return data.cp; }

  T& operator[](size_t i) { return data[i]; }

  const T& operator[](size_t i) const { return data[i]; }


  // Part 2


  using iterator = T*;
  using const_iterator = const T*;

  iterator begin() noexcept { return data.buf; }
  iterator end() noexcept { return data.buf + sz; }

  const_iterator begin() const noexcept { return data.buf; }
  const_iterator end() const noexcept { return data.buf + sz; }

  const_iterator cbegin() const noexcept { return data.buf; }
  const_iterator cend() const noexcept { return data.buf + sz; }

  iterator Insert(const_iterator pos, const T& elem) {
    RawMemory<T> data2(Size() + 1);
    size_t n = static_cast<size_t>(std::distance(cbegin(), pos));
    std::uninitialized_move_n(data.buf, n, data2.buf);
    std::uninitialized_move_n(data.buf + n, Size() - n, data2.buf + n + 1);
    // revert on exception ??
    new (data2.buf + n) T(elem);
    data.Swap(data2);
    sz++;
    return data.buf + n;
  }
  iterator Insert(const_iterator pos, T&& elem) {
    RawMemory<T> data2(Size() + 1);
    size_t n = static_cast<size_t>(std::distance(cbegin(), pos));
    std::uninitialized_move_n(data.buf, n, data2.buf);
    std::uninitialized_move_n(data.buf + n, Size() - n, data2.buf + n + 1);
    new (data2.buf + n) T(std::move(elem));
    data.Swap(data2);
    sz++;
    return data.buf + n;
  }

  template <typename... Args>
  iterator Emplace(const_iterator pos, Args&&... args) {
    return Insert(pos, T(std::forward<Args>(args)...));
  }

  iterator Erase(const_iterator it) {
    RawMemory<T> data2(Size() - 1);
    size_t n = static_cast<size_t>(std::distance(cbegin(), it));
    std::uninitialized_move_n(data.buf, n, data2.buf);
    std::uninitialized_move_n(data.buf + n + 1, Size() - n - 1, data2.buf + n);
    data.Swap(data2);
    sz--;
    return data.buf + n;
  }
};