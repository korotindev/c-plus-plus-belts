#include "lexer.h"
#include <test_runner.h>

#include <string>
#include <sstream>

using namespace std;

int main() {
  TestRunner tr;
  Parse::RunLexerTests(tr);
  return 0;
}