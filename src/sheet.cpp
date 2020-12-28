#include "sheet.h"

using namespace std;

static void validate_position(Position pos, Size s) {
  if (pos.IsValid() && pos.col < s.cols && pos.row < s.rows) {
    return;
  }
  throw InvalidPositionException("can't use this position");
}

ICell::Value Cell::GetValue() const { return ""; }
std::string Cell::GetText() const { return ""; }
std::vector<Position> Cell::GetReferencedCells() const { return {}; }

void Sheet::SetCell(Position pos, std::string text) {}
const ICell* Sheet::GetCell(Position pos) const {
  validate_position(pos, size_);
  return cels[pos.row][pos.col].get();
}
ICell* Sheet::GetCell(Position pos) {
  validate_position(pos, size_);
  return cels[pos.row][pos.col].get();
}
void Sheet::ClearCell(Position pos) {
    validate_position(pos, size_);
    cels[pos.row][pos.col] = nullptr;
}
void Sheet::InsertRows(int before, int count) {}
void Sheet::InsertCols(int before, int count) {}
void Sheet::DeleteRows(int first, int count) {}
void Sheet::DeleteCols(int first, int count) {}
Size Sheet::GetPrintableSize() const { return size_; } // not sure, PRINTABLE!!!
void Sheet::PrintValues(std::ostream& output) const {}
void Sheet::PrintTexts(std::ostream& output) const {}