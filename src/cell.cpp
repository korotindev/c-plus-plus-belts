#include "cell.h"

using namespace std;

Cell::Cell(const ISheet* sheet, string text) : sheet_(sheet) {
  if (text.empty() || text[0] != kFormulaSign) {
    raw_text_ = move(text);
  } else {
    formula_ = ParseFormula(text.substr(1));
    raw_text_ = '=' + formula_->GetExpression();
  }
}

ICell::Value Cell::GetValue() const {
  if (ContainsFormula()) {
    if (!IsCached()) {
      auto result = formula_->Evaluate(*sheet_);
      visit([this](auto& elem) { cached_formula_value_ = move(elem); }, result);
    }
    return cached_formula_value_.value();
  } else {
    string_view sv = raw_text_;
    if (sv.size() > 0 && sv[0] == kEscapeSign) {
      sv = sv.substr(1);
    }
    return string(sv);
  }
}

std::string Cell::GetText() const { return raw_text_; }

std::vector<Position> Cell::GetReferencedCells() const {
  if (ContainsFormula()) {
    return GetFormula()->GetReferencedCells();
  } else {
    return {};
  }
}

void Cell::RebuildText(IFormula::HandlingResult result) {
  if (result == IFormula::HandlingResult::NothingChanged) return;

  raw_text_ = '=' + formula_->GetExpression();
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

bool Cell::ContainsFormula() const { return formula_ != nullptr; }

const IFormula* Cell::GetFormula() const { return formula_.get(); }

IFormula* Cell::GetFormula() { return formula_.get(); }

void Cell::AddExternalDep(Position pos) { external_deps_.insert(pos); }

void Cell::RemoveExternalDep(Position pos) { external_deps_.erase(pos); }

const PositionSet& Cell::ExternalDeps() const { return external_deps_; }

bool Cell::IsCached() const { return cached_formula_value_.has_value() || formula_ == nullptr; }

void Cell::InvalidateCache() { cached_formula_value_.reset(); }

void Cell::RebuildExternalDepsWith(function<void(vector<Position>&)> func) {
  vector<Position> positions(external_deps_.begin(), external_deps_.end());
  external_deps_.clear();
  func(positions);
  external_deps_ = PositionSet(positions.begin(), positions.end());
}
