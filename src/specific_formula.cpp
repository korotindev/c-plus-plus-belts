#include "specific_formula.h"

using namespace std;

IFormula::Value SpecificFormula::Evaluate(const ISheet& sheet) const { return 0.0; }
std::string SpecificFormula::GetExpression() const { return ""; }
std::vector<Position> SpecificFormula::GetReferencedCells() const { return {}; }
IFormula::HandlingResult SpecificFormula::HandleInsertedRows(int before, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult SpecificFormula::HandleInsertedCols(int before, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult SpecificFormula::HandleDeletedRows(int first, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult SpecificFormula::HandleDeletedCols(int first, int count) {
  return IFormula::HandlingResult::NothingChanged;
}