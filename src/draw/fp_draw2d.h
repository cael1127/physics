#pragma once

#include <stdint.h>

typedef struct FpDraw2dOps {
  void (*clear)(void* ctx, uint32_t rgb);
  void (*line)(void* ctx, int x0, int y0, int x1, int y1, uint32_t rgb);
  void (*tri_fill)(void* ctx, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t rgb);
  void (*quad_fill)(void* ctx, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t rgb);
  void (*text)(void* ctx, int x, int y, const char* utf8, uint32_t rgb);
} FpDraw2dOps;

typedef struct FpDraw2d {
  const FpDraw2dOps* ops;
  void* ctx;
} FpDraw2d;

void fp_draw2d_clear(FpDraw2d* d, uint32_t rgb);
void fp_draw2d_line(FpDraw2d* d, int x0, int y0, int x1, int y1, uint32_t rgb);
void fp_draw2d_tri_fill(FpDraw2d* d, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t rgb);
void fp_draw2d_quad_fill(FpDraw2d* d, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t rgb);
void fp_draw2d_text(FpDraw2d* d, int x, int y, const char* utf8, uint32_t rgb);

const FpDraw2dOps* fp_draw2d_win32_ops(void);
