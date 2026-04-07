#include "rigid2d/fp2_shape.h"

#include <math.h>

Fp2Shape fp2_shape_circle(float radius) {
  Fp2Shape s;
  s.type = FP2_SHAPE_CIRCLE;
  s.u.circle.radius = radius;
  return s;
}

Fp2Shape fp2_shape_box(float hx, float hy) {
  Fp2Shape s;
  s.type = FP2_SHAPE_BOX;
  s.u.box.half_extents = fp_v2(hx, hy);
  return s;
}

static Fp2Aabb fp2_aabb_from_circle(float r, FpVec2 p) {
  Fp2Aabb a;
  a.min = fp_v2(p.x - r, p.y - r);
  a.max = fp_v2(p.x + r, p.y + r);
  return a;
}

static Fp2Aabb fp2_aabb_from_box(FpVec2 he, FpTransform2 xf) {
  // Conservative AABB of oriented box
  FpVec2 ax = fp_v2(fabsf(xf.R.c0.x), fabsf(xf.R.c0.y));
  FpVec2 ay = fp_v2(fabsf(xf.R.c1.x), fabsf(xf.R.c1.y));
  FpVec2 e = fp_v2_add(fp_v2_mul(ax, he.x), fp_v2_mul(ay, he.y));

  Fp2Aabb a;
  a.min = fp_v2(xf.p.x - e.x, xf.p.y - e.y);
  a.max = fp_v2(xf.p.x + e.x, xf.p.y + e.y);
  return a;
}

Fp2Aabb fp2_aabb_from_shape(Fp2Shape s, FpTransform2 xf) {
  switch (s.type) {
    case FP2_SHAPE_CIRCLE:
      return fp2_aabb_from_circle(s.u.circle.radius, xf.p);
    case FP2_SHAPE_BOX:
      return fp2_aabb_from_box(s.u.box.half_extents, xf);
    default: {
      Fp2Aabb a;
      a.min = xf.p;
      a.max = xf.p;
      return a;
    }
  }
}

