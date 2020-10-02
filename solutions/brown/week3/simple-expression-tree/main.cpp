#include <sstream>

#include "Common.h"
#include "test_runner.h"

using namespace std;

class ValueExpression : public Expression {
 public:
  ValueExpression(int num) : value(num) {}

  int Evaluate() const override { return value; }

  std::string ToString() const override {
    ostringstream out;
    out << value;
    return out.str();
  }

 private:
  int value;
};

ExpressionPtr Value(int num) { return make_unique<ValueExpression>(num); }

class SumExpression : public Expression {
 public:
  SumExpression(ExpressionPtr l, ExpressionPtr r) : left(move(l)), right(move(r)) {}

  int Evaluate() const override { return left->Evaluate() + right->Evaluate(); }

  std::string ToString() const override {
    ostringstream out;
    out << "(" << left->ToString() << ")+(" << right->ToString() << ")";
    return out.str();
  }

 private:
  ExpressionPtr left, right;
};

ExpressionPtr Sum(ExpressionPtr l, ExpressionPtr r) { return make_unique<SumExpression>(move(l), move(r)); }

class ProductExpression : public Expression {
 public:
  ProductExpression(ExpressionPtr l, ExpressionPtr r) : left(move(l)), right(move(r)) {}

  int Evaluate() const override { return left->Evaluate() * right->Evaluate(); }

  std::string ToString() const override {
    ostringstream out;
    out << "(" << left->ToString() << ")*(" << right->ToString() << ")";
    return out.str();
  }

 private:
  ExpressionPtr left, right;
};

ExpressionPtr Product(ExpressionPtr l, ExpressionPtr r) { return make_unique<ProductExpression>(move(l), move(r)); }

string Print(const Expression *e) {
  if (!e) {
    return "Null expression provided";
  }
  stringstream output;
  output << e->ToString() << " = " << e->Evaluate();
  return output.str();
}

void Test() {
  ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
  ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

  ExpressionPtr e2 = Sum(move(e1), Value(5));
  ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

  ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, Test);
  return 0;
}