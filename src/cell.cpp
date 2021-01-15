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
  if (!IsCached()) {
    if (ContainsFormula()) {
      IFormula::Value formula_value = GetFormula()->Evaluate(*sheet_);
      ICell::Value result;
      visit([&result](auto val) { result = val; }, formula_value);
      cached_value_ = result;
    } else {
      string_view view = get<string>(data_);
      if (view.size() > 0 && view[0] == kEscapeSign) {
        view = view.substr(1);
      }
      cached_value_ = string(view);
    }
  }

  return cached_value_.value();
}

std::string Cell::GetText() const { return cached_text_; }

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
  RebuildExternalDepsWith([before, count](vector<Position>& positions) {
    PositionModifiers::HandleInsertedRows(positions, before, count);
  });
  return result;
}

IFormula::HandlingResult Cell::HandleInsertedCols(int before, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleInsertedCols(before, count);
  RebuildText(result);
  RebuildExternalDepsWith([before, count](vector<Position>& positions) {
    PositionModifiers::HandleInsertedCols(positions, before, count);
  });
  return result;
}

IFormula::HandlingResult Cell::HandleDeletedRows(int first, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleDeletedRows(first, count);
  RebuildText(result);
  RebuildExternalDepsWith(
      [first, count](vector<Position>& positions) { PositionModifiers::HandleDeletedRows(positions, first, count); });
  return result;
}

IFormula::HandlingResult Cell::HandleDeletedCols(int first, int count) {
  if (!ContainsFormula()) return IFormula::HandlingResult::NothingChanged;
  auto result = GetFormula()->HandleDeletedCols(first, count);
  RebuildText(result);
  RebuildExternalDepsWith(
      [first, count](vector<Position>& positions) { PositionModifiers::HandleDeletedCols(positions, first, count); });
  return result;
}

bool Cell::ContainsFormula() const { return holds_alternative<unique_ptr<IFormula>>(data_); }

const IFormula* Cell::GetFormula() const { return get<unique_ptr<IFormula>>(data_).get(); }

IFormula* Cell::GetFormula() { return get<unique_ptr<IFormula>>(data_).get(); }

void Cell::AddExternalDep(Position pos) { external_deps_.insert(pos); }

void Cell::RemoveExternalDep(Position pos) { external_deps_.erase(pos); }

const PositionSet& Cell::ExternalDeps() const { return external_deps_; }

bool Cell::IsCached() const { return cached_value_.has_value(); }

void Cell::InvalidateCache() { cached_value_.reset(); }

void Cell::RebuildExternalDepsWith(function<void(vector<Position>&)> func) {
  vector<Position> positions(external_deps_.begin(), external_deps_.end());
  func(positions);
  external_deps_ = PositionSet(positions.begin(), positions.end());
}
