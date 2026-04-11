#include "draw/fp_draw2d.h"

void fp_draw2d_clear(FpDraw2d* d, uint32_t rgb) {
  if (d && d->ops && d->ops->clear) d->ops->clear(d->ctx, rgb);
}

void fp_draw2d_line(FpDraw2d* d, int x0, int y0, int x1, int y1, uint32_t rgb) {
  if (d && d->ops && d->ops->line) d->ops->line(d->ctx, x0, y0, x1, y1, rgb);
}

void fp_draw2d_tri_fill(FpDraw2d* d, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t rgb) {
  if (d && d->ops && d->ops->tri_fill) d->ops->tri_fill(d->ctx, x0, y0, x1, y1, x2, y2, rgb);
}

void fp_draw2d_quad_fill(FpDraw2d* d, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t rgb) {
  if (d && d->ops && d->ops->quad_fill) d->ops->quad_fill(d->ctx, x0, y0, x1, y1, x2, y2, x3, y3, rgb);
}

void fp_draw2d_text(FpDraw2d* d, int x, int y, const char* utf8, uint32_t rgb) {
  if (d && d->ops && d->ops->text) d->ops->text(d->ctx, x, y, utf8, rgb);
}
