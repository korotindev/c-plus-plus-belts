#include "cell.h"

using namespace std;

Cell::Cell(const ISheet* sheet, string text) : sheet_(sheet) {
  if (text.empty() || text[0] != kFormulaSign) {
    data_ = move(text);
  } else {
    data_ = ParseFormula(text.substr(1));
  }
}

ICell::Value Cell::GetValue() const {
  if (holds_alternative<string>(data_)) {
    string_view view = get<string>(data_);
    if (view.size() > 0 && view[0] == kEscapeSign) {
      view = view.substr(1);
    }
    return string(view);
  } else {
    IFormula::Value formula_value = GetFormula()->Evaluate(*sheet_);
    ICell::Value result;
    visit([&result](auto val) { result = val; }, formula_value);
    return result;
  }
}

std::string Cell::GetText() const {
  if (holds_alternative<string>(data_)) {
    string_view view = get<string>(data_);
    return string(view);
  } else {
    return "=" + get<unique_ptr<IFormula>>(data_)->GetExpression();
  }
}

std::vector<Position> Cell::GetReferencedCells() const {
  if (holds_alternative<string>(data_)) {
    return {};
  } else {
    return get<unique_ptr<IFormula>>(data_)->GetReferencedCells();
  }
}

bool Cell::ContainsFormula() const { return holds_alternative<unique_ptr<IFormula>>(data_); }

const IFormula* Cell::GetFormula() const { return get<unique_ptr<IFormula>>(data_).get(); }

IFormula* Cell::GetFormula() { return get<unique_ptr<IFormula>>(data_).get(); }
