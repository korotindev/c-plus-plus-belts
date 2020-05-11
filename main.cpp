#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

struct Domain {
  string name;

  bool IsSubdomainOf(const Domain& other) const {
    auto subdomainIter = this->name.rbegin();
    auto domainIter = other.name.rbegin();
    while (subdomainIter != this->name.rend() && domainIter != other.name.rend()) {
      if (*subdomainIter != *domainIter) {
        return false;
      }
      subdomainIter++;
      domainIter++;
    }

    if (domainIter == other.name.rend()) {
      return subdomainIter == this->name.rend() ? true : *subdomainIter == '.';
    }

    return false;
  }
};

bool operator==(const Domain& lhs, const Domain& rhs) {
  return lhs.name == rhs.name;
}

ostream& operator<<(ostream& output, const Domain& domain) {
  return output << "Domain{" << domain.name << '}';
}

vector<Domain> ReadDomains(istream& input) {
  size_t count;
  input >> count;

  vector<Domain> domains;
  for (size_t i = 0; i < count; ++i) {
    string domain;
    input >> domain;
    domains.push_back({move(domain)});
  }
  return domains;
}

bool IsBadSubdomain(const Domain& subdomain, const vector<Domain>& banned_subdomains) {
  return any_of(
    begin(banned_subdomains),
    end(banned_subdomains),
    [&subdomain](const Domain& banned_domain) {
      return subdomain.IsSubdomainOf(banned_domain);
    });
}

void Solution(istream& input, ostream& output) {
  auto banned_domains = ReadDomains(input);
  auto subdomains = ReadDomains(input);
  for (const Domain& subdomain : subdomains) {
    if (IsBadSubdomain(subdomain, banned_domains)) {
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
    "google.com\n"
  );

  vector<Domain> domains = ReadDomains(input);
  vector<Domain> expected_domains = {{"yandex.ru"},
                                     {"google.com"}};
  ASSERT_EQUAL(domains, expected_domains);
}

void TestDoubleReadDomains() {
  stringstream input(
    "2\n"
    "yandex.ru\n"
    "google.com\n"
    "2\n"
    "ya.ru\n"
    "mail.ru\n"
  );

  vector<Domain> domains = ReadDomains(input);
  vector<Domain> expected_domains = {{"yandex.ru"},
                                     {"google.com"}};
  ASSERT_EQUAL(domains, expected_domains);
  domains = ReadDomains(input);
  expected_domains = {{"ya.ru"},
                      {"mail.ru"}};
  ASSERT_EQUAL(domains, expected_domains);
}

void TestSubdomainCheck() {
  {
    Domain subdomain{"dima.google.com"};
    Domain domain{"google.com"};

    ASSERT(subdomain.IsSubdomainOf(domain));
    ASSERT(!domain.IsSubdomainOf(subdomain));
  }
  {
    Domain subdomain{"google.com"};
    Domain domain{"google.com"};

    ASSERT(subdomain.IsSubdomainOf(domain));
    ASSERT(domain.IsSubdomainOf(subdomain));
  }
}

void TestBadSubdomainCheck() {
  vector<Domain> banned_domains = {{"ya.ru"},
                                   {"ok.ru"}};

  {
    Domain domain{"m.ya.ru"};
    ASSERT(IsBadSubdomain(domain, banned_domains));
  }
  {
    Domain domain{"m.ok.ru"};
    ASSERT(IsBadSubdomain(domain, banned_domains));
  }
  {
    Domain domain{"ya.ru"};
    ASSERT(IsBadSubdomain(domain, banned_domains));
  }
  {
    Domain domain{"ok.ru"};
    ASSERT(IsBadSubdomain(domain, banned_domains));
  }
  {
    Domain domain{"google.com"};
    ASSERT(!IsBadSubdomain(domain, banned_domains));
  }
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
    "ya.ya\n"
  );
  string expected_output("Bad\n"
                         "Bad\n"
                         "Bad\n"
                         "Bad\n"
                         "Bad\n"
                         "Good\n"
                         "Good\n"
  );
  stringstream output;
  Solution(input, output);
  ASSERT_EQUAL(expected_output, output.str());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestReadDomains);
  RUN_TEST(tr, TestDoubleReadDomains);
  RUN_TEST(tr, TestSubdomainCheck);
  RUN_TEST(tr, TestBadSubdomainCheck);
  RUN_TEST(tr, TestSolution);
  Solution(cin, cout);
  return 0;
}
