// Задача: B. Поиск образцов в тексте*
// Ссылка: https://contest.yandex.ru/contest/19251/problems/B/
// Время посылки: 2 ноя 2020, 13:25:29
// Номер посылки: 39079132

#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string_view>

using namespace std;

static const auto magic = []() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  return nullptr;
}();

class Trie {
  vector<vector<int>> vertexes;
  vector<pair<int, char>> parents;
  vector<bool> terminals;

  int add_path(int parent_vertex_id, char sym) {
    int new_vertex_id = add_empty_vertex();
    parents[new_vertex_id] = make_pair(parent_vertex_id, sym);
    vertexes[parent_vertex_id][sym_to_id(sym)] = new_vertex_id;
    return new_vertex_id;
  }

  void mark_terminal(int vertex_id) { terminals[vertex_id] = true; }

 public:
  Trie() { add_empty_vertex(); }

  int add_empty_vertex() {
    vector<int> empty(26, -1);
    int vertex_id = vertexes.size();
    vertexes.push_back(move(empty));
    terminals.push_back(false);
    parents.emplace_back(-1, ' ');
    return vertex_id;
  }

  int sym_to_id(char sym) const { return sym - 'a'; }

  int get_path(int vertex_id, char sym) const { return vertexes.at(vertex_id).at(sym_to_id(sym)); }

  bool has_path(int vertex_id, char sym) const { return get_path(vertex_id, sym) != -1; }

  const vector<int> &get_children(int vertex_id) const { return vertexes.at(vertex_id); }

  const pair<int, char> &get_parent(int vertex_id) const { return parents.at(vertex_id); }

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

  size_t size() const { return vertexes.size(); }
};

class TrieSufLinks {
  unique_ptr<Trie> trie;
  vector<int> suf_links;
  vector<bool> subterminals;
  vector<int> levels;

  void build_suf_link(int target_vertex_id) {
    if (target_vertex_id == 0) {
      suf_links[target_vertex_id] = -1;
      return;
    }

    auto [vertex_id, target_edge_sym] = trie->get_parent(target_vertex_id);
    vertex_id = suf_links[vertex_id];

    while (vertex_id != -1 && !trie->has_path(vertex_id, target_edge_sym)) {
      vertex_id = suf_links[vertex_id];
    }

    if (vertex_id == -1) {
      suf_links[target_vertex_id] = 0;  // to root;
    } else {
      suf_links[target_vertex_id] = trie->get_path(vertex_id, target_edge_sym);
    }
  };

  void compute_subterminal(int vertex_id) {
    if (vertex_id == 0) {
      return;
    }
    auto suffix_vertex_id = suf_links[vertex_id];
    if (trie->is_terminal(suffix_vertex_id) || subterminals[suffix_vertex_id]) {
      subterminals[vertex_id] = true;
    }
  }

  template <typename Visitor>
  void bfs(Visitor visitor) {
    queue<pair<int, int>> vertexes;
    vertexes.emplace(0, 0);

    while (!vertexes.empty()) {
      auto [vertex_id, level] = vertexes.front();

      visitor(vertex_id, level);

      for (auto child : trie->get_children(vertex_id)) {
        if (child != -1) {
          vertexes.emplace(child, level + 1);
        }
      }

      vertexes.pop();
    }
  }

 public:
  TrieSufLinks(unique_ptr<Trie> trie_) : trie(move(trie_)) {
    suf_links.resize(trie->size(), -1);
    subterminals.resize(trie->size(), false);
    levels.resize(trie->size(), 0);

    bfs([this](int vertex_id, int level) {
      build_suf_link(vertex_id);
      compute_subterminal(vertex_id);
      levels[vertex_id] = level;
    });
  }

  template <typename Visitor>
  void visit_all_ocurrences(const string &str, Visitor visitor) const {
    int vertex_id = 0;

    auto visit_occurence = [&visitor, this, &str](int pos, int vertex_id) {
      visitor(pos - this->levels[vertex_id] + 1, pos);
    };

    for (int i = 0; i < str.size(); ++i) {
      const char sym = str[i];

      while (vertex_id != 0 && !trie->has_path(vertex_id, sym)) {
        vertex_id = suf_links[vertex_id];
      }

      if (trie->has_path(vertex_id, sym)) {
        vertex_id = trie->get_path(vertex_id, sym);
      } else {
        vertex_id = 0;
        continue;
      }

      if (subterminals[vertex_id]) {
        int tmp_vertex_id = vertex_id;
        while (tmp_vertex_id != -1 && subterminals[tmp_vertex_id]) {
          if (trie->is_terminal(tmp_vertex_id)) {
            visit_occurence(i, tmp_vertex_id);
          }
          tmp_vertex_id = suf_links[tmp_vertex_id];
        }
        if (trie->is_terminal(tmp_vertex_id)) {
          visit_occurence(i, tmp_vertex_id);
        }
      } else if (trie->is_terminal(vertex_id)) {
        visit_occurence(i, vertex_id);
      }
    }
  }
};

int main() {
  string text;
  cin >> text;

  size_t patterns_size;
  cin >> patterns_size;
  vector<string> patterns(patterns_size);

  auto trie = make_unique<Trie>();
  for (auto &pattern : patterns) {
    cin >> pattern;
    trie->add_string(pattern);
  }

  TrieSufLinks trie_suf_links(move(trie));

  unordered_map<string_view, vector<int>> occurences;
  trie_suf_links.visit_all_ocurrences(text, [&occurences, &text](int from, int to) {
    string_view pattern_sv(text);
    pattern_sv.remove_prefix(from);
    pattern_sv.remove_suffix(text.size() - to - 1);
    occurences[pattern_sv].push_back(from);
  });

  for (const auto &pattern : patterns) {
    cout << pattern;
    string_view pattern_sv(pattern);
    if (auto it = occurences.find(pattern_sv); it != occurences.end()) {
      for (const auto position : it->second) {  // it's ok if we create a new one
        cout << ' ' << position + 1;            // indexation from 1
      }
    }

    cout << "\n";
  }

  return 0;
}