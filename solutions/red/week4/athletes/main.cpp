#include <iostream>
#include <list>
#include <map>

using namespace std;

int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  int n;
  cin >> n;

  list<int> athletes;
  map<int, list<int>::iterator> index_to_iterator;

  for (int i = 0; i < n; i++) {
    int next_athlete_index, athlete_index;
    cin >> athlete_index >> next_athlete_index;
    auto next_athlete_it = index_to_iterator.find(next_athlete_index);
    if (next_athlete_it == index_to_iterator.end()) {
      auto new_it = athletes.insert(athletes.end(), athlete_index);
      index_to_iterator[athlete_index] = new_it;
    } else {
      auto new_it = athletes.insert(next_athlete_it->second, athlete_index);
      index_to_iterator[athlete_index] = new_it;
    }
  }

  for (const auto &index : athletes) {
    cout << index << ' ';
  }

  return 0;
}