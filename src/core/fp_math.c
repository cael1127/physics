#include "core/fp_math.h"

#include <math.h>
#include <float.h>

FpVec2 fp_v2_add(FpVec2 a, FpVec2 b) { return fp_v2(a.x + b.x, a.y + b.y); }
FpVec2 fp_v2_sub(FpVec2 a, FpVec2 b) { return fp_v2(a.x - b.x, a.y - b.y); }
FpVec2 fp_v2_mul(FpVec2 a, float s) { return fp_v2(a.x * s, a.y * s); }
FpVec2 fp_v2_mad(FpVec2 a, float s, FpVec2 b) { return fp_v2(a.x + s * b.x, a.y + s * b.y); }
float fp_v2_dot(FpVec2 a, FpVec2 b) { return a.x * b.x + a.y * b.y; }
float fp_v2_len2(FpVec2 a) { return fp_v2_dot(a, a); }
float fp_v2_len(FpVec2 a) { return sqrtf(fp_v2_len2(a)); }

FpVec2 fp_v2_norm(FpVec2 a) {
  float l2 = fp_v2_len2(a);
  if (l2 <= 0.0f) return fp_v2(0.0f, 0.0f);
  float inv = 1.0f / sqrtf(l2);
  return fp_v2_mul(a, inv);
}

FpVec2 fp_v2_perp(FpVec2 a) { return fp_v2(-a.y, a.x); }

FpMat22 fp_m22_rot(float radians) {
  float c = cosf(radians);
  float s = sinf(radians);
  FpMat22 A;
  A.c0 = fp_v2(c, s);
  A.c1 = fp_v2(-s, c);
  return A;
}

FpVec2 fp_m22_mul_v2(FpMat22 A, FpVec2 v) {
  return fp_v2(A.c0.x * v.x + A.c1.x * v.y,
               A.c0.y * v.x + A.c1.y * v.y);
}

FpVec2 fp_m22_tmul_v2(FpMat22 A, FpVec2 v) {
  // [c0 c1]^T * v = [dot(c0,v), dot(c1,v)]
  return fp_v2(fp_v2_dot(A.c0, v), fp_v2_dot(A.c1, v));
}

FpVec2 fp_xf2_mul(FpTransform2 xf, FpVec2 v) {
  return fp_v2_add(xf.p, fp_m22_mul_v2(xf.R, v));
}

FpVec2 fp_xf2_tmul(FpTransform2 xf, FpVec2 v) {
  return fp_m22_tmul_v2(xf.R, fp_v2_sub(v, xf.p));
}

bool fp_finite2(FpVec2 v) {
  // isnan/isfinite are C99 but MSVC's isfinite is _finite; use portable checks.
  if (!(v.x == v.x) || !(v.y == v.y)) return false;
  if (v.x > FLT_MAX || v.x < -FLT_MAX) return false;
  if (v.y > FLT_MAX || v.y < -FLT_MAX) return false;
  return true;
}

