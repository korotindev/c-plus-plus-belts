#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>
#include <cctype>

using namespace std;

namespace Parse {

bool operator==(const Token& lhs, const Token& rhs) {
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream& operator<<(std::ostream& os, const Token& rhs) {
  using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
  if (rhs.Is<type>()) return os << #type;

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}

Lexer::Lexer(std::istream& input) : tokens_(), input_(input), depth_(0) {}

const Token& Lexer::CurrentToken() const { return tokens_.front(); }

size_t ReadSpacesCount(std::istream& input) {
  size_t spaces_count = 1;
  char c;

  while (input && (c = input.get()) == ' ') {
    spaces_count++;
  }

  if (c != ' ') {
    input.putback(c);
  }

  return spaces_count;
}

Token Lexer::NextSpaceToken(char c) {
  if (c == '\n') {
    tokens_.push(TokenType::Newline());
    return CurrentToken();
  } else {
    size_t spaces_count = ReadSpacesCount(input_);
    size_t new_depth = spaces_count / 2;
    if (depth_ == new_depth) {
      return NextToken();
    } else if (new_depth > depth_) {
      depth_ = new_depth;
      tokens_.push(TokenType::Indent());
      return CurrentToken();
    } else {
      size_t count = depth_ - new_depth;
      for (size_t i = 0; i < count; i++) {
        tokens_.push(TokenType::Dedent());
      }
      depth_ = new_depth;
      return CurrentToken();
    }
  }
}


Token Lexer::NextNumberToken(char c) {
  string s(c, 1);

  while(input_ >> c && isdigit(c)) {
    s.push_back(c);
  }

  if (!isdigit(c)) {
    input_.putback(c);
  }

  tokens_.push(TokenType::Number{atoi(s.data())});
  return CurrentToken();
}

Token Lexer::NextWordToken(char c) {
  string s(c, 1);

  while(input_ >> c && (isalpha(c) || c == '_')) {
    s.push_back(c);
  }

  if (!(isalpha(c) || c == '_')) {
    input_.putback(c);
  }

  Token new_token;

  if (s == "class") {
    new_token = TokenType::Class();
  } else if (s == "return") {
    new_token = TokenType::Return();
  } else if (s == "if") {
    new_token = TokenType::If();
  } else if (s == "else") {
    new_token = TokenType::Else();
  } else if (s == "def") {
    new_token = TokenType::Def();
  } else if (s == "print") {
    new_token = TokenType::Print();
  } else if (s == "or") {
    new_token = TokenType::Or();
  } else if (s == "None") {
    new_token = TokenType::None();
  } else if (s == "and") {
    new_token = TokenType::And();
  } else if (s == "not") {
    new_token = TokenType::Not();
  } else if (s == "True") {
    new_token = TokenType::True();
  } else if (s == "False") {
    new_token = TokenType::False();
  }
  return CurrentToken();
}

Token Lexer::NextSymbolToken(char c) {

  
}

Token Lexer::NextToken() {
  if (!tokens_.empty()) {
    if (!CurrentToken().Is<TokenType::Eof>()) {
      tokens_.pop();
    }
    return CurrentToken();
  } else if (!input_) {
    tokens_.push(TokenType::Eof());
    return CurrentToken();
  }

  char c = input_.get();

  if (isalpha(c) || c == '_') {
    return NextWordToken(c);
  } else if (isdigit(c)) {
    return NextNumberToken(c);
  } else if (isspace(c)) {
    return NextSpaceToken(c);
  } else {
    return TokenType::Char{c};
  }
}

} /* namespace Parse */