#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace std;
//       c       d       c
// a b a ? a b a ? a b a ? a b a
// 0 0 1 0 3 0 1 0 7 0 1 0 3 0 1

// a
// 0 1 4 5 2 0 1

string get_res(const vector<size_t> &data) {
  if (data.empty() || data[0] != 0) {
    return "!";
  }

  string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t alphabet_id = 0;

  string s(data.size(), ' ');
  s[0] = alphabet[alphabet_id++];

  for (size_t i = 1; i < data.size(); i++) {
    if (s[i] == ' ') {
      if (data[i] == 0) {
        if (alphabet_id == alphabet.size()) {
          return "!";
        } else {
          s[i] = alphabet[alphabet_id++];
        }
      } else {
        if (i + data[i] > data.size()) {
          return "!";
        }
        for (size_t j = 0; j < data[i]; j++) {
          s[i + j] = s[j];
        }
      }
    } else {
      if (i + data[i] > data.size()) {
        return "!";
      }

      for (size_t j = 0; j < data[i]; j++) {
        if (s[i + j] != s[j]) {
          return "!";
        }
      }
    }
  }

  return s;
}

vector<int> z(string_view str) {
  vector<int> result(str.size(), 0);
  for (size_t i = 1; i < str.size(); i++) {
    auto &back = result[i];
    while (i + back < str.size() && str[back] == str[i + back]) {
      back++;
    }
  }
  return result;
}

ostream &operator<<(ostream &out, const vector<int> &vec) {
  copy(vec.begin(), vec.end(), ostream_iterator<int>(out, " "));
  return out;
}

int main() {
  size_t n;
  cin >> n;
  while (n--) {
    size_t k;
    cin >> k;
    vector<size_t> data(k);
    for (auto &elem : data) {
      cin >> elem;
    }

    cout << get_res(data) << '\n';
  }
  return 0;
}