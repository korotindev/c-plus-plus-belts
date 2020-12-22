#pragma once
#include "working_time.pb.h"


struct DateTime {
  int week_day;
  int hours;
  int minutes;
  int ToMinutesPoint() const;
};

DateTime operator+(DateTime dt, int minutes);

int CalculateWaitTime(DateTime dt, const YellowPages::WorkingTime& working_time);
