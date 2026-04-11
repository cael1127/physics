#pragma once

#include "core/fp_math.h"

typedef struct FpCamera2 {
  FpVec2 center;
  float pixels_per_meter;
  float zoom; // 1 = default; >1 zoom in
} FpCamera2;

void fp_camera2_init_default(FpCamera2* c);

// World y-up to screen pixels (top-left origin).
void fp_camera2_world_to_screen(const FpCamera2* c, int viewport_w, int viewport_h, FpVec2 world, int* out_sx, int* out_sy);
