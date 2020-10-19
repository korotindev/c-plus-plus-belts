#include <iostream>

using namespace std;

struct Temp {
  unsigned int a;
  unsigned int b;
};

int main() {
  Temp t;
  t.a = 101;
  t.b = 200;
  unsigned int *ptr = reinterpret_cast<unsigned int *>(&t);
  cout << sizeof(Temp) << endl;
  cout << *ptr << endl;
  ptr++;
  cout << *ptr << endl;
  ptr--;
  cout << *ptr << endl;
  return 0;
}