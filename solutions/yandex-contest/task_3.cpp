// Задача: C. Packed Prefix
// Ссылка: https://contest.yandex.ru/contest/19251/problems/C/
// Время посылки: 1 ноя 2020, 18:30:48
// Номер посылки: 39042615

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <stack>
#include <unordered_map>
#include <vector>

using namespace std;

static const auto magic = []() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  return nullptr;
}();

class Trie {
  vector<vector<int>> vertexes;
  vector<bool> terminals;

 public:
  Trie() { add_empty_vertex(); }

  int add_empty_vertex() {
    vector<int> empty(26, -1);
    int vertex_id = vertexes.size();
    vertexes.push_back(move(empty));
    terminals.push_back(false);
    return vertex_id;
  }

  int edge_id(char sym) const {
    assert('a' <= sym && sym <= 'z');
    return sym - 'a';
  }

  char edge_name(int child_id) const {
    assert(0 <= child_id && child_id <= 25);
    return static_cast<char>('a' + child_id);
  }

  int get_path(int vertex_id, char sym) const { return vertexes.at(vertex_id).at(edge_id(sym)); }

  bool has_path(int vertex_id, char sym) const { return get_path(vertex_id, sym) != -1; }

  const vector<int> &get_children(int vertex_id) const { return vertexes.at(vertex_id); }

  int add_path(int parent_vertex_id, char sym) {
    int new_vertex_id = add_empty_vertex();
    vertexes[parent_vertex_id][edge_id(sym)] = new_vertex_id;
    return new_vertex_id;
  }

  void mark_terminal(int vertex_id) { terminals[vertex_id] = true; }

  bool is_terminal(int vertex_id) const { return terminals.at(vertex_id); }

  void add_string(const string &s) {
    int vertex_id = 0;
    for (const char c : s) {
      if (auto new_vertex_id = get_path(vertex_id, c); new_vertex_id != -1) {
        vertex_id = new_vertex_id;
      } else {
        vertex_id = add_path(vertex_id, c);
      }
    }
    mark_terminal(vertex_id);
  }

  void add_longest_common_prefix(const string &s) {
    int vertex_id = 0;
    for (const char c : s) {
      if (is_terminal(vertex_id)) {
        break;
      }

      if (has_path(vertex_id, c)) {
        vertex_id = get_path(vertex_id, c);
        continue;
      }

      const auto &children = get_children(vertex_id);
      if (any_of(children.begin(), children.end(), [](const int child_vertex_id) { return child_vertex_id != -1; })) {
        break;
      }

      vertex_id = add_path(vertex_id, c);
    }

    mark_terminal(vertex_id);
  }

  bool has_string(string_view sv) const {
    int vertex_id = 0;
    for (const char c : sv) {
      vertex_id = get_path(vertex_id, c);
      if (vertex_id == -1) {
        return false;
      }
    }

    return true;
  }

  size_t size() { return vertexes.size(); }
};

string unpack(const string &str) {
  if (str.empty()) {
    return "";
  }
  string result;

  struct packed_part {
    size_t repetitions;
    string part;
  };

  stack<packed_part> unpacked_parts;

  auto merge_two_top_parts = [&unpacked_parts]() {
    size_t repetitions = unpacked_parts.top().repetitions;
    string part = move(unpacked_parts.top().part);
    unpacked_parts.pop();
    if (unpacked_parts.empty()) {
      unpacked_parts.push({1, ""});
    }
    while (repetitions--) {
      unpacked_parts.top().part += part;
    }
  };

  for (size_t i = 0; i < str.size(); ++i) {
    const char sym = str[i];
    if (isdigit(sym)) {
      unpacked_parts.push({static_cast<size_t>(sym - '0'), ""});
      i++;
    } else if (sym == ']') {
      merge_two_top_parts();
    } else {
      if (unpacked_parts.empty()) {
        unpacked_parts.push({1, ""});
      }
      unpacked_parts.top().part += sym;
    }
  }

  while (unpacked_parts.size() > 1) {
    merge_two_top_parts();
  }

  result = move(unpacked_parts.top().part);

  return result;
}

string longest_common_prefix(const Trie &trie) {
  int vertex_id = 0;
  string result;

  while (vertex_id != -1 && !trie.is_terminal(vertex_id)) {
    char c;
    int new_vertex_id = -1;
    for (char sym = 'a'; sym <= 'z'; sym++) {
      if (trie.has_path(vertex_id, sym)) {
        c = sym;
        new_vertex_id = trie.get_path(vertex_id, sym);
      }
    }

    result += c;
    vertex_id = new_vertex_id;
  }

  return result;
}

int main() {
  size_t n = 0;
  cin >> n;
  cin.ignore(1);
  Trie trie;
  while (n--) {
    string str;
    getline(cin, str);
    trie.add_longest_common_prefix(unpack(str));
  }

  cout << longest_common_prefix(trie);

  return 0;
}