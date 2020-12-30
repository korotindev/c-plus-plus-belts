#pragma once

#include "common.h"
#include "formula.h"
#include <variant>

class Sheet;

class Cell : public ICell {
  const Sheet& sheet_;
  std::variant<std::string, std::unique_ptr<IFormula>> data_;
 public:
  Cell(const Sheet& sheet, std::string text);
  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;
};

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
  std::vector<std::vector<std::shared_ptr<Cell>>> cels;
};