#include <iostream>
#include <fstream>
#include <string_view>

#include "processes.h"

using namespace std;

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: belts-runner [make_base|process_requests]\n";
    return 5;
  }

  const string_view mode(argv[1]);

  if (mode == "make_base") {
    MakeBase(cin);
  } else if (mode == "process_requests") {
    ProcessRequests(cin, cout);
  }

  return 0;
}
