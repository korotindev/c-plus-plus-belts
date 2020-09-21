#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using Palletes = vector<pair<int, int>>;

Palletes ReadPallets(istream &in) {
  int n;
  in >> n;
  Palletes palletes;
  for (int i = 0; i < n; i++) {
    int x, y;
    in >> x >> y;
    palletes.emplace_back(max(x, y), min(x, y));
  }
  return palletes;
}

int Process(Palletes &pallets) {
  int sum = 0;
  sort(pallets.begin(), pallets.end(), [](const pair<int, int> &lhs, const pair<int, int> &rhs) { return lhs > rhs; });

  int currentPal = 0;
  auto canBePlaced = [](const pair<int, int> &object, const pair<int, int> &holder) {
    return holder.first > object.first && holder.second > object.second;
  };

  for (size_t i = 0; i < pallets.size(); i++) {
    if (!canBePlaced(pallets[i], pallets[currentPal])) {
      currentPal = i;
      sum++;
    }
  }

  return sum;
}

void Test() {
  int qwe = 1;
  {
    stringstream input;
    input << "12" << endl
          << "1 5" << endl
          << "2 3" << endl
          << "2 15" << endl
          << "2 15" << endl
          << "2 15" << endl
          << "2 4" << endl
          << "2 7" << endl
          << "3 5" << endl
          << "5 5" << endl
          << "5 7" << endl
          << "6 8" << endl
          << "6 9" << endl;

    auto pals = ReadPallets(input);
    int res = Process(pals);

    ASSERT_EQUAL(res, 5);
  }

  {
    stringstream input;
    input << "2" << endl << "1 3" << endl << "2 8" << endl;

    auto pals = ReadPallets(input);
    int res = Process(pals);

    ASSERT_EQUAL(res, 1);
  }

  {
    stringstream input;
    input << "6" << endl
          << "1 3" << endl
          << "1 7" << endl
          << "7 1" << endl
          << "8 1" << endl
          << "1000 1" << endl
          << "1 8" << endl;

    auto pals = ReadPallets(input);
    int res = Process(pals);

    ASSERT_EQUAL(res, 6);
  }
}

int main() {
  std::ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  TestRunner tr;
  RUN_TEST(tr, Test);

  auto pallets = ReadPallets(cin);
  cout << Process(pallets) << '\n';
  return 0;
}
