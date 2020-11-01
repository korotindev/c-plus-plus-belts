#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
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
  vector<pair<int, char>> parents;
  vector<bool> terminals;

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

  int edge_id(char sym) const { return sym - 'a'; }

  int get_path(int vertex_id, char sym) const { return vertexes.at(vertex_id).at(edge_id(sym)); }

  bool has_path(int vertex_id, char sym) const { return get_path(vertex_id, sym) != -1; }

  const vector<int> &get_children(int vertex_id) const { return vertexes.at(vertex_id); }

  const pair<int, char> &get_parent(int vertex_id) const { return parents.at(vertex_id); }

  int add_path(int parent_vertex_id, char sym) {
    int new_vertex_id = add_empty_vertex();
    parents[new_vertex_id] = make_pair(parent_vertex_id, sym);
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

  size_t size() { return vertexes.size(); }
};

class TrieSufLinks {
  Trie trie;
  vector<int> suf_links;
  vector<bool> subterminals;
  vector<int> levels;

  void build_suf_link(int target_vertex_id) {
    if (target_vertex_id == 0) {
      suf_links[target_vertex_id] = -1;
      return;
    }

    auto [vertex_id, target_edge_sym] = trie.get_parent(target_vertex_id);
    vertex_id = suf_links[vertex_id];

    while (vertex_id != -1 && !trie.has_path(vertex_id, target_edge_sym)) {
      vertex_id = suf_links[vertex_id];
    }

    if (vertex_id == -1) {
      suf_links[target_vertex_id] = 0;  // to root;
    } else {
      suf_links[target_vertex_id] = trie.get_path(vertex_id, target_edge_sym);
    }
  };

  void compute_subterminal(int vertex_id) {
    if (vertex_id == 0) {
      return;
    }
    auto suffix_vertex_id = suf_links[vertex_id];
    if (trie.is_terminal(suffix_vertex_id) || subterminals[suffix_vertex_id]) {
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

      for (auto child : trie.get_children(vertex_id)) {
        if (child != -1) {
          vertexes.emplace(child, level + 1);
        }
      }

      vertexes.pop();
    }
  }

 public:
  TrieSufLinks(Trie &&trie) : trie(move(trie)) {
    suf_links.resize(this->trie.size(), -1);
    subterminals.resize(this->trie.size(), false);
    levels.resize(this->trie.size(), 0);
    bfs([this](int vertex_id, int level) {
      build_suf_link(vertex_id);
      compute_subterminal(vertex_id);
      levels[vertex_id] = level;
    });
  }

  vector<pair<int, int>> find_all_ocurrences(const string &str) const {
    vector<pair<int, int>> res;
    int vertex_id = 0;
    for (int i = 0; i < str.size(); ++i) {
      const char sym = str[i];

      while (vertex_id != 0 && !trie.has_path(vertex_id, sym)) {
        vertex_id = suf_links[vertex_id];
      }
      if (trie.has_path(vertex_id, sym)) {
        vertex_id = trie.get_path(vertex_id, sym);
      } else {
        vertex_id = 0;
        continue;
      }

      if (subterminals[vertex_id]) {
        int tmp_vertex_id = vertex_id;
        while (tmp_vertex_id != -1 && subterminals[tmp_vertex_id]) {
          if (trie.is_terminal(tmp_vertex_id)) {
            res.emplace_back(i - levels[tmp_vertex_id] + 1, i);
          }
          tmp_vertex_id = suf_links[tmp_vertex_id];
        }
        if (trie.is_terminal(tmp_vertex_id)) {
          res.emplace_back(i - levels[tmp_vertex_id] + 1, i);
        }
      } else if (trie.is_terminal(vertex_id)) {
        res.emplace_back(i - levels[vertex_id] + 1, i);
      }
    }

    return res;
  }
};

//  {"abbab", "baba", "aabb", "abaab", "bbbaa", "bb"};
// aababbbababbaabb

bool dp(vector<pair<int, int>> &ranges, const string &s) {
  auto comparator = [](auto lhs, auto rhs) {
    return make_pair(lhs.second, lhs.first) < make_pair(rhs.second, rhs.first);
  };
  auto right_comparator = [](auto lhs, auto rhs) { return lhs.second < rhs.second; };

  // for (const auto &p : ranges) {
  //   cout << "from " << p.first << " to " << p.second << ": ";
  //   for (int i = p.first; i <= p.second; i++) {
  //     cout << s[i];
  //   }
  //   cout << endl;
  // }

  vector<int> data(ranges.size() + 1);
  vector<int> prev_compatible(ranges.size() + 1);
  prev_compatible[0] = -1;
  for (int i = 1; i < prev_compatible.size(); i++) {
    auto it = upper_bound(ranges.begin(), ranges.end(), make_pair(-1, ranges[i - 1].first), comparator);
    prev_compatible[i] = distance(ranges.begin(), it);
  }

  for (int i = 1; i < data.size(); i++) {
    int range_cost = ranges[i - 1].second - ranges[i - 1].first + 1;
    data[i] = max(data[i - 1], data[prev_compatible[i]] + range_cost);
  }

  return data[ranges.size()] == s.size();
}

int main() {
  string s;
  cin >> s;

  size_t n;
  cin >> n;
  vector<string> data(n);
  Trie trie;

  for (auto &elem : data) {
    cin >> elem;
    trie.add_string(elem);
  }

  TrieSufLinks trie_suf_links(move(trie));

  vector<pair<int, int>> ranges = trie_suf_links.find_all_ocurrences(s);

  if (dp(ranges, s)) {
    cout << "YES";
  } else {
    cout << "NO";
  }

  return 0;
}