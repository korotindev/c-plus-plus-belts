#include "datetime.h"

using namespace std;

int DateTime::ToMinutesPoint() const { return hours * 60 + minutes; }

DateTime operator+(DateTime dt, int minutes) {
  dt.minutes += minutes;
  dt.hours += dt.minutes / 60;
  dt.minutes %= 60;
  dt.week_day += dt.hours / 24;
  dt.hours %= 24;
  dt.week_day %= 7;
  return dt;
}

static int CalculateEverydayScheduleWaitTime(DateTime dt, const YellowPages::WorkingTime& working_time) {
  const auto& intervals = working_time.intervals();
  int start_minutes = dt.ToMinutesPoint();
  auto next_good_interval_it =
      find_if(intervals.begin(), intervals.end(), [start_minutes](const YellowPages::WorkingTimeInterval& interval) {
        return start_minutes < interval.minutes_to();
      });

  if (next_good_interval_it == intervals.end()) {
    return 1440 - start_minutes + intervals[0].minutes_from();
  }

  if (next_good_interval_it->minutes_from() <= start_minutes) {
    return 0.0;
  }

  return next_good_interval_it->minutes_from() - start_minutes;
}

static int CalculateRegularScheduleWaitTime(DateTime dt, const YellowPages::WorkingTime& working_time) {
  const auto& intervals = working_time.intervals();
  int start_minutes = dt.ToMinutesPoint();
  auto next_good_interval_it = find_if(intervals.begin(), intervals.end(),
                                       [&dt, start_minutes](const YellowPages::WorkingTimeInterval& interval) {
                                         if (dt.week_day == interval.day() - 1) {
                                           return start_minutes < interval.minutes_to();
                                         }
                                         return dt.week_day < interval.day() - 1;
                                       });

  if (next_good_interval_it == intervals.end()) {
    return 1440 - start_minutes + 1440 * (6 - dt.week_day + intervals[0].day() - 1) + intervals[0].minutes_from();
  }

  if (next_good_interval_it->day() - 1 == dt.week_day) {
    if (next_good_interval_it->minutes_from() <= start_minutes) {
      return 0.0;
    }
    return next_good_interval_it->minutes_from() - start_minutes;
  }

  return 1440 - start_minutes + 1440 * (next_good_interval_it->day() - 1 - dt.week_day) +
         next_good_interval_it->minutes_from();
}

int CalculateWaitTime(DateTime dt, const YellowPages::WorkingTime& working_time) {
  if (working_time.intervals_size() == 0) {
    return 0;
  }

  if (working_time.intervals()[0].day() == YellowPages::WorkingTimeInterval::Day::WorkingTimeInterval_Day_EVERYDAY) {
    return CalculateEverydayScheduleWaitTime(dt, working_time);
  } else {
    return CalculateRegularScheduleWaitTime(dt, working_time);
  }

  return 0;
}
