/** Copyright (C) 2016 European Spallation Source */

#include <common/Timer.h>

/** */
Timer::Timer(void) { t1 = HRClock::now(); }

/** */
void Timer::now(void) { t1 = HRClock::now(); }

/** */
uint64_t Timer::timeus(void) {
  Timer::TP t2 = Timer::HRClock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}
