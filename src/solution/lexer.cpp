#include "lexer.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <unordered_map>

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

Lexer::Lexer(std::istream& input) : current_token_(), current_copies_(0), input_(input), depth_(0) { NextToken(); }

const Token& Lexer::CurrentToken() const { return current_token_; }

size_t ReadSpacesCount(std::istream& input) {
  size_t spaces_count = 1;
  char c;

  while (input.get(c) && c == ' ') {
    spaces_count++;
  }

  if (c != ' ') {
    input.putback(c);
  }

  return spaces_count;
}

Token Lexer::NextSpaceToken(char c) {
  if (c == '\n') {
    current_token_ = TokenType::Newline();
    return CurrentToken();
  } else {
    size_t spaces_count = ReadSpacesCount(input_);
    size_t new_depth = spaces_count / 2;

    if (!current_token_.Is<TokenType::Newline>() || depth_ == new_depth) {
      return NextToken();
    }

    if (new_depth > depth_) {
      current_token_ = TokenType::Indent();
    } else {
      size_t count = depth_ - new_depth - 1;
      current_token_ = TokenType::Dedent();
      current_copies_ = count;
    }

    depth_ = new_depth;
    return CurrentToken();
  }
}

Token Lexer::NextNumberToken(char c) {
  string s{c};

  while (input_.get(c) && isdigit(c)) {
    s.push_back(c);
  }

  if (!isdigit(c)) {
    input_.putback(c);
  }

  current_token_ = TokenType::Number{atoi(s.data())};
  return CurrentToken();
}

Token Lexer::NextWordToken(char c) {
  string s{c};

  while (input_.get(c) && (isalnum(c) || c == '_')) {
    s.push_back(c);
  }

  if (!(isalnum(c) || c == '_')) {
    input_.putback(c);
  }

  if (s == "class") {
    current_token_ = TokenType::Class();
  } else if (s == "return") {
    current_token_ = TokenType::Return();
  } else if (s == "if") {
    current_token_ = TokenType::If();
  } else if (s == "else") {
    current_token_ = TokenType::Else();
  } else if (s == "def") {
    current_token_ = TokenType::Def();
  } else if (s == "print") {
    current_token_ = TokenType::Print();
  } else if (s == "or") {
    current_token_ = TokenType::Or();
  } else if (s == "None") {
    current_token_ = TokenType::None();
  } else if (s == "and") {
    current_token_ = TokenType::And();
  } else if (s == "not") {
    current_token_ = TokenType::Not();
  } else if (s == "True") {
    current_token_ = TokenType::True();
  } else if (s == "False") {
    current_token_ = TokenType::False();
  } else {
    current_token_ = TokenType::Id{move(s)};
  }

  return CurrentToken();
}

Token Lexer::NextSymbolToken(char c) {
  Token token;
  string s{c};
  if (input_.get(c)) {
    s.push_back(c);
  }

  if (s.size() == 2) {
    if (s == ">=") {
      current_token_ = TokenType::GreaterOrEq();
    } else if (s == "<=") {
      current_token_ = TokenType::LessOrEq();
    } else if (s == "!=") {
      current_token_ = TokenType::NotEq();
    } else if (s == "==") {
      current_token_ = TokenType::Eq();
    } else {
      input_.putback(c);
      current_token_ = TokenType::Char{s[0]};
    }
  } else {
    current_token_ = TokenType::Char{c};
  }

  return CurrentToken();
}

Token Lexer::NextToken() {
  if (current_copies_) {
    current_copies_--;
    return CurrentToken();
  }

  char c;

  if (!input_.get(c)) {
    current_token_ = TokenType::Eof();
    return CurrentToken();
  }

  if (isalpha(c) || c == '_') {
    return NextWordToken(c);
  } else if (isdigit(c)) {
    return NextNumberToken(c);
  } else if (isspace(c)) {
    return NextSpaceToken(c);
  } else {
    return NextSymbolToken(c);
  }
}

} /* namespace Parse */