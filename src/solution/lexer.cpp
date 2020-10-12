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

Lexer::Lexer(std::istream& input) : tokens_(), depth_(0) { Parse(input); }

const Token& Lexer::CurrentToken() const { return tokens_.front(); }

Token Lexer::NextToken() {
  if (!CurrentToken().Is<TokenType::Eof>()) {
    tokens_.pop();
  }
  return CurrentToken();
}

void Lexer::Parse(std::istream& input) {
  string line;
  while (getline(input, line, '\n')) {
    size_t iterations = ParseLine(line);
    if (input && iterations) {
      tokens_.push(TokenType::Newline());
    }
  };

  while(depth_--) {
    tokens_.push(TokenType::Dedent());
  }
  tokens_.push(TokenType::Eof());
}

size_t Lexer::ParseIndent(string_view str) {
  size_t spaces_count = 0;
  while (str[spaces_count] == ' ' && spaces_count < str.size()) {
    spaces_count++;
  }

  size_t new_depth = spaces_count / 2;

  if (depth_ == new_depth || spaces_count == str.size()) {
    return spaces_count;
  }

  if (new_depth > depth_) {
    tokens_.push(TokenType::Indent());
  } else {
    size_t count = depth_ - new_depth;
    while (count--) {
      tokens_.push(TokenType::Dedent());
    }
  }

  depth_ = new_depth;
  return spaces_count;
}

size_t Lexer::ParseWord(string_view str) {
  size_t prefix = 0;
  while (((isalnum(str[prefix]) || str[prefix] == '_')) && prefix < str.size()) {
    prefix++;
  }
  str.remove_suffix(str.size() - prefix);

  Token token;

  if (str == "class") {
    token = TokenType::Class();
  } else if (str == "return") {
    token = TokenType::Return();
  } else if (str == "if") {
    token = TokenType::If();
  } else if (str == "else") {
    token = TokenType::Else();
  } else if (str == "def") {
    token = TokenType::Def();
  } else if (str == "print") {
    token = TokenType::Print();
  } else if (str == "or") {
    token = TokenType::Or();
  } else if (str == "None") {
    token = TokenType::None();
  } else if (str == "and") {
    token = TokenType::And();
  } else if (str == "not") {
    token = TokenType::Not();
  } else if (str == "True") {
    token = TokenType::True();
  } else if (str == "False") {
    token = TokenType::False();
  } else {
    token = TokenType::Id{string(str)};
  }

  tokens_.push(move(token));
  return prefix;
}


size_t Lexer::ParseNumber(string_view str) {
  size_t prefix = 0;
  while (isdigit(str[prefix++]) && prefix < str.size());

  str.remove_suffix(str.size() - prefix);
  tokens_.push(TokenType::Number{atoi(str.data())});

  return prefix;
}


size_t Lexer::ParseString(string_view str) {
  auto prefix_pos = str.find(str[0], 1);
  str.remove_suffix(str.size() - prefix_pos);
  str.remove_prefix(1);
  tokens_.push(TokenType::String{string(str)});
  return prefix_pos + 1;
}

size_t Lexer::ParseSymbol(string_view str) {
  size_t prefix = 1;
  if (prefix < str.size()) {
    prefix = 2;
  }
  string_view tmp = str;
  tmp.remove_suffix(str.size() - prefix);

  Token token;

  if (tmp.size() == 2) {
    if (tmp == ">=") {
      token = TokenType::GreaterOrEq();
    } else if (tmp == "<=") {
      token = TokenType::LessOrEq();
    } else if (tmp == "!=") {
      token = TokenType::NotEq();
    } else if (tmp == "==") {
      token = TokenType::Eq();
    } else {
      prefix = 1;
      token = TokenType::Char{tmp[0]};
    }
  } else {
    token = TokenType::Char{tmp[0]};
  }

  tokens_.push(move(token));
  return prefix;
}

size_t Lexer::ParseLine(string_view str) {
  size_t prefix_size = ParseIndent(str);
  str.remove_prefix(prefix_size);

  size_t iterations = 0;

  while (str.size()) {
    iterations++;
    if (isspace(str[0])) {
      prefix_size = 1;
    } else if (isalpha(str[0]) || str[0] == '_') {
      prefix_size = ParseWord(str);
    } else if (isdigit(str[0])) {
      prefix_size = ParseNumber(str);
    } else if (str[0] == '\'' || str[0] == '\"') {
      prefix_size = ParseString(str);
    } else {
      prefix_size = ParseSymbol(str);
    }
    str.remove_prefix(prefix_size);
  }

  return iterations;
}

} /* namespace Parse */