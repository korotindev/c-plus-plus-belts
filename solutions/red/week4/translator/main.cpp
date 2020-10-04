#include <forward_list>
#include <string_view>

#include "profile.h"
#include "test_runner.h"

using namespace std;

class Translator {
 public:
  void Add(string_view source, string_view target) {
    auto strv_target = GetCachedClone(target);
    auto strv_source = GetCachedClone(source);
    forward[strv_source] = strv_target;
    backward[strv_target] = strv_source;
  }

  string_view TranslateForward(string_view source) const {
    if (forward.count(source) == 0) return {};
    return forward.at(source);
  }

  string_view TranslateBackward(string_view target) const {
    if (backward.count(target) == 0) return "";
    return backward.at(target);
  }

 private:
  string_view GetCachedClone(string_view str) {
    for (const auto *dict : {&forward, &backward}) {
      const auto it = dict->find(str);
      if (it != dict->end()) {
        return it->first;
      }
    }

    return data.emplace_front(str);
  }

  map<string_view, string_view> forward;
  map<string_view, string_view> backward;
  forward_list<string> data;
};

void TestSimple() {
  Translator translator;
  translator.Add(string("okno"), string("window"));
  translator.Add(string("stol"), string("table"));

  ASSERT_EQUAL(translator.TranslateForward("okno"), "window");
  ASSERT_EQUAL(translator.TranslateBackward("table"), "stol");
  ASSERT_EQUAL(translator.TranslateBackward("stol"), "");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSimple);
  return 0;
}