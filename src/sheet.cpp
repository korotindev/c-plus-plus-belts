#include "sheet.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

namespace {
  void validate_position(Position pos) {
    if (!pos.IsValid()) {
      throw InvalidPositionException("invalid position");
    }
  }

  enum class DFSStatus { NotVisited, Processing, Visited };

  void ValidateNoCycles(const Sheet& sheet, Position pos, unordered_map<string, DFSStatus>& visited) {
    auto& mark = visited[pos.ToString()];
    if (mark == DFSStatus::Processing) {
      throw CircularDependencyException("cycles not allowed: " + pos.ToString());
    }
    mark = DFSStatus::Processing;

    if (auto ptr = sheet.GetCell(pos)) {
      for (auto ref : ptr->GetReferencedCells()) {
        auto status = visited[ref.ToString()];
        if (status == DFSStatus::Visited) continue;
        ValidateNoCycles(sheet, ref, visited);
      }
    }

    mark = DFSStatus::Visited;
  }

  void ValidateNoCyclesAfterInsertion(const Sheet& sheet, Position pos, const ICell* ptr) {
    unordered_map<string, DFSStatus> visited;
    visited[pos.ToString()] = DFSStatus::Processing;

    for (auto ref : ptr->GetReferencedCells()) {
      auto status = visited[ref.ToString()];
      if (status == DFSStatus::Visited) continue;
      ValidateNoCycles(sheet, ref, visited);
    }
  }
}  // namespace

void Sheet::ExpandSize(Position pos) {
  size_t new_rows = max(static_cast<size_t>(pos.row + 1), cells.size());
  cells.resize(new_rows);

  size_t new_cols = max(static_cast<size_t>(pos.col + 1), cells[0].size());
  for (auto& row : cells) {
    row.resize(new_cols);
  }
}

bool Sheet::AccessablePosition(Position pos) const {
  return static_cast<size_t>(pos.row) < cells.size() && static_cast<size_t>(pos.col) < cells[0].size();
}

void Sheet::SetCell(Position pos, std::string text) {
  validate_position(pos);
  ExpandSize(pos);
  auto cell_ptr = make_unique<Cell>(this, move(text));
  ValidateNoCyclesAfterInsertion(*this, pos, cell_ptr.get());
  for (auto ref : cell_ptr->GetReferencedCells()) {
    if (!GetCell(ref)) {
      SetCell(ref, "");
    }
  }
  cells[pos.row][pos.col] = move(cell_ptr);
}

const ICell* Sheet::GetCell(Position pos) const {
  validate_position(pos);
  if (AccessablePosition(pos)) {
    return cells[pos.row][pos.col].get();
  }
  return nullptr;
}

ICell* Sheet::GetCell(Position pos) {
  validate_position(pos);
  if (AccessablePosition(pos)) {
    return cells[pos.row][pos.col].get();
  }
  return nullptr;
}

void Sheet::ClearCell(Position pos) {
  validate_position(pos);
  if (AccessablePosition(pos)) {
    cells[pos.row][pos.col] = nullptr;
  }
}

void Sheet::InsertRows(int before, int count) {
  if (GetPrintableSize().rows + count >= Position::kMaxCols) {
    throw TableTooBigException("cell col ref overflow");
  }
  for (const auto& row : cells) {
    for (const auto& cell : row) {
      if (!cell) {
        continue;
      }
      for (const auto ref : cell->GetReferencedCells()) {
        if (ref.row + count >= Position::kMaxCols) {
          throw TableTooBigException("cell row ref overflow");
        }
      }
    }
  }

  for (auto& row : cells) {
    for (auto& cell : row) {
      if (!cell) {
        continue;
      }
      if (cell->ContainsFormula()) {
        cell->GetFormula()->HandleInsertedRows(before, count);
      }
    }
  }

  cells.insert(cells.begin() + before, count, vector(cells[0].size(), shared_ptr<Cell>()));
}

void Sheet::InsertCols(int before, int count) {
  if (GetPrintableSize().cols + count >= Position::kMaxCols) {
    throw TableTooBigException("cell col ref overflow");
  }
  for (const auto& row : cells) {
    for (const auto& cell : row) {
      if (!cell) {
        continue;
      }
      for (const auto ref : cell->GetReferencedCells()) {
        if (ref.col + count >= Position::kMaxCols) {
          throw TableTooBigException("cell col ref overflow");
        }
      }
    }
  }

  for (auto& row : cells) {
    for (auto& cell : row) {
      if (!cell) {
        continue;
      }
      if (cell->ContainsFormula()) {
        cell->GetFormula()->HandleInsertedCols(before, count);
      }
    }
    row.insert(row.begin() + before, count, shared_ptr<Cell>());
  }
}

void Sheet::DeleteRows(int first, int count) {
  cells.erase(cells.begin() + first, cells.begin() + first + count);
  for (auto& row : cells) {
    for (auto& cell : row) {
      if (!cell) {
        continue;
      }
      if (cell->ContainsFormula()) {
        cell->GetFormula()->HandleDeletedRows(first, count);
      }
    }
  }
}

void Sheet::DeleteCols(int first, int count) {
  for (auto& row : cells) {
    row.erase(row.begin() + first, row.begin() + first + count);
    for (auto& cell : row) {
      if (!cell) {
        continue;
      }
      if (cell->ContainsFormula()) {
        cell->GetFormula()->HandleDeletedCols(first, count);
      }
    }
  }
}

pair<Position, Position> Sheet::GetPrintableArea() const {
  Position top_left{0, 0};
  Position bottom_right{0, 0};

  for (int i = 0; i < static_cast<int>(cells.size()); i++) {
    for (int j = 0; j < static_cast<int>(cells[0].size()); j++) {
      const auto& cell = cells[i][j];
      if (!cell) continue;
      if (!cell->ContainsFormula() && cell->GetText().empty()) continue;
      if (top_left.IsValid()) {
        // top_left.row = min(top_left.row, i);
        // top_left.col = min(top_left.row, j);
        bottom_right.row = max(bottom_right.row, i + 1);
        bottom_right.col = max(bottom_right.col, j + 1);
      } else {
        // top_left.row = i;
        // top_left.col = j;
        bottom_right.row = i + 1;
        bottom_right.col = j + 1;
      }
    }
  }

  return {top_left, bottom_right};
}

Size Sheet::GetPrintableSize() const {
  auto [top_left, bottom_right] = GetPrintableArea();
  return {bottom_right.row - top_left.row, bottom_right.col - top_left.col};
}

void Sheet::PrintValues(std::ostream& output) const {
  auto [top_left, bottom_right] = GetPrintableArea();
  for (int i = top_left.row; i < bottom_right.row; i++) {
    for (int j = top_left.col; j < bottom_right.col; j++) {
      if (j > 0) output << '\t';
      if (const auto& ptr = cells[i][j]) {
        visit([&output](auto value) { output << value; }, ptr->GetValue());
      }
    }
    output << '\n';
  }
}

void Sheet::PrintTexts(std::ostream& output) const {
  auto [top_left, bottom_right] = GetPrintableArea();
  for (int i = top_left.row; i < bottom_right.row; i++) {
    for (int j = top_left.col; j < bottom_right.col; j++) {
      if (j > 0) output << '\t';
      if (const auto& ptr = cells[i][j]) {
        output << ptr->GetText();
      }
    }
    output << '\n';
  }
}