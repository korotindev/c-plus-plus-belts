#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <vector>
#include <unordered_map>

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

  int sym_to_id(char sym) const { return sym - 'a'; }

  int get_path(int vertex_id, char sym) const { return vertexes.at(vertex_id).at(sym_to_id(sym)); }

  bool has_path(int vertex_id, char sym) const { return get_path(vertex_id, sym) != -1; }

  const vector<int> &get_children(int vertex_id) const { return vertexes.at(vertex_id); }

  int add_path(int parent_vertex_id, char sym) {
    int new_vertex_id = add_empty_vertex();
    vertexes[parent_vertex_id][sym_to_id(sym)] = new_vertex_id;
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

bool is_pal(string_view sv) {
  for (size_t i = 0; i < sv.size() / 2; i++) {
    if (sv[i] != sv[sv.size() - i - 1]) {
      return false;
    }
  }

  return true;
}

vector<pair<int, int>> find_all_palindrome_pairs(const vector<string> &strs) {
  Trie trie;
  unordered_map<string_view, int> mm;
  vector<string> reversed_strs(strs.begin(), strs.end());

  for (size_t i = 0; i < reversed_strs.size(); ++i) {
    reverse(reversed_strs[i].begin(), reversed_strs[i].end());
    trie.add_string(reversed_strs[i]);
    mm[reversed_strs[i]] = i;
  }

  vector<pair<int, int>> res;

  for (size_t i = 0; i < strs.size(); ++i) {
    const string &str = strs[i];
    for (size_t j = 0; j < str.size(); ++j) {
      string_view left_part(str);
      left_part.remove_suffix(str.size() - j);

      string_view right_part(str);
      right_part.remove_prefix(left_part.size());

      auto lhs_it = mm.find(left_part);
      if (trie.has_string(left_part) && is_pal(right_part) && lhs_it != mm.end() && lhs_it->second != i) {
        res.push_back({i, mm[left_part]});
      }

      auto rhs_it = mm.find(right_part);
      if (trie.has_string(right_part) && is_pal(left_part) && rhs_it != mm.end() && rhs_it->second != i) {
        res.push_back({mm[right_part], i});
      }
    }
  }

  sort(res.begin(), res.end());

  return res;
}

int main() {
  size_t n = 0;
  cin >> n;
  vector<string> strs(n);
  for (auto &str : strs) {
    cin >> str;
  }

  auto palidrome_pairs = find_all_palindrome_pairs(strs);

  for (const auto &p : palidrome_pairs) {
    cout << p.first + 1 << ' ' << p.second + 1 << "\n";
  }

  return 0;
}