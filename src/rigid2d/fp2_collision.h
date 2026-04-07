#pragma once

#include "core/fp_math.h"
#include "rigid2d/fp2_shape.h"

typedef struct Fp2ContactPoint {
  FpVec2 p;
} Fp2ContactPoint;

typedef struct Fp2Manifold {
  int hit;
  FpVec2 normal;      // from A to B
  float penetration;  // along normal
  Fp2ContactPoint cp; // single-point manifold (for now)
} Fp2Manifold;

Fp2Manifold fp2_collide(Fp2Shape a, FpTransform2 xa, Fp2Shape b, FpTransform2 xb);

