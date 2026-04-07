#pragma once

#include <stdbool.h>

typedef struct FpVec2 {
  float x;
  float y;
} FpVec2;

typedef struct FpMat22 {
  // column-major (c0, c1)
  FpVec2 c0;
  FpVec2 c1;
} FpMat22;

typedef struct FpTransform2 {
  FpVec2 p;
  FpMat22 R;
} FpTransform2;

static inline FpVec2 fp_v2(float x, float y) { FpVec2 v = {x, y}; return v; }

FpVec2 fp_v2_add(FpVec2 a, FpVec2 b);
FpVec2 fp_v2_sub(FpVec2 a, FpVec2 b);
FpVec2 fp_v2_mul(FpVec2 a, float s);
FpVec2 fp_v2_mad(FpVec2 a, float s, FpVec2 b); // a + s*b
float fp_v2_dot(FpVec2 a, FpVec2 b);
float fp_v2_len2(FpVec2 a);
float fp_v2_len(FpVec2 a);
FpVec2 fp_v2_norm(FpVec2 a);
FpVec2 fp_v2_perp(FpVec2 a);

FpMat22 fp_m22_rot(float radians);
FpVec2 fp_m22_mul_v2(FpMat22 A, FpVec2 v);
FpVec2 fp_m22_tmul_v2(FpMat22 A, FpVec2 v); // transpose(A)*v

FpVec2 fp_xf2_mul(FpTransform2 xf, FpVec2 v);
FpVec2 fp_xf2_tmul(FpTransform2 xf, FpVec2 v);

bool fp_finite2(FpVec2 v);

