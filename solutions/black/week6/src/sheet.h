#pragma once

#include "common.h"
#include "formula.h"
#include "cell.h"
#include <variant>
#include <functional>

class Sheet : public ISheet {
 public:
  using CellPtr = std::unique_ptr<Cell>;
  using Row = std::vector<CellPtr>;
  Sheet();
  void SetCell(Position pos, std::string text) override;
  const ICell* GetCell(Position pos) const override;
  ICell* GetCell(Position pos) override;
  void ClearCell(Position pos) override;
  void InsertRows(int before, int count = 1) override;
  void InsertCols(int before, int count = 1) override;
  void DeleteRows(int first, int count = 1) override;
  void DeleteCols(int first, int count = 1) override;
  Size GetPrintableSize() const override;
  void PrintValues(std::ostream& output) const override;
  void PrintTexts(std::ostream& output) const override;

 private:
  inline const CellPtr& GetCellImpl(Position pos) const;
  inline CellPtr& GetCellImpl(Position pos);
  bool AccessablePosition(Position pos) const;
  void ExpandRow(Position pos);
  void IterateOverTableRows(std::function<void(Row& row, size_t row_id)> f);
  void IterateOverTableCells(std::function<void(CellPtr& ptr, Position pos)> f);
  void ValidateRowsInsertion(int before, int count) const;
  void ValidateColsInsertion(int before, int count) const;
  void ClearCellsStat();
  void CollectCellStat(Position pos);
  void RemoveCellFromStat(Position pos);
  void InvalidateCache(Position pos);
  void FitRows();

  std::vector<Row> data;
  std::vector<int> row_stat;
  std::vector<int> col_stat;
};