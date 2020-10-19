#include "Json.h"

using namespace std;

namespace Json {

  Document::Document(Node root) : root(move(root)) {}

  const Node &Document::GetRoot() const { return root; }

  Node LoadNode(istream &input);

  Node LoadArray(istream &input) {
    vector<Node> result;

    for (char c; input >> c && c != ']';) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadDouble(istream &input) {
    double result = 0;
    input >> result;
    return Node(result);
  }

  Node LoadBool(istream &input) {
    string line;
    getline(input, line, 'e');
    return Node(line + 'e' == "true");
  }

  Node LoadString(istream &input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream &input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}';) {
      if (c == ',') {
        input >> c;
      }

      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadNode(istream &input) {
    char c;
    input >> c;
    if (c == '[') {
      return LoadArray(input);
    } else if (c == '{') {
      return LoadDict(input);
    } else if (c == '"') {
      return LoadString(input);
    } else if (c == 't' || c == 'f') {
      input.putback(c);
      return LoadBool(input);
    } else {
      input.putback(c);
      return LoadDouble(input);
    }
  }

  Document Load(istream &input) { return Document{LoadNode(input)}; }

  bool operator==(const Document &lhs, const Document &rhs) {
    stringstream outputLhs;
    outputLhs << lhs;
    stringstream outputRhs;
    outputRhs << rhs;
    return outputLhs.str() == outputRhs.str();
  }

  std::ostream &operator<<(std::ostream &output, const Document &data) { return output << data.GetRoot(); }

  void PrintArray(std::ostream &output, const vector<Node> &nodes) {
    output << "[";
    bool first = true;
    for (const auto &node : nodes) {
      if (!first) {
        output << ",";
      }
      first = false;
      output << node;
    }
    output << "]";
  }

  void PrintMap(std::ostream &output, const map<string, Node> &nodes) {
    output << "{";
    bool first = true;
    for (const auto &[key, node] : nodes) {
      if (!first) {
        output << ",";
      }
      first = false;
      output << "\"" << key << "\":" << node;
    }
    output << "}";
  }

  std::ostream &operator<<(std::ostream &output, const Node &data) {
    if (holds_alternative<vector<Node>>(data)) {
      PrintArray(output, data.AsArray());
    } else if (holds_alternative<map<string, Node>>(data)) {
      PrintMap(output, data.AsMap());
    } else if (holds_alternative<size_t>(data)) {
      output << data.AsInteger();
    } else if (holds_alternative<double>(data)) {
      output << data.AsNumber();
    } else if (holds_alternative<bool>(data)) {
      output << data.AsBool();
    } else {
      output << '"' << data.AsString() << '"';
    }
    return output;
  }
}  // namespace Json