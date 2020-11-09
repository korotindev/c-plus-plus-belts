#pragma once

#include <utility>

struct BadOptionalAccess {};

template <typename T>
class Optional {
 private:
  alignas(T) unsigned char data[sizeof(T)];
  bool defined = false;

  template <typename Arg>
  void Initialize(Arg&& arg) {
    new (data) T(std::forward<Arg>(arg));
    defined = true;
  }

  template <typename Arg>
  void Assign(Arg&& arg) {
    operator*() = std::forward<Arg>(arg);
    defined = true;
  }

  template <typename Arg>
  void AssignOrInitialize(Arg&& arg) {
    if (defined) {
      Assign(std::forward<Arg>(arg));
    } else {
      Initialize(std::forward<Arg>(arg));
    }
  }

 public:
  Optional() = default;
  Optional(const T& elem) { Initialize(elem); }
  Optional(T&& elem) { Initialize(std::move(elem)); }
  Optional(const Optional& other) {
    if (other.defined) {
      Initialize(*other);
    }
  }
  Optional(Optional&& other) {
    if (other.defined) {
      Initialize(std::move(*other));
    }
  }

  Optional& operator=(const T& elem) {
    AssignOrInitialize(elem);
    return *this;
  }
  Optional& operator=(T&& elem) {
    AssignOrInitialize(std::move(elem));
    return *this;
  }
  Optional& operator=(const Optional& other) {
    if (other.defined) {
      AssignOrInitialize(*other);
    } else {
      Reset();
    }
    return *this;
  }
  Optional& operator=(Optional&& other) {
    if (other.defined) {
      AssignOrInitialize(std::move(*other));
    } else {
      Reset();
    }
    return *this;
  }

  bool HasValue() const { return defined; }

  T& operator*() { return *operator->(); }
  const T& operator*() const { return *operator->(); }
  T* operator->() { return reinterpret_cast<T*>(data); }
  const T* operator->() const { return reinterpret_cast<const T*>(data); }

  T& Value() {
    if (!defined) throw BadOptionalAccess();
    return operator*();
  }
  const T& Value() const {
    if (!defined) throw BadOptionalAccess();
    return operator*();
  }

  void Reset() {
    if (defined) {
      operator*().~T();
      defined = false;
    }
  }

  ~Optional() { Reset(); }
};