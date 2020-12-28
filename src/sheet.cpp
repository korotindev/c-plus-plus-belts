#include "sheet.h"

using namespace std;

ICell::Value Cell::GetValue() const { return ""; }
std::string Cell::GetText() const { return ""; }
std::vector<Position> Cell::GetReferencedCells() const { return {}; }

void Sheet::SetCell(Position pos, std::string text) {}
const ICell* Sheet::GetCell(Position pos) const { return &tmp_cell_; }
ICell* Sheet::GetCell(Position pos) { return &tmp_cell_; }
void Sheet::ClearCell(Position pos) {}
void Sheet::InsertRows(int before, int count) {}
void Sheet::InsertCols(int before, int count) {}
void Sheet::DeleteRows(int first, int count) {}
void Sheet::DeleteCols(int first, int count) {}
Size Sheet::GetPrintableSize() const { return size_; }
void Sheet::PrintValues(std::ostream& output) const {}
void Sheet::PrintTexts(std::ostream& output) const {}