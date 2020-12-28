#pragma once

#include "formula.h"

class SpecificFormula : public IFormula {
 public:
  Value Evaluate(const ISheet& sheet) const override;
  std::string GetExpression() const override;
  std::vector<Position> GetReferencedCells() const override;
  HandlingResult HandleInsertedRows(int before, int count = 1) override;
  HandlingResult HandleInsertedCols(int before, int count = 1) override;
  HandlingResult HandleDeletedRows(int first, int count = 1) override;
  HandlingResult HandleDeletedCols(int first, int count = 1) override;
};