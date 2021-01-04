#include "statement.h"

#include <cmath>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <cstdlib>

using namespace std;

namespace Ast {
  IFormula::Value ValueStatement::Evaluate(const ISheet&) const { return data; }

  string ValueStatement::ToString() const {
    stringstream ss;
    ss << data;
    return ss.str();
  }

  StatementType ValueStatement::Type() const { return StatementType::Value; }

  string UnaryOperationStatement::ToString() const {
    if (op_type == OperationType::Sub) {
      return "-" + rhs->ToString();
    } else {
      return "+" + rhs->ToString();
    }
  }

  IFormula::Value UnaryOperationStatement::Evaluate(const ISheet& sheet) const {
    auto data = rhs->Evaluate(sheet);
    if (holds_alternative<FormulaError>(data)) {
      return data;
    }

    double res = get<double>(data);
    return op_type == OperationType::Sub ? -res : res;
  }

  StatementType UnaryOperationStatement::Type() const { return StatementType::UnaryOp; }

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

  string BinaryOperationStatement::ToString() const {
    string result = lhs->ToString();

    switch (op_type) {
      case OperationType::Add: {
        result.push_back('+');
        break;
      }
      case OperationType::Sub: {
        result.push_back('-');
        break;
      }
      case OperationType::Mul: {
        result.push_back('*');
        break;
      }
      case OperationType::Div: {
        result.push_back('/');
        break;
      }
      default:
        throw FormulaException("invalid operation type");
    }

    result += rhs->ToString();

    return result;
  }

  StatementType BinaryOperationStatement::Type() const { return StatementType::BinaryOp; }

  IFormula::Value CellStatement::Evaluate(const ISheet& sheet) const {
    auto cell_ptr = sheet.GetCell(pos);
    if (!cell_ptr) {
      return 0;
    }

    const auto cell_val = sheet.GetCell(pos)->GetValue();
    if (holds_alternative<double>(cell_val)) {
      return get<double>(cell_val);
    }

    if (holds_alternative<string>(cell_val)) {
      const auto& str = get<string>(cell_val);

      char *end = nullptr;
      double result = strtod(str.c_str(), &end);
      if (*end != '\0') {
        return FormulaError::Category::Value;
      }

      return result;
    }

    return get<FormulaError>(cell_val);
  }

  string CellStatement::ToString() const {
    if (pos.IsValid()) {
      return pos.ToString();
    }
    return string(FormulaError(FormulaError::Category::Ref).ToString());
  }

  StatementType CellStatement::Type() const { return StatementType::Cell; }

  IFormula::Value ParensStatement::Evaluate(const ISheet& sheet) const { return statement->Evaluate(sheet); }

  string ParensStatement::ToString() const { return "(" + statement->ToString() + ")"; }

  StatementType ParensStatement::Type() const { return StatementType::Parens; }

  namespace {
    bool NeedRemoveParensNode(unique_ptr<Statement>& parent, unique_ptr<Statement>& node) {
      const auto node_type = node->Type();
      if (node_type != StatementType::Parens) {
        return false;
      }

      if (!parent || parent->Type() == StatementType::Parens) {
        return node_type == StatementType::Parens;
      }

      const auto parent_type = parent->Type();
      const auto parens_ptr = dynamic_cast<ParensStatement*>(node.get());
      const auto child_ptr = dynamic_cast<BinaryOperationStatement*>(parens_ptr->statement.get());

      if (!child_ptr) {
        return true;
      }

      if (parent_type == StatementType::UnaryOp) {
        return child_ptr->op_type != OperationType::Add && child_ptr->op_type != OperationType::Sub;
      }

      static const unordered_map<OperationType, int> operations_priority = {
          {OperationType::Add, 10},
          {OperationType::Sub, 20},
          {OperationType::Mul, 30},
          {OperationType::Div, 40},
      };

      const auto parent_ptr = dynamic_cast<BinaryOperationStatement*>(parent.get());
      const int mul_operation_priority = operations_priority.at(OperationType::Mul);
      const int child_operation_priority = operations_priority.at(child_ptr->op_type);
      bool left_child = parent_ptr->lhs.get() == parens_ptr;

      switch (parent_ptr->op_type) {
        case OperationType::Add:
          return true;
        case OperationType::Sub:
          return left_child || child_operation_priority >= mul_operation_priority;
        case OperationType::Mul:
          return child_operation_priority >= mul_operation_priority;
        case OperationType::Div:
          return left_child && child_operation_priority >= mul_operation_priority;
        default:
          throw FormulaException("invalid operation type");
      }
    }

    void RemoveUnnecessaryParens(unique_ptr<Statement>& parent, unique_ptr<Statement>& node) {
      while (NeedRemoveParensNode(parent, node)) {
        auto statement = move(dynamic_cast<ParensStatement*>(node.get())->statement);
        node = move(statement);
      }

      switch (node->Type()) {
        case StatementType::BinaryOp:
          RemoveUnnecessaryParens(node, dynamic_cast<BinaryOperationStatement*>(node.get())->lhs);
          RemoveUnnecessaryParens(node, dynamic_cast<BinaryOperationStatement*>(node.get())->rhs);
          break;
        case StatementType::UnaryOp:
          RemoveUnnecessaryParens(node, dynamic_cast<UnaryOperationStatement*>(node.get())->rhs);
          break;
        case StatementType::Parens:
          RemoveUnnecessaryParens(node, dynamic_cast<ParensStatement*>(node.get())->statement);
          break;
        default:
          break;
      }
    }
  }  // namespace

  unique_ptr<Statement> RemoveUnnecessaryParens(unique_ptr<Statement> root) {
    unique_ptr<Statement> parent;
    RemoveUnnecessaryParens(parent, root);
    return root;
  }

  void ModifyCellStatements(Statement* root, function<void(Position& pos)> func) {
    switch (root->Type()) {
      case StatementType::BinaryOp:
        ModifyCellStatements(dynamic_cast<BinaryOperationStatement*>(root)->lhs.get(), func);
        ModifyCellStatements(dynamic_cast<BinaryOperationStatement*>(root)->rhs.get(), func);
        break;
      case StatementType::UnaryOp:
        ModifyCellStatements(dynamic_cast<UnaryOperationStatement*>(root)->rhs.get(), func);
        break;
      case StatementType::Parens:
        ModifyCellStatements(dynamic_cast<ParensStatement*>(root)->statement.get(), func);
        break;
      case StatementType::Cell:
        func(dynamic_cast<CellStatement*>(root)->pos);
        break;
      default:
        break;
    }
  }
}  // namespace Ast
