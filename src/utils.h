#pragma once

#include <vector>
#include <unordered_set>

#include "common.h"
#include "formula.h"

namespace PositionModifiers {
  using HandlingResult = IFormula::HandlingResult;
  HandlingResult HandleInsertedRows(Position &pos, int before, int count);
  HandlingResult HandleInsertedRows(std::vector<Position> &positions, int before, int count);

  HandlingResult HandleInsertedCols(Position &pos, int before, int count);
  HandlingResult HandleInsertedCols(std::vector<Position> &positions, int before, int count);

  HandlingResult HandleDeletedRows(Position &pos, int first, int count);
  HandlingResult HandleDeletedRows(std::vector<Position> &positions, int first, int count);

  HandlingResult HandleDeletedCols(Position &pos, int first, int count);
  HandlingResult HandleDeletedCols(std::vector<Position> &positions, int first, int count);

  struct Hasher {
    size_t operator()(Position pos) const;
  };

}  // namespace PositionModifiers

using PositionSet = std::unordered_set<Position, PositionModifiers::Hasher>;