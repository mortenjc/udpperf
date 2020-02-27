/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// wrapper for the cheap and fast time stamp counter (TSC) which is a 64 bit
/// counter running at CPU clock. Can be used (with caution) as a timer
///
//===----------------------------------------------------------------------===//

#include <cstdint>

static __inline__ unsigned long long rdtsc(void) {
  unsigned hi, lo;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

class TSCTimer {

public:
  TSCTimer(void) { t1 = rdtsc(); }

  /** */
  void now(void) { t1 = rdtsc(); }

  /** */
  uint64_t timetsc(void) { return (rdtsc() - t1); }

private:
  uint64_t t1;
};
