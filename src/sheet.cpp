#include "sheet.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

namespace {
  template <typename T>
  void PushElementsApart(vector<T>& vec, int before, int count) {
    if (static_cast<size_t>(before) >= vec.size()) return;
    vector<T> tmp(count);
    vec.insert(vec.begin() + before, move_iterator(tmp.begin()), move_iterator(tmp.end()));
  }

  template <typename T>
  void PushElementsCloser(vector<T>& vec, int first, int count) {
    vec.erase(vec.begin() + first, vec.begin() + first + count);
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

  void ValidateNoSelfLinks(Position pos, const ICell* cell) {
    for(auto ref : cell->GetReferencedCells()) {
      if (ref == pos) {
        throw CircularDependencyException("cycles not allowed!");
      }
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

void Sheet::ExpandRow(Position pos) { 
  auto &row = data[pos.row];
  if (row.size() <= static_cast<size_t>(pos.col)) {
    row.resize(pos.col + 1);
  }
}

bool Sheet::AccessablePosition(Position pos) const { return static_cast<size_t>(pos.col) < data[pos.row].size(); }

void Sheet::SetCell(Position pos, std::string text) {
  ValidatePosition(pos);
  ExpandRow(pos);
  auto &prev_cell = GetCellImpl(pos);
  auto new_cell = make_unique<Cell>(this, move(text));

  if (prev_cell) {
    if (prev_cell->GetText() == new_cell->GetText()) {
      return;
    }

    ValidateNoCyclesAfterInsertion(*this, pos, new_cell.get());
    for(auto ref : prev_cell->GetReferencedCells()) {
      GetCellImpl(ref)->RemoveExternalDep(pos);
    }

    for(const auto& ref : prev_cell->ExternalDeps()) {
      new_cell->AddExternalDep(ref);
    }
  } else {
    CollectCellStat(pos);
    ValidateNoSelfLinks(pos, new_cell.get());
  }

  for (auto ref : new_cell->GetReferencedCells()) {
    if (!GetCell(ref)) {
      SetCell(ref, "");
    }
    GetCellImpl(ref)->AddExternalDep(pos);
  }

  InvalidateCache(pos);
  prev_cell = move(new_cell);
}

const ICell* Sheet::GetCell(Position pos) const {
  ValidatePosition(pos);
  if (AccessablePosition(pos)) {
    return GetCellImpl(pos).get();
  }
  return nullptr;
}

ICell* Sheet::GetCell(Position pos) {
  ValidatePosition(pos);
  if (AccessablePosition(pos)) {
    return GetCellImpl(pos).get();
  }
  return nullptr;
}

void Sheet::ClearCell(Position pos) {
  ValidatePosition(pos);
  if (AccessablePosition(pos)) {
    GetCellImpl(pos) = nullptr;
    RemoveCellFromStat(pos);
  }
  if (row_stat[pos.row] == 0) {
    data[pos.row].clear();
  }
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
    cell->HandleInsertedRows(before, count);
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
    cell->HandleInsertedCols(before, count);
  });
}

void Sheet::DeleteRows(int first, int count) {
  PushElementsCloser(data, first, count);
  ClearCellsStat();

  IterateOverTableCells([this, first, count](CellPtr& cell, Position pos) {
    if (!cell) return;
    CollectCellStat(pos);
    cell->HandleDeletedRows(first, count);
  });

  FitRows();
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
    cell->HandleDeletedCols(first, count);
  });

  FitRows();
}

Size Sheet::GetPrintableSize() const {
  Size size;

  for (int i = 0; i < static_cast<int>(data.size()); i++) {
    for (int j = 0; j < static_cast<int>(data[i].size()); j++) {
      const auto& cell = data[i][j];
      if (!cell) continue;
      if (!cell->ContainsFormula() && cell->GetText().empty()) continue;
      size.rows = max(size.rows, i + 1);
      size.cols = max(size.cols, j + 1);
    }
  }

  return size;
}

void Sheet::PrintValues(std::ostream& output) const {
  auto size = GetPrintableSize();
  for (int i = 0; i < size.rows; i++) {
    for (int j = 0; j < size.cols; j++) {
      if (j > 0) output << '\t';
      if (static_cast<size_t>(j) >= data[i].size()) continue;
      if (const auto& ptr = data[i][j]) {
        visit([&output](auto value) { output << value; }, ptr->GetValue());
      }
    }
    output << '\n';
  }
}

void Sheet::PrintTexts(std::ostream& output) const {
  auto size = GetPrintableSize();
  for (int i = 0; i < size.rows; i++) {
    for (int j = 0; j < size.cols; j++) {
      if (j > 0) output << '\t';
      if (static_cast<size_t>(j) >= data[i].size()) continue;
      if (const auto& ptr = data[i][j]) {
        output << ptr->GetText();
      }
    }
    output << '\n';
  }
}

inline const Sheet::CellPtr& Sheet::GetCellImpl(Position pos) const {
  return data[pos.row][pos.col];
}

inline Sheet::CellPtr& Sheet::GetCellImpl(Position pos) { return data[pos.row][pos.col]; }

void Sheet::InvalidateCache(Position pos) {
  if (!AccessablePosition(pos)) {
    return;
  }
  auto& cell_ptr = GetCellImpl(pos);
  if (cell_ptr && cell_ptr->IsCached()) {
    cell_ptr->InvalidateCache();
    for (const auto& ref : cell_ptr->ExternalDeps()) {
      InvalidateCache(ref);
    }
  }
}

void Sheet::FitRows() {
  for(size_t i = 0; i < row_stat.size(); i++) {
    if (row_stat[i] == 0) {
      data[i].clear();
    }
  }
}