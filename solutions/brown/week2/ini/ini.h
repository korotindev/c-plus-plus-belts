#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "test_runner.h"

namespace Ini {
std::pair<std::string_view, std::string_view> Split(std::string_view line, char by) {
  size_t pos = line.find(by);
  std::string_view left = line.substr(0, pos);

  if (pos < line.size() && pos + 1 < line.size()) {
    return {left, line.substr(pos + 1)};
  } else {
    return {left, std::string_view()};
  }
}

using Section = std::unordered_map<std::string, std::string>;

class Document {
 public:
  Section &AddSection(std::string s) {
    auto [iter, _] = sections.emplace(std::move(s), Section());
    return GetSection(iter->first);
  }

  Section &GetSection(const std::string &s) { return sections.at(s); }
  const Section &GetSection(const std::string &s) const { return sections.at(s); }

  size_t SectionCount() const { return sections.size(); }

 private:
  std::unordered_map<std::string, Section> sections;
};

Document Load(std::istream &input) {
  Document doc;
  Section *s;
  for (std::string current_line; getline(input, current_line);) {
    if (current_line[0] == '[') {
      s = &doc.AddSection(current_line.substr(1, current_line.size() - 2));
    } else if (!current_line.empty()) {
      auto pair = Split(current_line, '=');
      s->insert(static_cast<std::pair<std::string, std::string>>(pair));
    }
  }
  return doc;
}
}  // namespace Ini