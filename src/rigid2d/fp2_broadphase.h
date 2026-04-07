#pragma once

#include "rigid2d/fp2_shape.h"

typedef struct Fp2Pair {
  int a;
  int b;
} Fp2Pair;

// Naive broadphase: produces potentially-colliding pairs by AABB overlap.
int fp2_broadphase_pairs(const Fp2Aabb* aabbs, int count, Fp2Pair* out_pairs, int max_pairs);

