#include "profile.h"
#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <tuple>
#include <vector>

using namespace std;

template <typename It>
class Range {
public:
  Range(It begin, It end) : begin_(begin), end_(end) {}

  It begin() const { return begin_; }

  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

struct DateHasher;

class DateIterator;

class Date {
  friend DateHasher;
  friend DateIterator;

public:
  Date() = default;

  Date(size_t year_, size_t month_, size_t day_) : year(year_), month(month_), day(day_) {}

  friend bool operator<(const Date &lhs, const Date &rhs) {
    return make_tuple(lhs.year, lhs.month, lhs.day) < make_tuple(rhs.year, rhs.month, rhs.day);
  }

  friend bool operator<=(const Date &lhs, const Date &rhs) {
    return make_tuple(lhs.year, lhs.month, lhs.day) <= make_tuple(rhs.year, rhs.month, rhs.day);
  }

  friend bool operator==(const Date &lhs, const Date &rhs) {
    return make_tuple(lhs.year, lhs.month, lhs.day) == make_tuple(rhs.year, rhs.month, rhs.day);
  }

  friend void operator>>(istream &input, Date &rhs) {
    input >> rhs.year;
    input.ignore(1);
    input >> rhs.month;
    input.ignore(1);
    input >> rhs.day;
  }

  friend ostream &operator<<(ostream &output, const Date &rhs) {
    output << rhs.year << '-' << rhs.month << '-' << rhs.day;
    return output;
  }

private:
  size_t year = 2000;
  size_t month = 0;
  size_t day = 0;
};

class DateIterator {
public:
  DateIterator(Date date_) : begin_date(date_) {}

  DateIterator operator++(int) {
    const size_t month_pos = this->begin_date.month - 1;
    const size_t month_length = [this, month_pos]() {
      if (month_pos == 1 && this->IsLeapYear()) {
        return this->month_lengths[month_pos] + 1;
      } else {
        return this->month_lengths[month_pos];
      }
    }();

    this->begin_date.day++;

    if (this->begin_date.day > month_length) {
      this->begin_date.day = 1;
      this->begin_date.month++;
    }

    if (this->begin_date.month > 12) {
      this->begin_date.month = 1;
      this->begin_date.year++;
    }

    return *this;
  }

  Date &operator*() { return begin_date; }

private:
  bool IsLeapYear() {
    return (begin_date.year % 4 == 0) && !(begin_date.year % 100 == 0 && begin_date.year % 400 != 0);
  }

  const vector<size_t> month_lengths = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  Date begin_date;
};

struct DateHasher {
  size_t operator()(const Date &dt) const {
    size_t r1 = int_hasher(dt.year);
    size_t r2 = int_hasher(dt.month);
    size_t r3 = int_hasher(dt.day);

    size_t x = 37;

    return r1 * x * x + r2 * x + r3;
  }

  hash<size_t> int_hasher;
};

class DB {
  static const size_t min_year = 2000;
  static const size_t max_year = 2099;

public:
  DB() {
    Date minDate(min_year, 1, 1);
    Date maxDate(max_year, 12, 31);
    DateIterator dateIterator(minDate);

    size_t index = 0;
    for (; *dateIterator <= maxDate; dateIterator++) {
      date_to_size_t[*dateIterator] = index++;
    }
    earned_per_day.assign(index, 0);
    spend_per_day.assign(index, 0);
  }

  void Earn(Date fromDate, Date toDate, double value) {
    auto datesRange = TransformDatesToRange(fromDate, toDate, earned_per_day);
    const size_t days_count = distance(datesRange.begin(), datesRange.end());
    double diff_per_day = value / static_cast<double>(days_count);
    transform(datesRange.begin(), datesRange.end(), datesRange.begin(),
              [diff_per_day](double d) { return d += diff_per_day; });
  }

  double ComputeIncome(Date fromDate, Date toDate) {
    auto earnDatesRange = TransformDatesToRange(fromDate, toDate, earned_per_day);
    auto spendDatesRange = TransformDatesToRange(fromDate, toDate, spend_per_day);
    return accumulate(earnDatesRange.begin(), earnDatesRange.end(), double(0)) -
           accumulate(spendDatesRange.begin(), spendDatesRange.end(), double(0));
  }

  void PayTax(Date fromDate, Date toDate, int percentage) {
    auto datesRange = TransformDatesToRange(fromDate, toDate, earned_per_day);
    double coefficient = 1 - percentage / 100.0;
    transform(datesRange.begin(), datesRange.end(), datesRange.begin(),
              [coefficient](double d) { return d * coefficient; });
  }

  void Spend(Date fromDate, Date toDate, double value) {
    auto datesRange = TransformDatesToRange(fromDate, toDate, spend_per_day);
    const size_t days_count = distance(datesRange.begin(), datesRange.end());
    double diff_per_day = value / static_cast<double>(days_count);
    transform(datesRange.begin(), datesRange.end(), datesRange.begin(),
              [diff_per_day](double d) { return d += diff_per_day; });
  }

private:
  size_t DateToUnixEpoch(Date d) { return date_to_size_t[d]; }

  template <class Container>
  Range<typename Container::iterator> TransformDatesToRange(Date fromDate, Date toDate, Container &v) {
    const size_t from = DateToUnixEpoch(fromDate);
    const size_t to = DateToUnixEpoch(toDate);
    auto beginIter = v.begin();
    advance(beginIter, from);
    auto endIter = v.begin();
    advance(endIter, to + 1);
    return Range(beginIter, endIter);
  }

  vector<double> earned_per_day;
  vector<double> spend_per_day;
  unordered_map<Date, size_t, DateHasher> date_to_size_t;
};

void TestDateRead() {
  stringstream ss("2000-03-06");

  Date d, expected(2000, 03, 06);
  ss >> d;

  ASSERT_EQUAL(d, expected);
}

void TestDateIterator() {
  Date d(2000, 12, 31), expected(2001, 1, 1);

  DateIterator dateIterator(d);
  dateIterator++;

  ASSERT_EQUAL(*dateIterator, expected);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestDateRead);
  RUN_TEST(tr, TestDateIterator);

  DB db;
  std::cout.precision(25);

  int q;

  cin >> q;

  for (int i = 0; i < q; i++) {
    string command;
    cin >> command;
    Date from, to;
    cin >> from;
    cin >> to;

    if (command == "ComputeIncome") {
      cout << db.ComputeIncome(from, to) << '\n';
    } else if (command == "Earn") {
      int earned = 0;
      cin >> earned;
      db.Earn(from, to, earned);
    } else if (command == "PayTax") {
      int percentage = 0;
      cin >> percentage;
      db.PayTax(from, to, percentage);
    } else if (command == "Spend") {
      int spent = 0;
      cin >> spent;
      db.Spend(from, to, spent);
    }
  }
  return 0;
}
