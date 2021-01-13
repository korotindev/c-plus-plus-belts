#include "cell.h"

using namespace std;

Cell::Cell(const ISheet* sheet, string text) : sheet_(sheet) {
  if (text.empty() || text[0] != kFormulaSign) {
    data_ = move(text);
  } else {
    data_ = ParseFormula(text.substr(1));
  }
  cached_text_ = BuildText();
}

ICell::Value Cell::GetValue() const {
  if (ContainsFormula()) {
    IFormula::Value formula_value = GetFormula()->Evaluate(*sheet_);
    ICell::Value result;
    visit([&result](auto val) { result = val; }, formula_value);
    return result;
  } else {
    string_view view = get<string>(data_);
    if (view.size() > 0 && view[0] == kEscapeSign) {
      view = view.substr(1);
    }
    return string(view);
  }
}

std::string Cell::GetText() const {
  return cached_text_;
}

std::vector<Position> Cell::GetReferencedCells() const {
  if (ContainsFormula()) {
    return GetFormula()->GetReferencedCells();
  } else {
    return {};
  }
}

string Cell::BuildText() {
  if (ContainsFormula()) {
    return "=" + GetFormula()->GetExpression();
  } else {
    string_view view = get<string>(data_);
    return string(view);
  }
}

void Cell::RebuildText(IFormula::HandlingResult result) {
  if (result == IFormula::HandlingResult::NothingChanged) return;

  cached_text_ = BuildText();    
}

IFormula::HandlingResult Cell::HandleInsertedRows(int before, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleInsertedRows(before, count);
  RebuildText(result);
  return result;
}

IFormula::HandlingResult Cell::HandleInsertedCols(int before, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleInsertedCols(before, count);
  RebuildText(result);
  return result;
}

IFormula::HandlingResult Cell::HandleDeletedRows(int first, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleDeletedRows(first, count);
  RebuildText(result);
  return result;
}

IFormula::HandlingResult Cell::HandleDeletedCols(int first, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleDeletedCols(first, count);
  RebuildText(result);
  return result;
}

bool Cell::ContainsFormula() const { return holds_alternative<unique_ptr<IFormula>>(data_); }

const IFormula* Cell::GetFormula() const { return get<unique_ptr<IFormula>>(data_).get(); }
IFormula* Cell::GetFormula() { return get<unique_ptr<IFormula>>(data_).get(); }