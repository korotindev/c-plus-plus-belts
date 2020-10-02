#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Svg {

struct Point {
  double x;
  double y;
};

struct Rgb {
  short red;
  short green;
  short blue;
  std::string str() {
    return "rgb(" + std::to_string(red) + "," + std::to_string(green) + "," + std::to_string(blue) + ")";
  }
};

struct Color {
  Color() : value("none") {}
  Color(std::string value_) : value(move(value_)) {}
  Color(const char *value_) : value(value_) {}
  Color(Rgb rgb) : value(rgb.str()) {}
  const std::string value;
  const std::string &str() { return value; }
};

inline Color NoneColor;

class Document;

class AbstractNode {
  friend Document;

 public:
  virtual ~AbstractNode() = default;
  virtual void Render(std::ostream &out) const = 0;
  virtual std::unique_ptr<AbstractNode> Clone() const = 0;
};

template <class Derived>
class Node : public AbstractNode {
 public:
  Node() { SetFillColor(NoneColor).SetStrokeColor(NoneColor).SetStrokeWidth(1.0); }
  Node(Node &&) = default;
  Node &operator=(Node &&) = default;
  Node(const Node &) = default;
  Node &operator=(const Node &) = default;

  virtual std::unique_ptr<AbstractNode> Clone() const override { return std::make_unique<Derived>(self()); }

  Derived &SetFillColor(const Color &color) {
    properties["fill"] = color.value;
    return self();
  }
  Derived &SetStrokeColor(const Color &color) {
    properties["stroke"] = color.value;
    return self();
  }
  Derived &SetStrokeWidth(double width) {
    properties["stroke-width"] = std::to_string(width);
    return self();
  }
  Derived &SetStrokeLineCap(const std::string &linecap) {
    properties["stroke-linecap"] = linecap;
    return self();
  }
  Derived &SetStrokeLineJoin(const std::string &linejoin) {
    properties["stroke-linejoin"] = linejoin;
    return self();
  }

 protected:
  std::unordered_map<std::string, std::string> properties;
  Derived &self() { return static_cast<Derived &>(*this); }
  const Derived &self() const { return static_cast<const Derived &>(*this); }
};

class Circle : public Node<Circle> {
 public:
  Circle() : Node() { SetCenter(Point{0, 0}).SetRadius(1.0); }

  Circle &SetCenter(Point p) {
    properties["cx"] = std::to_string(p.x);
    properties["cy"] = std::to_string(p.y);
    return *this;
  }
  Circle &SetRadius(double radius) {
    properties["r"] = std::to_string(radius);
    return *this;
  }

  virtual void Render(std::ostream &out) const override {
    out << "<circle ";
    for (const auto &[prop, val] : properties) {
      out << prop << "=\"" << val << "\" ";
    }
    out << "/>";
  }
};

class Polyline : public Node<Polyline> {
 public:
  Polyline() : Node() { properties["points"] = ""; }
  Polyline &AddPoint(Point point) {
    auto it = properties.find("points");
    auto &points = it->second;
    std::string point_str = std::to_string(point.x) + "," + std::to_string(point.y);
    if (points.empty()) {
      points = point_str;
    } else {
      points += " " + point_str;
    }
    return *this;
  }

  virtual void Render(std::ostream &out) const override {
    out << "<polyline ";
    for (const auto &[prop, val] : properties) {
      out << prop << "=\"" << val << "\" ";
    }
    out << "/>";
  }
};

class Text : public Node<Text> {
 public:
  Text() : Node() { SetPoint(Point{}).SetOffset(Point{}).SetFontSize(1); }

  Text &SetPoint(Point point) {
    properties["x"] = std::to_string(point.x);
    properties["y"] = std::to_string(point.y);
    return *this;
  }

  Text &SetOffset(Point point) {
    properties["dx"] = std::to_string(point.x);
    properties["dy"] = std::to_string(point.y);
    return *this;
  }

  Text &SetFontSize(uint32_t size) {
    properties["font-size"] = std::to_string(size);
    return *this;
  }

  Text &SetFontFamily(const std::string &family) {
    properties["font-family"] = family;
    return *this;
  }

  Text &SetData(const std::string &data_) {
    data = data_;
    return *this;
  }

  virtual void Render(std::ostream &out) const override {
    out << "<text ";
    for (const auto &[prop, val] : properties) {
      out << prop << "=\"" << val << "\" ";
    }
    out << ">" << data << "</text>";
  }

 private:
  std::string data;
};

class Document {
 public:
  template <typename NodesItem>
  void Add(NodesItem &&node) {
    auto node_ptr = node.Clone();
    nodes.push_back(move(node_ptr));
  }

  void Render(std::ostream &out) {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
    out << "\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
    out << "\n";
    for (const auto &node : nodes) {
      node->Render(out);
      out << "\n";
    }
    out << "</svg>";
  }

 private:
  std::vector<std::unique_ptr<AbstractNode>> nodes;
};
}  // namespace Svg