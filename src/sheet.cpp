#include "sheet.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

namespace {
  template <typename T>
  void PushElementsApart(vector<T>& vec, int before, int count) {
    int i = vec.size() - 1;
    while (i - count >= before) {
      vec[i] = move(vec[i - count]);
      i--;
    }
  }

  template <typename T>
  void PushElementsCloser(vector<T>& vec, int first, int count) {
    for (size_t i = first; i + count < vec.size(); i++) {
      vec[i] = move(vec[i + count]);
    }
  }

  void ValidatePosition(Position pos) {
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

  void ValidateInsertionStat(const vector<int>& stat, int before, int count, string_view err_msg) {
    auto last_non_zero_it = find_if(stat.rbegin(), stat.rend(), [](int x) { return x != 0; });
    if (last_non_zero_it == stat.rend()) {
      return;
    }

    auto last_non_zero_idx = distance(stat.begin(), last_non_zero_it.base());

    if (last_non_zero_idx + static_cast<size_t>(count) >= stat.size()) {
      throw TableTooBigException(to_string(last_non_zero_idx) + ":" + string(err_msg));
    }
  }
}  // namespace

Sheet::Sheet() {
  data.resize(Position::kMaxRows);
  ClearCellsStat();
}

void Sheet::ClearCellsStat() {
  row_stat.assign(Position::kMaxRows, 0);
  col_stat.assign(Position::kMaxCols, 0);
}

void Sheet::CollectCellStat(Position pos) {
  col_stat[pos.col]++;
  row_stat[pos.row]++;
}

void Sheet::RemoveCellFromStat(Position pos) {
  col_stat[pos.col]--;
  row_stat[pos.row]--;
}

void Sheet::ExpandRow(Position pos) { data[pos.row].resize(Position::kMaxCols); }

bool Sheet::AccessablePosition(Position pos) const { return static_cast<size_t>(pos.col) < data[pos.row].size(); }

void Sheet::SetCell(Position pos, std::string text) {
  ValidatePosition(pos);
  ExpandRow(pos);
  auto cell_ptr = make_unique<Cell>(this, move(text));
  ValidateNoCyclesAfterInsertion(*this, pos, cell_ptr.get());
  for (auto ref : cell_ptr->GetReferencedCells()) {
    if (!GetCell(ref)) {
      SetCell(ref, "");
    }
  }
  auto &cell = data[pos.row][pos.col];
  if (!cell) {
    CollectCellStat(pos);
  }
  cell = move(cell_ptr);
}

const ICell* Sheet::GetCell(Position pos) const {
  ValidatePosition(pos);
  if (AccessablePosition(pos)) {
    return data[pos.row][pos.col].get();
  }
  return nullptr;
}

ICell* Sheet::GetCell(Position pos) {
  ValidatePosition(pos);
  if (AccessablePosition(pos)) {
    return data[pos.row][pos.col].get();
  }
  return nullptr;
}

void Sheet::ClearCell(Position pos) {
  ValidatePosition(pos);
  if (AccessablePosition(pos)) {
    data[pos.row][pos.col] = nullptr;
  }
  RemoveCellFromStat(pos);
}

void Sheet::IterateOverTableRows(function<void(Row& row, size_t row_id)> func) {
  for (size_t i = 0; i < data.size(); i++) {
    func(data[i], i);
  }
}

void Sheet::IterateOverTableCells(function<void(CellPtr& cell, Position pos)> func) {
  IterateOverTableRows([func](Row& row, size_t row_id) {
    for (size_t i = 0; i < row.size(); i++) {
      func(row[i], {static_cast<int>(row_id), static_cast<int>(i)});
    }
  });
}

void Sheet::ValidateRowsInsertion(int before, int count) const {
  ValidateInsertionStat(row_stat, before, count, "error in row");
}

void Sheet::ValidateColsInsertion(int before, int count) const {
  ValidateInsertionStat(col_stat, before, count, "error in col");
}

void Sheet::InsertRows(int before, int count) {
  ValidateRowsInsertion(before, count);
  PushElementsApart(data, before, count);
  ClearCellsStat(); 

  IterateOverTableCells([this, before, count](CellPtr& cell, Position pos) {
    if (!cell) return;
    CollectCellStat(pos);
    if (!cell->ContainsFormula()) return;
    cell->GetFormula()->HandleInsertedRows(before, count);
  });
}

void Sheet::InsertCols(int before, int count) {
  ValidateColsInsertion(before, count);
  IterateOverTableRows([before, count](Row& row, size_t){
    if (row.empty()) return;
    PushElementsApart(row, before, count);
  });
  ClearCellsStat();

  IterateOverTableCells([this, before, count](CellPtr& cell, Position pos) {
    if (!cell) return;
    CollectCellStat(pos);
    if (!cell->ContainsFormula()) return;
    cell->GetFormula()->HandleInsertedCols(before, count);
  });
}

void Sheet::DeleteRows(int first, int count) {
  PushElementsCloser(data, first, count);
  ClearCellsStat();

  IterateOverTableCells([this, first, count](CellPtr& cell, Position pos) {
    if (!cell) return;
    CollectCellStat(pos);
    if (!cell->ContainsFormula()) return;
    cell->GetFormula()->HandleDeletedRows(first, count);
  });
}

void Sheet::DeleteCols(int first, int count) {
  IterateOverTableRows([first, count](Row& row, size_t) {
    if (row.empty()) return;
    PushElementsCloser(row, first, count);
  });
  ClearCellsStat();

  IterateOverTableCells([this, first, count](CellPtr& cell, Position pos) {
    if (!cell) return;
    CollectCellStat(pos);
    if (!cell->ContainsFormula()) return;
    cell->GetFormula()->HandleDeletedCols(first, count);
  });
}

pair<Position, Position> Sheet::GetPrintableArea() const {
  Position top_left{0, 0};
  Position bottom_right{0, 0};

  for (int i = 0; i < static_cast<int>(data.size()); i++) {
    for (int j = 0; j < static_cast<int>(data[i].size()); j++) {
      const auto& cell = data[i][j];
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
      if (data[i].size() <= static_cast<size_t>(j)) continue;
      if (const auto& ptr = data[i][j]) {
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
      if (data[i].size() <= static_cast<size_t>(j)) continue;
      if (const auto& ptr = data[i][j]) {
        output << ptr->GetText();
      }
    }
    output << '\n';
  }
}