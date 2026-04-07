#pragma once

#include <stdint.h>

typedef struct FpTime {
  int64_t ticks;
  int64_t freq;
} FpTime;

FpTime fp_time_now(void);
double fp_time_seconds(FpTime t);

