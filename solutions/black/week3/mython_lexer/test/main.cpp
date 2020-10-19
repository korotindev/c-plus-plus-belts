#include <test_runner.h>

#include <sstream>
#include <string>

#include "lexer.h"

using namespace std;

int main() {
  TestRunner tr;
  Parse::RunLexerTests(tr);
  return 0;
}