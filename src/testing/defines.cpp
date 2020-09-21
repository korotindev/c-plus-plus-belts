#include <vector>
#include <iostream>

using namespace std;

int main() {
  const vector<int> vec = {1};
  int* a = new int;
  cout << vec[1];
  return 0;
}
