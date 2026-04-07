#pragma once

#include "core/fp_math.h"

typedef enum Fp2ShapeType {
  FP2_SHAPE_CIRCLE = 0,
  FP2_SHAPE_BOX = 1
} Fp2ShapeType;

typedef struct Fp2Circle {
  float radius;
} Fp2Circle;

typedef struct Fp2Box {
  FpVec2 half_extents;
} Fp2Box;

typedef struct Fp2Shape {
  Fp2ShapeType type;
  union {
    Fp2Circle circle;
    Fp2Box box;
  } u;
} Fp2Shape;

Fp2Shape fp2_shape_circle(float radius);
Fp2Shape fp2_shape_box(float hx, float hy);

typedef struct Fp2Aabb {
  FpVec2 min;
  FpVec2 max;
} Fp2Aabb;

Fp2Aabb fp2_aabb_from_shape(Fp2Shape s, FpTransform2 xf);

