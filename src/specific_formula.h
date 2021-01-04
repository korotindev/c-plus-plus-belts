#pragma once

#include <string>

#include "formula.h"
#include "statement.h"
#include "common.h"

class SpecificFormula : public IFormula {
 public:
  SpecificFormula(std::string expression);
  Value Evaluate(const ISheet& sheet) const override;
  std::string GetExpression() const override;
  std::vector<Position> GetReferencedCells() const override;
  HandlingResult HandleInsertedRows(int before, int count = 1) override;
  HandlingResult HandleInsertedCols(int before, int count = 1) override;
  HandlingResult HandleDeletedRows(int first, int count = 1) override;
  HandlingResult HandleDeletedCols(int first, int count = 1) override;

 private:
  std::unique_ptr<Ast::Statement> statement_;
  std::vector<Position> references_;
};