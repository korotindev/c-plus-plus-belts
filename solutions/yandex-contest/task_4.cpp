#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<int> z_function(const string &str) {
  vector<int> z(str.size() + 1, 0);
  z[0] = str.size();
  int l = 0, r = 0;
  int z_size = z.size() - 1;

  for (int i = 1; i < z_size; ++i) {
    z[i] = max(0, min(z[i - l], r - i));
    while (str[i + z[i]] == str[z[i]]) {
      z[i]++;
    }
    if (z[i - l] + i > r) {
      l = i;
      r = i + z[i];
    }
  }

  return z;
}

size_t find_min_repeating_pattern_size(const vector<int> &z) {
  auto check_pattern = [&z](const size_t pattern_size) {
    size_t k = 0;
    // size_t 0 - size_t 1 == max(size_t) => i < z.size() => for loop will check i == 0;
    for (size_t i = z.size() - pattern_size - 1; i < z.size(); i -= pattern_size) {
      if (z[i] == k + pattern_size) {
        k += pattern_size;
      } else {
        return false;
      }
    }

    return true;
  };
  
  // checking patterns which overlap half of size does not make sense
  for (size_t i = z.size(); i >= z.size() / 2; --i) {
    if (z[i - 1] == z.size() - i && z[i - 1] > 0) {
      if (check_pattern(z[i - 1])) {
        return z[i - 1];
      }
    }
  }
  return z.size() - 1;
}

int main() {
  string str;
  getline(cin, str);
  const auto z = z_function(str);
  cout << str.size() / find_min_repeating_pattern_size(z);
  return 0;
}