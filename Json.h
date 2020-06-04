#ifndef C_PLUS_PLUS_BELTS_JSON_H
#define C_PLUS_PLUS_BELTS_JSON_H

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Json {

  class Node : public std::variant<
    std::vector<Node>,
    std::map<std::string, Node>,
    double,
    bool,
    std::string> {
  public:
    using variant::variant;

    const auto& AsArray() const {
      return std::get<std::vector<Node>>(*this);
    }

    const auto& AsMap() const {
      return std::get<std::map<std::string, Node>>(*this);
    }

    double AsNumber() const {
      return std::get<double>(*this);
    }

    bool AsBool() const {
      return std::get<bool>(*this);
    }

    const auto& AsString() const {
      return std::get<std::string>(*this);
    }
  };

  class Document {
  public:
    explicit Document(Node root);

    const Node& GetRoot() const;

  private:
    Node root;
  };

  Document Load(std::istream& input);

  bool operator==(const Document &lhs, const Document &rhs);
  std::ostream& operator<<(std::ostream& output, const Document &rhs);
  std::ostream& operator<<(std::ostream& output, const Node &data);
}


#endif //C_PLUS_PLUS_BELTS_JSON_H
