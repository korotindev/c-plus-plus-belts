#include "object.h"
#include "object_holder.h"
#include "statement.h"
#include "lexer.h"
#include "parse.h"

#include "mython.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void RunMythonProgram(istream& input, ostream& output) {
  Ast::Print::SetOutputStream(output);

  Parse::Lexer lexer(input);
  auto program = ParseProgram(lexer);

  Runtime::Closure closure;
  program->Execute(closure);
}