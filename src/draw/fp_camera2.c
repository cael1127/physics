#include "draw/fp_camera2.h"

#include <math.h>

void fp_camera2_init_default(FpCamera2* c) {
  c->center = fp_v2(0.0f, 0.0f);
  c->pixels_per_meter = 60.0f;
  c->zoom = 1.0f;
}

void fp_camera2_world_to_screen(const FpCamera2* c, int viewport_w, int viewport_h, FpVec2 world, int* out_sx, int* out_sy) {
  float ppm = c->pixels_per_meter * c->zoom;
  float sx = (world.x - c->center.x) * ppm + 0.5f * (float)viewport_w;
  float sy = (-(world.y - c->center.y)) * ppm + 0.5f * (float)viewport_h;
  if (out_sx) *out_sx = (int)lrintf(sx);
  if (out_sy) *out_sy = (int)lrintf(sy);
}
