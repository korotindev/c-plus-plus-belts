#include "formula.h"
#include "specific_formula.h"

using namespace std;

unique_ptr<IFormula> ParseFormula(string expression) {
    return make_unique<SpecificFormula>();
}