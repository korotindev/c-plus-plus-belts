#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "profile.h"

namespace TestRunnerPrivate {
  template <class Container>
  std::ostream &PrintContainer(std::ostream &os, const Container &container) {
    os << "{";
    bool first = true;
    for (const auto &x : container) {
      if (!first) {
        os << ", ";
      }
      first = false;
      os << x;
    }
    return os << "}";
  }
}  // namespace TestRunnerPrivate

template <class K, class V>
std::ostream &operator<<(std::ostream &os, const std::pair<K, V> &p) {
  return os << "{" << p.first << ": " << p.second << "}";
}

template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &data) {
  return TestRunnerPrivate::PrintContainer(os, data);
}

template <class T>
std::ostream &operator<<(std::ostream &os, const std::set<T> &s) {
  return TestRunnerPrivate::PrintContainer(os, s);
}

template <class T>
std::ostream &operator<<(std::ostream &os, const std::unordered_set<T> &s) {
  return os << TestRunnerPrivate::PrintContainer(os, s);
}

template <class K, class V>
std::ostream &operator<<(std::ostream &os, const std::map<K, V> &m) {
  return TestRunnerPrivate::PrintContainer(os, m);
}

template <class K, class V>
std::ostream &operator<<(std::ostream &os, const std::unordered_map<K, V> &m) {
  return TestRunnerPrivate::PrintContainer(os, m);
}

template <class T, class U>
void AssertEqual(const T &t, const U &u, const std::string &hint = {}) {
  if (!(t == u)) {
    std::ostringstream os;
    os << "\n  Assertion failed: \n    " << t << "\n    != \n    " << u << "\n";
    if (!hint.empty()) {
      os << " hint: " << hint;
    }
    throw std::runtime_error(os.str());
  }
}

template <class T, class U, class DiffT>
void AssertCompare(const T &t, const U &u, const DiffT &diff, const std::string &hint = {}) {
  auto res = t - u;
  if (res < 0) {
    res *= -1;
  }
  if (res > diff) {
    std::ostringstream os;
    os << "\n  Assertion failed: \n    abs(" << t << " - " << u << ") = " << res << " > " << diff << "\n";
    if (!hint.empty()) {
      os << " hint: " << hint;
    }
    throw std::runtime_error(os.str());
  }
}

inline void Assert(bool b, const std::string &hint) { AssertEqual(b, true, hint); }

class TestRunner {
 public:
  template <class TestFunc>
  void RunTest(TestFunc func, const std::string &test_name) {
    {
      LOG_DURATION(test_name + " duration");
      try {
        func();
        std::cerr << test_name << " OK" << std::endl;
      } catch (std::exception &e) {
        ++fail_count;
        std::cerr << test_name << " fail: " << e.what() << std::endl;
      } catch (...) {
        ++fail_count;
        std::cerr << "Unknown exception caught" << std::endl;
      }
    }
  }

  ~TestRunner() {
    if (fail_count > 0) {
      std::cerr << fail_count << " unit tests failed. Terminate" << std::endl;
      exit(1);
    }
  }

 private:
  int fail_count = 0;
};
#ifndef FILE_NAME
#define FILE_NAME __FILE__
#endif

#define ASSERT_EQUAL(x, y)                                                                   \
  {                                                                                          \
    std::ostringstream __assert_equal_private_os;                                            \
    __assert_equal_private_os << #x << " != " << #y << ", " << FILE_NAME << ":" << __LINE__; \
    AssertEqual(x, y, __assert_equal_private_os.str());                                      \
  }

#define ASSERT_COMPARE(x, y, diff)                                                           \
  {                                                                                          \
    std::ostringstream __assert_equal_private_os;                                            \
    __assert_equal_private_os << #x << " != " << #y << ", " << FILE_NAME << ":" << __LINE__; \
    AssertCompare(x, y, diff, __assert_equal_private_os.str());                              \
  }

#define ASSERT_COMPARE_INFO(x, y, diff, hint)                                                                                \
  {                                                                                                                          \
    std::ostringstream __assert_equal_private_os;                                                                            \
    __assert_equal_private_os << #x << " != " << #y << ", " << FILE_NAME << ":" << __LINE__ << " additional info: " << hint; \
    AssertCompare(x, y, diff, __assert_equal_private_os.str());                                                              \
  }

#define ASSERT(x)                                                               \
  {                                                                             \
    std::ostringstream __assert_private_os;                                     \
    __assert_private_os << #x << " is false, " << FILE_NAME << ":" << __LINE__; \
    Assert(static_cast<bool>(x), __assert_private_os.str());                    \
  }

#define RUN_TEST(tr, func) tr.RunTest(func, #func)

#define ASSERT_THROWS(expr, expected_exception)                                                                 \
  {                                                                                                             \
    bool __assert_private_flag = true;                                                                          \
    try {                                                                                                       \
      expr;                                                                                                     \
      __assert_private_flag = false;                                                                            \
    } catch (expected_exception &) {                                                                            \
    } catch (...) {                                                                                             \
      std::ostringstream __assert_private_os;                                                                   \
      __assert_private_os << "Expression " #expr                                                                \
                             " threw an unexpected exception"                                                   \
                             " " FILE_NAME ":"                                                                  \
                          << __LINE__;                                                                          \
      Assert(false, __assert_private_os.str());                                                                 \
    }                                                                                                           \
    if (!__assert_private_flag) {                                                                               \
      std::ostringstream __assert_private_os;                                                                   \
      __assert_private_os << "Expression " #expr " is expected to throw " #expected_exception " " FILE_NAME ":" \
                          << __LINE__;                                                                          \
      Assert(false, __assert_private_os.str());                                                                 \
    }                                                                                                           \
  }

#define ASSERT_DOESNT_THROW(expr)                           \
  try {                                                     \
    expr;                                                   \
  } catch (...) {                                           \
    std::ostringstream __assert_private_os;                 \
    __assert_private_os << "Expression " #expr              \
                           " threw an unexpected exception" \
                           " " FILE_NAME ":"                \
                        << __LINE__;                        \
    Assert(false, __assert_private_os.str());               \
  }
