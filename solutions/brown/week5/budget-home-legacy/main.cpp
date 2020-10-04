#include <array>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

using namespace std;

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

pair<string_view, string_view> SplitTwo(string_view s, string_view delimiter = " ") {
  const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
  return {lhs, rhs_opt.value_or("")};
}

string_view ReadToken(string_view &s, string_view delimiter = " ") {
  const auto [lhs, rhs] = SplitTwo(s, delimiter);
  s = rhs;
  return lhs;
}

int ConvertToInt(string_view str) {
  // use std::from_chars when available to git rid of string copy
  size_t pos;
  const int result = stoi(string(str), &pos);
  if (pos != str.length()) {
    std::stringstream error;
    error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
    throw invalid_argument(error.str());
  }
  return result;
}

template <typename Number>
void ValidateBounds(Number number_to_check, Number min_value, Number max_value) {
  if (number_to_check < min_value || number_to_check > max_value) {
    std::stringstream error;
    error << number_to_check << " is out of [" << min_value << ", " << max_value << "]";
    throw out_of_range(error.str());
  }
}

class Date {
 public:
  static Date FromString(string_view str) {
    const int year = ConvertToInt(ReadToken(str, "-"));
    const int month = ConvertToInt(ReadToken(str, "-"));
    ValidateBounds(month, 1, 12);
    const int day = ConvertToInt(str);
    ValidateBounds(day, 1, 31);
    return {year, month, day};
  }

  // Weird legacy, can't wait for std::chrono::year_month_day
  time_t AsTimestamp() const {
    std::tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = day_;
    t.tm_mon = month_ - 1;
    t.tm_year = year_ - 1900;
    t.tm_isdst = 0;
    return mktime(&t);
  }

 private:
  int year_;
  int month_;
  int day_;

  Date(int year, int month, int day) : year_(year), month_(month), day_(day) {}
};

int ComputeDaysDiff(const Date &date_to, const Date &date_from) {
  const time_t timestamp_to = date_to.AsTimestamp();
  const time_t timestamp_from = date_from.AsTimestamp();
  static constexpr int SECONDS_IN_DAY = 60 * 60 * 24;
  return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}

static const Date START_DATE = Date::FromString("2000-01-01");
static const Date END_DATE = Date::FromString("2100-01-01");
static const size_t DAY_COUNT = ComputeDaysDiff(END_DATE, START_DATE);
static const size_t DAY_COUNT_P2 = 1 << 16;
static const size_t VERTEX_COUNT = DAY_COUNT_P2 * 2;

size_t ComputeDayIndex(const Date &date) { return ComputeDaysDiff(date, START_DATE); }

array<double, VERTEX_COUNT> tree_earned, tree_earned_add, tree_spend, tree_spend_add, tree_factor;

void Init() {
  tree_earned.fill(0);
  tree_earned_add.fill(0);
  tree_spend.fill(0);
  tree_spend_add.fill(0);
  tree_factor.fill(1);
}

void Push(size_t vertex_id, size_t node_left, size_t node_right) {
  for (size_t vertex_child_id = vertex_id * 2; vertex_child_id <= vertex_id * 2 + 1; ++vertex_child_id) {
    if (vertex_child_id < VERTEX_COUNT) {
      tree_factor[vertex_child_id] *= tree_factor[vertex_id];
      (tree_earned_add[vertex_child_id] *= tree_factor[vertex_id]) += tree_earned_add[vertex_id];
      (tree_earned[vertex_child_id] *= tree_factor[vertex_id]) +=
          tree_earned_add[vertex_id] * (node_right - node_left) / 2;

      tree_spend_add[vertex_child_id] += tree_spend_add[vertex_id];
      (tree_spend[vertex_child_id]) += tree_spend_add[vertex_id] * (node_right - node_left) / 2;
    }
  }
  tree_factor[vertex_id] = 1;
  tree_earned_add[vertex_id] = 0;
  tree_spend_add[vertex_id] = 0;
}

double ComputeSum(size_t vertex_id, size_t node_left, size_t node_right, size_t query_left, size_t query_right) {
  if (vertex_id >= VERTEX_COUNT || query_right <= node_left || node_right <= query_left) {
    return 0;
  }
  Push(vertex_id, node_left, node_right);
  if (query_left <= node_left && node_right <= query_right) {
    return tree_earned[vertex_id] - tree_spend[vertex_id];
  }
  return ComputeSum(vertex_id * 2, node_left, (node_left + node_right) / 2, query_left, query_right) +
         ComputeSum(vertex_id * 2 + 1, (node_left + node_right) / 2, node_right, query_left, query_right);
}

void Add(size_t vertex_id, size_t node_left, size_t node_right, size_t query_right, size_t query_left,
         double earned_per_day, double spend_per_day) {
  if (vertex_id >= VERTEX_COUNT || query_left <= node_left || node_right <= query_right) {
    return;
  }
  Push(vertex_id, node_left, node_right);
  if (query_right <= node_left && node_right <= query_left) {
    tree_earned_add[vertex_id] += earned_per_day;
    tree_earned[vertex_id] += earned_per_day * (node_right - node_left);

    tree_spend_add[vertex_id] += spend_per_day;
    tree_spend[vertex_id] += spend_per_day * (node_right - node_left);
    return;
  }
  Add(vertex_id * 2, node_left, (node_left + node_right) / 2, query_right, query_left, earned_per_day, spend_per_day);
  Add(vertex_id * 2 + 1, (node_left + node_right) / 2, node_right, query_right, query_left, earned_per_day,
      spend_per_day);
  tree_earned[vertex_id] = (vertex_id * 2 < VERTEX_COUNT ? tree_earned[vertex_id * 2] : 0) +
                           (vertex_id * 2 + 1 < VERTEX_COUNT ? tree_earned[vertex_id * 2 + 1] : 0);

  tree_spend[vertex_id] = (vertex_id * 2 < VERTEX_COUNT ? tree_spend[vertex_id * 2] : 0) +
                          (vertex_id * 2 + 1 < VERTEX_COUNT ? tree_spend[vertex_id * 2 + 1] : 0);
}

void Multiply(size_t vertex_id, size_t node_left, size_t node_right, size_t query_left, size_t query_right,
              double factor) {
  if (vertex_id >= VERTEX_COUNT || query_right <= node_left || node_right <= query_left) {
    return;
  }
  Push(vertex_id, node_left, node_right);
  if (query_left <= node_left && node_right <= query_right) {
    tree_factor[vertex_id] *= factor;
    tree_earned_add[vertex_id] *= factor;
    tree_earned[vertex_id] *= factor;
    return;
  }
  Multiply(vertex_id * 2, node_left, (node_left + node_right) / 2, query_left, query_right, factor);
  Multiply(vertex_id * 2 + 1, (node_left + node_right) / 2, node_right, query_left, query_right, factor);
  tree_earned[vertex_id] = (vertex_id * 2 < VERTEX_COUNT ? tree_earned[vertex_id * 2] : 0) +
                           (vertex_id * 2 + 1 < VERTEX_COUNT ? tree_earned[vertex_id * 2 + 1] : 0);
}

int main() {
  cout.precision(25);
  assert(DAY_COUNT <= DAY_COUNT_P2 && DAY_COUNT_P2 < DAY_COUNT * 2);

  Init();

  int query_count;
  cin >> query_count;

  for (int i = 0; i < query_count; ++i) {
    string query_type;
    cin >> query_type;

    string date_from_str, date_to_str;
    cin >> date_from_str >> date_to_str;

    auto idx_from = ComputeDayIndex(Date::FromString(date_from_str));
    auto idx_to = ComputeDayIndex(Date::FromString(date_to_str)) + 1;

    if (query_type == "ComputeIncome") {
      cout << ComputeSum(1, 0, DAY_COUNT_P2, idx_from, idx_to) << endl;
    } else if (query_type == "PayTax") {
      size_t percentage;
      cin >> percentage;
      double factor = 1.0 - percentage / 100.0;
      Multiply(1, 0, DAY_COUNT_P2, idx_from, idx_to, factor);
    } else if (query_type == "Earn") {
      double value;
      cin >> value;
      Add(1, 0, DAY_COUNT_P2, idx_from, idx_to, value / (idx_to - idx_from), 0);
    } else if (query_type == "Spend") {
      double value;
      cin >> value;
      Add(1, 0, DAY_COUNT_P2, idx_from, idx_to, 0, value / (idx_to - idx_from));
    }
  }

  return 0;
}