#include "utils.h"
#include <algorithm>

namespace PositionModifiers {
  HandlingResult HandleInsertedRows(Position &pos, int before, int count) {
    if (pos.row >= before) {
      pos.row += count;
      return HandlingResult::ReferencesRenamedOnly;
    }

    return HandlingResult::NothingChanged;
  }

  HandlingResult HandleInsertedRows(std::vector<Position> &positions, int before, int count) {
    HandlingResult result = HandlingResult::NothingChanged;
    for (auto &pos : positions) {
      if (HandleInsertedRows(pos, before, count) == HandlingResult::ReferencesRenamedOnly) {
        result = HandlingResult::ReferencesRenamedOnly;
      }
    }
    return result;
  }

  HandlingResult HandleInsertedCols(Position &pos, int before, int count) {
    if (pos.col >= before) {
      pos.col += count;
      return HandlingResult::ReferencesRenamedOnly;
    }

    return HandlingResult::NothingChanged;
  }

  HandlingResult HandleInsertedCols(std::vector<Position> &positions, int before, int count) {
    HandlingResult result = HandlingResult::NothingChanged;
    for (auto &pos : positions) {
      if (HandleInsertedCols(pos, before, count) == HandlingResult::ReferencesRenamedOnly) {
        result = HandlingResult::ReferencesRenamedOnly;
      }
    }
    return result;
  }

  size_t Hasher::operator()(Position pos) const {
    return pos.row * 100000 + pos.col;
  }
  HandlingResult HandleDeletedRows(Position &pos, int first, int count) {
    if (pos.row >= first + count) {
      pos.row -= count;
      return HandlingResult::ReferencesRenamedOnly;
    }

    if (pos.row >= first) {
      pos.row = -1;
      return HandlingResult::ReferencesChanged;
    }

    return HandlingResult::NothingChanged;
  }
  HandlingResult HandleDeletedRows(std::vector<Position> &positions, int first, int count) {
    HandlingResult result = HandlingResult::NothingChanged;

    for (auto &pos : positions) {
      auto pos_result = HandleDeletedRows(pos, first, count);
      if (result == HandlingResult::NothingChanged || pos_result == HandlingResult::ReferencesChanged) {
        result = pos_result;
      }
    }

    if (result == HandlingResult::ReferencesChanged) {
      positions.erase(remove_if(positions.begin(), positions.end(), [](const Position &pos) { return !pos.IsValid(); }),
                      positions.end());
    }

    return result;
  }

  HandlingResult HandleDeletedCols(Position &pos, int first, int count) {
    if (pos.col >= first + count) {
      pos.col -= count;
      return HandlingResult::ReferencesRenamedOnly;
    }

    if (pos.col >= first) {
      pos.col = -1;
      return HandlingResult::ReferencesChanged;
    }

    return HandlingResult::NothingChanged;
  }
  
  HandlingResult HandleDeletedCols(std::vector<Position> &positions, int first, int count) {
    HandlingResult result = HandlingResult::NothingChanged;

    for (auto &pos : positions) {
      auto pos_result = HandleDeletedCols(pos, first, count);
      if (result == HandlingResult::NothingChanged || pos_result == HandlingResult::ReferencesChanged) {
        result = pos_result;
      }
    }

    if (result == HandlingResult::ReferencesChanged) {
      positions.erase(remove_if(positions.begin(), positions.end(), [](const Position &pos) { return !pos.IsValid(); }),
                      positions.end());
    }

    return result;
  }
}  // namespace PositionModifiers
