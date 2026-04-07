#include "core/fp_timestep.h"

void fp_fixed_init(FpFixedTimestep* ts, double dt, double max_frame_time) {
  ts->dt = dt;
  ts->max_frame_time = max_frame_time;
  ts->acc = 0.0;
  ts->paused = false;
}

void fp_fixed_set_paused(FpFixedTimestep* ts, bool paused) {
  ts->paused = paused;
}

bool fp_fixed_toggle_paused(FpFixedTimestep* ts) {
  ts->paused = !ts->paused;
  return ts->paused;
}

int fp_fixed_advance(FpFixedTimestep* ts, double frame_dt) {
  if (ts->paused) return 0;
  if (frame_dt > ts->max_frame_time) frame_dt = ts->max_frame_time;
  if (frame_dt < 0.0) frame_dt = 0.0;
  ts->acc += frame_dt;
  int n = (int)(ts->acc / ts->dt);
  if (n > 8) n = 8; // spiral-of-death cap
  ts->acc -= (double)n * ts->dt;
  return n;
}

