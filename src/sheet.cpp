#include "sheet.h"

#include <sstream>

using namespace std;

namespace {
  void validate_position(Position pos) {
    if (!pos.IsValid()) {
      throw InvalidPositionException("invalid position");
    }
  }

  bool accessable_position(Position pos, Size size) { return pos.col < size.cols && pos.row < size.rows; }
}  // namespace

Cell::Cell(const Sheet& sheet, string text) : sheet_(sheet) {
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
    IFormula::Value formula_value = get<unique_ptr<IFormula>>(data_)->Evaluate(sheet_);
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

void Sheet::ExpandSize(Position pos) {
  size_.cols = max(pos.col + 1, size_.cols);
  size_.rows = max(pos.row + 1, size_.rows);
  cells.resize(size_.rows);
  for (auto& row : cells) {
    row.resize(size_.cols);
  }
}

void Sheet::SetCell(Position pos, std::string text) {
  validate_position(pos);
  ExpandSize(pos);
  cells[pos.row][pos.col] = make_unique<Cell>(*this, move(text));
}
const ICell* Sheet::GetCell(Position pos) const {
  validate_position(pos);
  if (accessable_position(pos, size_)) {
    return cells[pos.row][pos.col].get();
  }
  return nullptr;
}
ICell* Sheet::GetCell(Position pos) {
  validate_position(pos);
  if (accessable_position(pos, size_)) {
    return cells[pos.row][pos.col].get();
  }
  return nullptr;
}
void Sheet::ClearCell(Position pos) {
  validate_position(pos);
  if (accessable_position(pos, size_)) {
    cells[pos.row][pos.col] = nullptr;
  }
}
void Sheet::InsertRows(int before, int count) {
  if (size_.rows + count >= Position::kMaxRows) {
    throw TableTooBigException("rows overflow");
  }

  for(const auto& row : cells) {
    for(const auto &cell : row) {
      if (cell) {
        for(const auto ref : cell->GetReferencedCells()) {
          if (ref.row + count >= Position::kMaxCols) {
            throw TableTooBigException("cell row ref overflow");
          } 
        }
      }
    }
  }

  for(auto& row : cells) {
    for(auto &cell : row) {
      if (holds_alternative<unique_ptr<IFormula>>(cell->data_)) {
        get<unique_ptr<IFormula>>(cell->data_)->HandleInsertedRows(before, count);
      }
    }
  }
}
void Sheet::InsertCols(int before, int count) {
  if (size_.cols + count >= Position::kMaxCols) {
    throw TableTooBigException("cols overflow");
  }

  for(const auto& row : cells) {
    for(const auto &cell : row) {
      if (cell) {
        for(const auto ref : cell->GetReferencedCells()) {
          if (ref.col + count >= Position::kMaxCols) {
            throw TableTooBigException("cell col ref overflow");
          } 
        }
      }
    }
  }

  for(auto& row : cells) {
    for(auto &cell : row) {
      if (holds_alternative<unique_ptr<IFormula>>(cell->data_)) {
        get<unique_ptr<IFormula>>(cell->data_)->HandleInsertedCols(before, count);
      }
    }
  }
}
void Sheet::DeleteRows(int first, int count) {}
void Sheet::DeleteCols(int first, int count) {}
Size Sheet::GetPrintableSize() const { return size_; }  // not sure, PRINTABLE!!!
void Sheet::PrintValues(std::ostream& output) const {}
void Sheet::PrintTexts(std::ostream& output) const {}