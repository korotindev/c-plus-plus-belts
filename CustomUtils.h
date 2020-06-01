//
// Created by Дмитрий Коротин on 01.06.2020.
//

#ifndef C_PLUS_PLUS_BELTS_CUSTOMUTILS_H
#define C_PLUS_PLUS_BELTS_CUSTOMUTILS_H

#include <string_view>
#include <optional>
#include <string>
#include <sstream>

template<typename It>
class Range {
public:
    Range(It begin, It end) : begin_(begin), end_(end) {}
    It begin() const { return begin_; }
    It end() const { return end_; }

private:
    It begin_;
    It end_;
};

std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(std::string_view s, std::string_view delimiter = " ") {
    const size_t pos = s.find(delimiter);
    if (pos == s.npos) {
        return {s, std::nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimiter.length())};
    }
}

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimiter = " ") {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    return {lhs, rhs_opt.value_or("")};
}

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ") {
    const auto [lhs, rhs] = SplitTwo(s, delimiter);
    s = rhs;
    return lhs;
}

double ConvertToDouble(std::string_view str) {
    // use std::from_chars when available to git rid of string copy
    size_t pos;
    const double result = stod(std::string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw std::invalid_argument(error.str());
    }
    return result;
}

#endif //C_PLUS_PLUS_BELTS_CUSTOMUTILS_H
