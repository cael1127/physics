#pragma once

#include "platform/win32/fp_win32.h"
#include "rigid2d/fp2_world.h"

typedef struct DebugDraw2D {
  float pixels_per_meter;
  FpVec2 camera_center;
} DebugDraw2D;

void dbg2d_init(DebugDraw2D* d);
void dbg2d_draw_world(DebugDraw2D* d, FpWin32Window* w, const Fp2World* world, int width, int height);

