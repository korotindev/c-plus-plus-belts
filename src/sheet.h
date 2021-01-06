#pragma once

#include "common.h"
#include "formula.h"
#include "cell.h"
#include <variant>

class Sheet : public ISheet {
 public:
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
  void ExpandSize(Position pos);
  Size size_;
  std::vector<std::vector<std::shared_ptr<Cell>>> cells;
};