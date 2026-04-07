#pragma once

#include <stdbool.h>

typedef struct FpFixedTimestep {
  double dt;
  double max_frame_time;
  double acc;
  bool paused;
} FpFixedTimestep;

void fp_fixed_init(FpFixedTimestep* ts, double dt, double max_frame_time);
void fp_fixed_set_paused(FpFixedTimestep* ts, bool paused);
bool fp_fixed_toggle_paused(FpFixedTimestep* ts);

// Call once per frame with measured frame time (seconds).
// Returns how many fixed-steps to execute this frame.
int fp_fixed_advance(FpFixedTimestep* ts, double frame_dt);

