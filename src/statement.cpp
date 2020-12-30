#include "statement.h"

#include <cmath>
#include <variant>

using namespace std;

namespace Ast {
  IFormula::Value ValueStatement::Evaluate(const ISheet&) const { return data; }
  IFormula::Value UnaryOperationStatement::Evaluate(const ISheet& sheet) const {
    auto data = lhs->Evaluate(sheet);
    if (holds_alternative<FormulaError>(data)) {
      return data;
    }

    double res = get<double>(data);
    return op_type == OperationType::Sub ? -res : res;
  }

  IFormula::Value BinaryOperationStatement::Evaluate(const ISheet& sheet) const {
      auto lhs_data = lhs->Evaluate(sheet);
      if (holds_alternative<FormulaError>(lhs_data)) {
        return lhs_data;
      }

      auto rhs_data = rhs->Evaluate(sheet);
      if (holds_alternative<FormulaError>(rhs_data)) {
        return rhs_data;
      }

      double lhs_res = get<double>(lhs_data);
      double rhs_res = get<double>(rhs_data);

      double result;

      switch (op_type) {
        case OperationType::Add: {
          result = lhs_res + rhs_res;
          break;
        }
        case OperationType::Sub: {
          result = lhs_res - rhs_res;
          break;
        }
        case OperationType::Mul: {
          result = lhs_res * rhs_res;
          break;
        }
        case OperationType::Div: {
          result = lhs_res / rhs_res;
          break;
        }
        default:
          throw FormulaException("invalid operation type");
      }

      if (isfinite(result)) {
        return result;
      }

      return FormulaError(FormulaError::Category::Div0);
  }

  IFormula::Value CellStatement::Evaluate(const ISheet& sheet) const {
    auto cell_ptr = sheet.GetCell(pos);
    if (!cell_ptr) {
      return 0;
    }

    const auto cell_val = sheet.GetCell(pos)->GetValue();
    if (holds_alternative<double>(cell_val)) {
      return get<double>(cell_val);
    } else if (holds_alternative<string>(cell_val)) {
      const auto& str = get<string>(cell_val);
      if (str.empty()) {
        return 0;
      } else {
        return stod(str);
      }
    } else {
      return get<FormulaError>(cell_val);
    }

    return 0;
  }

  IFormula::Value ParensStatement::Evaluate(const ISheet& sheet) const {
    return statement->Evaluate(sheet);
  }
}  // namespace Ast