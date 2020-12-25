
// Generated from /workspaces/c-plus-plus-belts/src/grammar/SumMul.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "SumMulListener.h"


namespace DimaGeneratedGrammar {

/**
 * This class provides an empty implementation of SumMulListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SumMulBaseListener : public SumMulListener {
public:

  virtual void enterMain(SumMulParser::MainContext * /*ctx*/) override { }
  virtual void exitMain(SumMulParser::MainContext * /*ctx*/) override { }

  virtual void enterAdd(SumMulParser::AddContext * /*ctx*/) override { }
  virtual void exitAdd(SumMulParser::AddContext * /*ctx*/) override { }

  virtual void enterBrackets(SumMulParser::BracketsContext * /*ctx*/) override { }
  virtual void exitBrackets(SumMulParser::BracketsContext * /*ctx*/) override { }

  virtual void enterNumber(SumMulParser::NumberContext * /*ctx*/) override { }
  virtual void exitNumber(SumMulParser::NumberContext * /*ctx*/) override { }

  virtual void enterMul(SumMulParser::MulContext * /*ctx*/) override { }
  virtual void exitMul(SumMulParser::MulContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

}  // namespace DimaGeneratedGrammar
