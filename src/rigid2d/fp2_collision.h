#pragma once

#include "core/fp_math.h"
#include "rigid2d/fp2_shape.h"

typedef struct Fp2ContactPoint {
  FpVec2 p;
  float separation;
} Fp2ContactPoint;

typedef struct Fp2Manifold {
  int hit;
  FpVec2 normal;      // from A to B
  float penetration;  // along normal
  int cp_count;
  Fp2ContactPoint cps[2];
} Fp2Manifold;

Fp2Manifold fp2_collide(Fp2Shape a, FpTransform2 xa, Fp2Shape b, FpTransform2 xb);

