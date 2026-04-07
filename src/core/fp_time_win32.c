#include "core/fp_time.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

FpTime fp_time_now(void) {
  static int64_t freq = 0;
  if (freq == 0) {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    freq = (int64_t)f.QuadPart;
  }

  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);

  FpTime out;
  out.ticks = (int64_t)t.QuadPart;
  out.freq = freq;
  return out;
}

