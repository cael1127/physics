#pragma once

#include "draw/fp_camera2.h"
#include "draw/fp_draw2d.h"

typedef struct FpScene FpScene;
typedef struct FpWin32Window FpWin32Window;

typedef struct DebugDraw2D {
  FpCamera2 cam;
  FpDraw2d draw;
} DebugDraw2D;

void dbg2d_init(DebugDraw2D* d, FpWin32Window* win);

void dbg2d_draw_scene(
    DebugDraw2D* d,
    FpScene* scene,
    int viewport_w,
    int viewport_h,
    float fps,
    const char* scene_label);
