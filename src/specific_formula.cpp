#include "specific_formula.h"

#include "FormulaLexer.h"
#include "FormulaListener.h"
#include "FormulaParser.h"
#include "antlr4-runtime.h"

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
    stack<unique_ptr<Ast::Statement>> result;
    virtual void enterMain(FormulaParser::MainContext* /*ctx*/) override {}
    virtual void exitMain(FormulaParser::MainContext* /*ctx*/) override {}

    virtual void enterUnaryOp(FormulaParser::UnaryOpContext* /*ctx*/) override {}
    virtual void exitUnaryOp(FormulaParser::UnaryOpContext* ctx) override {
      auto unary_op = make_unique<Ast::UnaryOperationStatement>();
      unary_op->op_type = ctx->ADD() ? Ast::OperationType::Add : Ast::OperationType::Sub;
      unary_op->rhs = move(result.top());
      result.pop();
      result.push(move(unary_op));
    }

    virtual void enterParens(FormulaParser::ParensContext* /*ctx*/) override {}
    virtual void exitParens(FormulaParser::ParensContext* ctx) override {
      if (check_parens_needed(ctx)) {
        auto parens = make_unique<Ast::ParensStatement>();
        parens->statement = move(result.top());
        result.pop();
        result.push(move(parens));
      }
    }

    virtual void enterLiteral(FormulaParser::LiteralContext* /*ctx*/) override {}
    virtual void exitLiteral(FormulaParser::LiteralContext* ctx) override {
      auto literal = make_unique<Ast::ValueStatement>();
      literal->data = stod(ctx->NUMBER()->getText());
      result.push(move(literal));
    }

    virtual void enterCell(FormulaParser::CellContext* /*ctx*/) override {}
    virtual void exitCell(FormulaParser::CellContext* ctx) override {
      auto cell = make_unique<Ast::CellStatement>();
      cell->pos = Position::FromString(ctx->CELL()->getText());
      result.push(move(cell));
    }

    virtual void enterBinaryOp(FormulaParser::BinaryOpContext* /*ctx*/) override {}
    virtual void exitBinaryOp(FormulaParser::BinaryOpContext* ctx) override {
      auto rhs = move(result.top());
      result.pop();
      auto lhs = move(result.top());
      result.pop();

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
      result.push(move(binary_op));
    }

    virtual void enterEveryRule(antlr4::ParserRuleContext* /*ctx*/) override {}
    virtual void exitEveryRule(antlr4::ParserRuleContext* /*ctx*/) override {}
    virtual void visitTerminal(antlr4::tree::TerminalNode* /*node*/) override {}
    virtual void visitErrorNode(antlr4::tree::ErrorNode* /*node*/) override {}

   public:
    unique_ptr<Ast::Statement> GetResult() { return move(result.top()); }
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

  statement_ = listener.GetResult();
}

IFormula::Value SpecificFormula::Evaluate(const ISheet& sheet) const { return statement_->Evaluate(sheet); }

std::string SpecificFormula::GetExpression() const { return statement_->ToString(); }

std::vector<Position> SpecificFormula::GetReferencedCells() const { return {}; }

IFormula::HandlingResult SpecificFormula::HandleInsertedRows(int before, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult SpecificFormula::HandleInsertedCols(int before, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult SpecificFormula::HandleDeletedRows(int first, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult SpecificFormula::HandleDeletedCols(int first, int count) {
  return IFormula::HandlingResult::NothingChanged;
}