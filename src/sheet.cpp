#include "sheet.h"
#include <sstream>

using namespace std;

static void validate_position(Position pos) {
  if (!pos.IsValid()) throw InvalidPositionException("can't use this position");
}


Cell::Cell(const Sheet& sheet, string text) : sheet_(sheet) {
  if (text.empty() || text[0] != kFormulaSign) {
    data_ = move(text);
  } else {
    data_ = ParseFormula(move(text));
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
    IFormula::Value formula_value = get<unique_ptr<IFormula>>(data_)->Evaluate(sheet_);
    stringstream ss;
    visit([&ss](auto val) { ss << val; }, formula_value);
    return ss.str();
  }
}
std::string Cell::GetText() const {
  if (holds_alternative<string>(data_)) {
    string_view view = get<string>(data_);
    return string(view);
  } else {
    return "FORMULA";
  }
}
std::vector<Position> Cell::GetReferencedCells() const { return {}; }

void Sheet::ExpandSize(Position pos) {
  size_.cols = max(pos.col + 1, size_.cols);
  size_.rows = max(pos.row + 1, size_.rows);
  cels.resize(size_.rows);
  for(auto &row : cels) {
    row.resize(size_.cols); 
  }
}

void Sheet::SetCell(Position pos, std::string text) {
  validate_position(pos);
  ExpandSize(pos);
  cels[pos.row][pos.col] = make_unique<Cell>(*this, move(text)); 
}
const ICell* Sheet::GetCell(Position pos) const {
  validate_position(pos);
  return cels[pos.row][pos.col].get();
}
ICell* Sheet::GetCell(Position pos) {
  validate_position(pos);
  return cels[pos.row][pos.col].get();
}
void Sheet::ClearCell(Position pos) {
    validate_position(pos);
    cels[pos.row][pos.col] = nullptr;
}
void Sheet::InsertRows(int before, int count) {}
void Sheet::InsertCols(int before, int count) {}
void Sheet::DeleteRows(int first, int count) {}
void Sheet::DeleteCols(int first, int count) {}
Size Sheet::GetPrintableSize() const { return size_; } // not sure, PRINTABLE!!!
void Sheet::PrintValues(std::ostream& output) const {}
void Sheet::PrintTexts(std::ostream& output) const {}