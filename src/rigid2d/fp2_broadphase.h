#pragma once

#include "rigid2d/fp2_shape.h"

typedef struct Fp2Pair {
  int a;
  int b;
} Fp2Pair;

typedef enum Fp2BroadphaseMode {
  FP2_BROADPHASE_NAIVE = 0,
  FP2_BROADPHASE_SWEEP_AND_PRUNE = 1
} Fp2BroadphaseMode;

// Produces potentially-colliding pairs by AABB overlap.
int fp2_broadphase_pairs(const Fp2Aabb* aabbs, int count, Fp2Pair* out_pairs, int max_pairs, Fp2BroadphaseMode mode);

