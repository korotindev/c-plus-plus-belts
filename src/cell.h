#pragma once

#include <memory>
#include <variant>

#include "common.h"
#include "formula.h"

class Cell : public ICell {
  const ISheet* sheet_;
  std::variant<std::string, std::unique_ptr<IFormula>> data_;
  std::string cached_text_;

 public:
  Cell(const ISheet* sheet, std::string text);
  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;

  bool ContainsFormula() const;
  const IFormula* GetFormula() const;

  IFormula::HandlingResult HandleInsertedRows(int before, int count);
  IFormula::HandlingResult HandleInsertedCols(int before, int count);
  IFormula::HandlingResult HandleDeletedRows(int first, int count);
  IFormula::HandlingResult HandleDeletedCols(int first, int count);

 private:
  IFormula* GetFormula();
  std::string BuildText();
  void RebuildText(IFormula::HandlingResult result);
};