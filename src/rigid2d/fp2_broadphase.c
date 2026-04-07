#include "rigid2d/fp2_broadphase.h"

static int fp2_aabb_overlap(Fp2Aabb a, Fp2Aabb b) {
  if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
  if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
  return 1;
}

int fp2_broadphase_pairs(const Fp2Aabb* aabbs, int count, Fp2Pair* out_pairs, int max_pairs) {
  int n = 0;
  for (int i = 0; i < count; i++) {
    for (int j = i + 1; j < count; j++) {
      if (!fp2_aabb_overlap(aabbs[i], aabbs[j])) continue;
      if (n < max_pairs) {
        out_pairs[n].a = i;
        out_pairs[n].b = j;
        n++;
      }
    }
  }
  return n;
}

