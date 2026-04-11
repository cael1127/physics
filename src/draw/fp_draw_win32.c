#include "draw/fp_draw2d.h"
#include "platform/win32/fp_win32.h"

static void fp_dw32_clear(void* ctx, uint32_t rgb) { fp_win32_clear((FpWin32Window*)ctx, rgb); }
static void fp_dw32_line(void* ctx, int x0, int y0, int x1, int y1, uint32_t rgb) {
  fp_win32_line((FpWin32Window*)ctx, x0, y0, x1, y1, rgb);
}
static void fp_dw32_tri_fill(void* ctx, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t rgb) {
  fp_win32_fill_triangle((FpWin32Window*)ctx, x0, y0, x1, y1, x2, y2, rgb);
}
static void fp_dw32_quad_fill(void* ctx, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t rgb) {
  fp_win32_fill_quad((FpWin32Window*)ctx, x0, y0, x1, y1, x2, y2, x3, y3, rgb);
}
static void fp_dw32_text(void* ctx, int x, int y, const char* utf8, uint32_t rgb) {
  fp_win32_text_utf8((FpWin32Window*)ctx, x, y, utf8, rgb);
}

static const FpDraw2dOps g_fp_draw2d_win32_ops = {
    fp_dw32_clear,
    fp_dw32_line,
    fp_dw32_tri_fill,
    fp_dw32_quad_fill,
    fp_dw32_text,
};

const FpDraw2dOps* fp_draw2d_win32_ops(void) { return &g_fp_draw2d_win32_ops; }
