#pragma once
#include <memory>

#include "common.h"
#include "formula.h"

namespace Ast {
  enum class OperationType { Add, Sub, Mul, Div };

  struct Statement {
    virtual ~Statement() = default;
    virtual IFormula::Value Evaluate(const ISheet& sheet) const = 0;
    virtual std::string ToString() const = 0;
  };

  struct ValueStatement : public Statement {
    double data;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
  };

  struct UnaryOperationStatement : public Statement {
    std::unique_ptr<Statement> rhs;
    OperationType op_type;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
  };

  struct BinaryOperationStatement : public Statement {
    std::unique_ptr<Statement> lhs;
    std::unique_ptr<Statement> rhs;
    OperationType op_type;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
  };

  struct CellStatement : public Statement {
    Position pos;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
  };

  struct ParensStatement : public Statement {
    std::unique_ptr<Statement> statement;
    IFormula::Value Evaluate(const ISheet& sheet) const override;
    std::string ToString() const override;
  };
}  // namespace Ast