//#include "test_runner.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

string alphabet = "abcdefghijklmnopqrstuvwxyz";

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

unordered_set<string> SplitForSet(string_view s, string_view delimiter = " ") {
  unordered_set<string> parts;
  if (s.empty()) {
    return parts;
  }
  while (true) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    parts.emplace(lhs);
    if (!rhs_opt) {
      break;
    }
    s = *rhs_opt;
  }
  return parts;
}

unordered_set<string> ReadOriginalBook(istream &s) {
  string str;
  getline(s, str);
  unordered_set<string> book = SplitForSet(str);
  return book;
}

vector<string> ReadLetter(istream &in) {
  int n;
  in >> n;
  vector<string> letter;
  letter.resize(n);
  for (auto &word : letter) {
    in >> word;
  }
  return letter;
}

void Process(const unordered_set<string> &book, vector<string> &letter) {
  for (auto &word : letter) {
    auto it = book.find(word);
    while (it == book.end()) {
      for (auto &symbol : word) {
        if (symbol == 'z') {
          symbol = 'a';
          continue;
        }
        symbol += 1;
      }
      it = book.find(word);
    }
  }
}

// void TestReadBook() {
//    stringstream str("a abb bab abc");
//    unordered_set<string> res{"a", "abb", "bab", "abc"};
//
//    ASSERT_EQUAL(ReadOriginalBook(str), res);
//}
//
// void TestReadLetter() {
//    stringstream str("6\n"
//                     "q\n"
//                     "bcc\n"
//                     "aza\n"
//                     "abc\n"
//                     "z\n"
//                     "def");
//    vector<string> res{"q", "bcc", "aza", "abc", "z", "def"};
//
//    ASSERT_EQUAL(ReadLetter(str), res);
//}
//
// void TestProcess() {
//    stringstream input("a abb bab abc\n"
//                       "6\n"
//                       "q\n"
//                       "bcc\n"
//                       "aza\n"
//                       "abc\n"
//                       "z\n"
//                       "def");
//    stringstream output;
//    string res("a\n"
//               "abb\n"
//               "bab\n"
//               "abc\n"
//               "a\n"
//               "abc");
//
//    auto book = ReadOriginalBook(input);
//    auto letter = ReadLetter(input);
//    Process(book, letter);
//    vector<string> result = {"a", "abb", "bab", "abc", "a", "abc"};
//
//    ASSERT_EQUAL(letter, result);
//}

int main() {
  std::ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  //    TestRunner tr;
  //
  //    RUN_TEST(tr, TestReadBook);
  //    RUN_TEST(tr, TestReadLetter);
  //    RUN_TEST(tr, TestProcess);

  auto book = ReadOriginalBook(cin);
  auto letter = ReadLetter(cin);
  Process(book, letter);
  stringstream ostr;
  for (auto &word : letter) {
    ostr << word << '\n';
  }
  cout << ostr.str();
  return 0;
}
