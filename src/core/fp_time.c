#include "core/fp_time.h"

#include <time.h>

#ifndef _WIN32
FpTime fp_time_now(void) {
  // Fallback for non-Windows builds.
  // ticks: clock() ticks; freq: CLOCKS_PER_SEC
  FpTime out;
  out.ticks = (int64_t)clock();
  out.freq = (int64_t)CLOCKS_PER_SEC;
  return out;
}
#endif

double fp_time_seconds(FpTime t) {
  if (t.freq <= 0) return 0.0;
  return (double)t.ticks / (double)t.freq;
}

