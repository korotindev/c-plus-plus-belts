#pragma once
#include <functional>
#include <memory>

#include "common.h"
#include "formula.h"

namespace Ast {
  enum class OperationType { Add, Sub, Mul, Div };
  enum class StatementType { Value, UnaryOp, BinaryOp, Cell, Parens };

  struct Statement {
    virtual ~Statement() = default;
    virtual IFormula::Value Evaluate(const ISheet& sheet) const = 0;
    virtual std::string ToString() const = 0;
    virtual StatementType Type() const = 0;
  };

  struct ValueStatement : public Statement {
    double data;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
    StatementType Type() const override;
  };

  struct UnaryOperationStatement : public Statement {
    std::unique_ptr<Statement> rhs;
    OperationType op_type;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
    StatementType Type() const override;
  };

  struct BinaryOperationStatement : public Statement {
    std::unique_ptr<Statement> lhs;
    std::unique_ptr<Statement> rhs;
    OperationType op_type;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
    StatementType Type() const override;
  };

  struct CellStatement : public Statement {
    Position pos;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
    StatementType Type() const override;
  };

  struct ParensStatement : public Statement {
    std::unique_ptr<Statement> statement;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
    StatementType Type() const override;
  };

  std::unique_ptr<Statement> RemoveUnnecessaryParens(std::unique_ptr<Statement> root);
  void ModifyCellStatements(Statement* root, std::function<void(Position& pos)> func);
}  // namespace Ast