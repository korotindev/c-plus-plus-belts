#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "test_runner.h"

using namespace std;

vector<string> ReadDomains(istream &input) {
  size_t count;
  vector<string> domains;
  input >> count;
  domains.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    string domain;
    input >> domain;
    domains.push_back(move(domain));
  }
  return domains;
}

class BadSubdomainsChecker {
 public:
  BadSubdomainsChecker(vector<string> &&domain_names) {
    banned_domains.reserve(domain_names.size());
    for (auto &domain_name : domain_names) {
      banned_domains.push_back({move(domain_name)});
    }
    sort(banned_domains.begin(), banned_domains.end());

    size_t insert_pos = 0;
    for (auto &domain : banned_domains) {
      if (insert_pos == 0 || !domain.IsSubdomainOf(banned_domains[insert_pos - 1])) {
        swap(banned_domains[insert_pos++], domain);
      }
    }
    banned_domains.resize(insert_pos);
  }

  bool Check(string &&domain_name) const {
    ReversedDomain domain(move(domain_name));
    auto it = upper_bound(banned_domains.begin(), banned_domains.end(), domain);
    if (it == banned_domains.begin()) {
      return false;
    }
    it = prev(it);
    bool result = domain.IsSubdomainOf(*it);
    return result;
  }

 private:
  class ReversedDomain {
   public:
    string name;

   public:
    ReversedDomain() {}
    ReversedDomain(ReversedDomain &&reversed) : name(move(reversed.name)) {}

    ReversedDomain &operator=(ReversedDomain &&reversed) {
      this->name = move(reversed.name);
      return *this;
    }

    ReversedDomain(string &&domainName) {
      reverse(domainName.begin(), domainName.end());
      this->name = domainName;
    }

    bool IsSubdomainOf(const ReversedDomain &other) const {
      auto subdomainIter = this->name.begin();
      auto domainIter = other.name.begin();
      while (subdomainIter != this->name.end() && domainIter != other.name.end()) {
        if (*subdomainIter != *domainIter) {
          return false;
        }
        subdomainIter++;
        domainIter++;
      }

      if (domainIter == other.name.end()) {
        return subdomainIter == this->name.end() ? true : *subdomainIter == '.';
      }

      return false;
    }

    friend bool operator<(const ReversedDomain &lhs, const ReversedDomain &rhs) { return lhs.name < rhs.name; }
  };

  vector<ReversedDomain> banned_domains;
};

void Solution(istream &input, ostream &output) {
  BadSubdomainsChecker checker(ReadDomains(input));
  auto subdomains = ReadDomains(input);
  for (string &domain : subdomains) {
    if (checker.Check(move(domain))) {
      output << "Bad\n";
    } else {
      output << "Good\n";
    }
  }
}

void TestReadDomains() {
  stringstream input(
      "2\n"
      "yandex.ru\n"
      "google.com\n");

  vector<string> domains = ReadDomains(input);
  vector<string> expected_domains = {"yandex.ru", "google.com"};
  ASSERT_EQUAL(domains, expected_domains);
}

void TestDoubleReadDomains() {
  stringstream input(
      "2\n"
      "yandex.ru\n"
      "google.com\n"
      "2\n"
      "ya.ru\n"
      "mail.ru\n");

  vector<string> domains = ReadDomains(input);
  vector<string> expected_domains = {"yandex.ru", "google.com"};
  ASSERT_EQUAL(domains, expected_domains);
  domains = ReadDomains(input);
  expected_domains = {"ya.ru", "mail.ru"};
  ASSERT_EQUAL(domains, expected_domains);
}

void TestBadSubdomainChecker() {
  BadSubdomainsChecker checker({
      "ya.ru",
      "m.ya.ru",
      "ok.ru",
      "com",
      "test.vk.ru",
      "m.m.m",
      "m.m",
      "m",
      "h.h.h",
  });
  ASSERT(checker.Check("ya.ru"));
  ASSERT(checker.Check("m.ya.ru"));
  ASSERT(checker.Check("ok.ru"));
  ASSERT(checker.Check("com"));
  ASSERT(checker.Check("test.vk.ru"));
  ASSERT(checker.Check("m.m.m"));
  ASSERT(checker.Check("m.m"));
  ASSERT(checker.Check("m"));
  ASSERT(checker.Check("h.h.h"));

  ASSERT(checker.Check("m.m.ya.ru"));
  ASSERT(checker.Check("m.m.m.ya.ru"));
  ASSERT(checker.Check("m.ok.ru"));
  ASSERT(checker.Check("google.com"));
  ASSERT(checker.Check("apple.com"));
  ASSERT(checker.Check("m.apple.com"));
  ASSERT(checker.Check("m.m.m.m"));
  ASSERT(checker.Check("m.m.m.m.com"));

  ASSERT(!checker.Check("vk.ru"));
  ASSERT(!checker.Check("ru"));
  ASSERT(!checker.Check("ru"));
  ASSERT(!checker.Check("h.h"));
  ASSERT(!checker.Check("h"));
}

void TestSolution() {
  stringstream input(
      "4\n"
      "ya.ru\n"
      "maps.me\n"
      "m.ya.ru\n"
      "com\n"
      "7\n"
      "ya.ru\n"
      "ya.com\n"
      "m.maps.me\n"
      "moscow.m.ya.ru\n"
      "maps.com\n"
      "maps.ru\n"
      "ya.ya\n");
  string expected_output(
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Good\n"
      "Good\n");
  stringstream output;
  Solution(input, output);
  ASSERT_EQUAL(expected_output, output.str());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestReadDomains);
  RUN_TEST(tr, TestDoubleReadDomains);
  RUN_TEST(tr, TestBadSubdomainChecker);
  RUN_TEST(tr, TestSolution);
  Solution(cin, cout);
  return 0;
}
