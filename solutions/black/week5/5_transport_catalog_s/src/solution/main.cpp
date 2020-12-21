#include <iostream>
#include <string_view>

#include "commands.h"

using namespace std;

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: transport_catalog_part_q [make_base|process_requests]\n";
    return 5;
  }

  const string_view mode(argv[1]);

  if (mode == "process_requests") {
    ProcessRequests(cin, cout);
  } else if (mode == "make_base") {
    MakeBase(cin);
  }

  return 0;
}
