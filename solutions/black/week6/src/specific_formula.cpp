#include "specific_formula.h"

#include <memory>

#include "FormulaLexer.h"
#include "FormulaListener.h"
#include "FormulaParser.h"
#include "antlr4-runtime.h"
#include "utils.h"

using namespace std;

namespace {
  class BailErrorListener : public antlr4::BaseErrorListener {
   public:
    void syntaxError(antlr4::Recognizer* /* recognizer */, antlr4::Token* /* offendingSymbol */, size_t /* line */,
                     size_t /* charPositionInLine */, const std::string& msg, std::exception_ptr /* e */
                     ) override {
      throw FormulaException("Error when lexing: " + msg);
    }
  };

  bool check_parens_needed(const FormulaParser::ParensContext* ctx) { return true; }

  class SpecificFormulaListener : public FormulaListener {
    stack<unique_ptr<Ast::Statement>> statement_;
    vector<Position> references_;

    virtual void enterMain(FormulaParser::MainContext* /*ctx*/) override {}
    virtual void exitMain(FormulaParser::MainContext* /*ctx*/) override {}

    virtual void enterUnaryOp(FormulaParser::UnaryOpContext* /*ctx*/) override {}
    virtual void exitUnaryOp(FormulaParser::UnaryOpContext* ctx) override {
      auto unary_op = make_unique<Ast::UnaryOperationStatement>();
      unary_op->op_type = ctx->ADD() ? Ast::OperationType::Add : Ast::OperationType::Sub;
      unary_op->rhs = move(statement_.top());
      statement_.pop();
      statement_.push(move(unary_op));
    }

    virtual void enterParens(FormulaParser::ParensContext* /*ctx*/) override {}
    virtual void exitParens(FormulaParser::ParensContext* ctx) override {
      if (check_parens_needed(ctx)) {
        auto parens = make_unique<Ast::ParensStatement>();
        parens->statement = move(statement_.top());
        statement_.pop();
        statement_.push(move(parens));
      }
    }

    virtual void enterLiteral(FormulaParser::LiteralContext* /*ctx*/) override {}
    virtual void exitLiteral(FormulaParser::LiteralContext* ctx) override {
      auto literal = make_unique<Ast::ValueStatement>();
      literal->data = stod(ctx->NUMBER()->getText());
      statement_.push(move(literal));
    }

    virtual void enterCell(FormulaParser::CellContext* /*ctx*/) override {}
    virtual void exitCell(FormulaParser::CellContext* ctx) override {
      auto cell = make_unique<Ast::CellStatement>();
      cell->pos = Position::FromString(ctx->CELL()->getText());
      references_.push_back(cell->pos);
      statement_.push(move(cell));
    }

    virtual void enterBinaryOp(FormulaParser::BinaryOpContext* /*ctx*/) override {}
    virtual void exitBinaryOp(FormulaParser::BinaryOpContext* ctx) override {
      auto rhs = move(statement_.top());
      statement_.pop();
      auto lhs = move(statement_.top());
      statement_.pop();

      using OPType = Ast::OperationType;
      OPType op_type;

      if (ctx->ADD()) {
        op_type = OPType::Add;
      } else if (ctx->SUB()) {
        op_type = OPType::Sub;
      } else if (ctx->DIV()) {
        op_type = OPType::Div;
      } else {
        op_type = OPType::Mul;
        ;
      }

      auto binary_op = make_unique<Ast::BinaryOperationStatement>();
      binary_op->lhs = move(lhs);
      binary_op->rhs = move(rhs);
      binary_op->op_type = op_type;
      statement_.push(move(binary_op));
    }

    virtual void enterEveryRule(antlr4::ParserRuleContext* /*ctx*/) override {}
    virtual void exitEveryRule(antlr4::ParserRuleContext* /*ctx*/) override {}
    virtual void visitTerminal(antlr4::tree::TerminalNode* /*node*/) override {}
    virtual void visitErrorNode(antlr4::tree::ErrorNode* /*node*/) override {}

   public:
    unique_ptr<Ast::Statement> TakeStatement() { return move(statement_.top()); }
    vector<Position> GetReferences() {
      sort(references_.begin(), references_.end());
      references_.erase(unique(references_.begin(), references_.end()), references_.end());
      return references_;
    }
  };
}  // namespace

SpecificFormula::SpecificFormula(string expression) {
  antlr4::ANTLRInputStream input(expression);

  FormulaLexer lexer(&input);
  BailErrorListener error_listener;
  lexer.removeErrorListeners();
  lexer.addErrorListener(&error_listener);

  antlr4::CommonTokenStream tokens(&lexer);

  FormulaParser parser(&tokens);
  auto error_handler = std::make_shared<antlr4::BailErrorStrategy>();
  parser.setErrorHandler(error_handler);
  parser.removeErrorListeners();

  antlr4::tree::ParseTree* tree = parser.main();  // метод соответствует корневому правилу
  SpecificFormulaListener listener;
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

  statement_ = Ast::RemoveUnnecessaryParens(listener.TakeStatement());
  references_ = listener.GetReferences();
  for (auto& ref : references_) {
    if (!ref.IsValid()) {
      throw FormulaException("invalid ref in formula");
    }
  }
}

IFormula::Value SpecificFormula::Evaluate(const ISheet& sheet) const { return statement_->Evaluate(sheet); }

std::string SpecificFormula::GetExpression() const { return statement_->ToString(); }

std::vector<Position> SpecificFormula::GetReferencedCells() const { return references_; }

IFormula::HandlingResult SpecificFormula::HandleInsertedRows(int before, int count) {
  auto result = PositionModifiers::HandleInsertedRows(references_, before, count);

  if (result != IFormula::HandlingResult::NothingChanged) {
    Ast::ModifyCellStatements(statement_.get(), [before, count](Position& pos) {
      PositionModifiers::HandleInsertedRows(pos, before, count);
    });
  }

  return result;
}
IFormula::HandlingResult SpecificFormula::HandleInsertedCols(int before, int count) {
  auto result = PositionModifiers::HandleInsertedCols(references_, before, count);

  if (result != IFormula::HandlingResult::NothingChanged) {
    Ast::ModifyCellStatements(statement_.get(), [before, count](Position& pos) {
      PositionModifiers::HandleInsertedCols(pos, before, count);
    });
  }

  return result;
}
IFormula::HandlingResult SpecificFormula::HandleDeletedRows(int first, int count) {
  auto result = PositionModifiers::HandleDeletedRows(references_, first, count);

  if (result != HandlingResult::NothingChanged) {
    Ast::ModifyCellStatements(statement_.get(), [first, count](Position& pos) {
      PositionModifiers::HandleDeletedRows(pos, first, count);
    });
  }

  return result;
}
IFormula::HandlingResult SpecificFormula::HandleDeletedCols(int first, int count) {
  auto result = PositionModifiers::HandleDeletedCols(references_, first, count);

  if (result != HandlingResult::NothingChanged) {
    Ast::ModifyCellStatements(statement_.get(), [first, count](Position& pos) {
      PositionModifiers::HandleDeletedCols(pos, first, count);
    });
  }

  return result;
}