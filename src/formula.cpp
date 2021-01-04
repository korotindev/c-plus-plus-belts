#include "formula.h"

#include <sstream>

#include "specific_formula.h"

using namespace std;
unique_ptr<IFormula> ParseFormula(string expression) {
  try {
    return make_unique<SpecificFormula>(move(expression));
  } catch (exception &e) {
    throw_with_nested(FormulaException(e.what()));
  }
}