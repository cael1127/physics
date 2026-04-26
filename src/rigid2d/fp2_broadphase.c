#include "rigid2d/fp2_broadphase.h"

#include <stdlib.h>

static int fp2_aabb_overlap(Fp2Aabb a, Fp2Aabb b) {
  if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
  if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
  return 1;
}

static int fp2_pairs_naive(const Fp2Aabb* aabbs, int count, Fp2Pair* out_pairs, int max_pairs) {
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

typedef struct Fp2SweepItem {
  float min_x;
  int index;
} Fp2SweepItem;

static int fp2_sweep_cmp(const void* lhs, const void* rhs) {
  const Fp2SweepItem* a = (const Fp2SweepItem*)lhs;
  const Fp2SweepItem* b = (const Fp2SweepItem*)rhs;
  if (a->min_x < b->min_x) return -1;
  if (a->min_x > b->min_x) return 1;
  return a->index - b->index;
}

static int fp2_pairs_sweep_and_prune(const Fp2Aabb* aabbs, int count, Fp2Pair* out_pairs, int max_pairs) {
  if (count <= 1) return 0;

  Fp2SweepItem* items = (Fp2SweepItem*)malloc((size_t)count * sizeof(Fp2SweepItem));
  if (!items) return fp2_pairs_naive(aabbs, count, out_pairs, max_pairs);

  for (int i = 0; i < count; i++) {
    items[i].min_x = aabbs[i].min.x;
    items[i].index = i;
  }
  qsort(items, (size_t)count, sizeof(Fp2SweepItem), fp2_sweep_cmp);

  int n = 0;
  for (int ii = 0; ii < count; ii++) {
    int i = items[ii].index;
    float max_x = aabbs[i].max.x;
    for (int jj = ii + 1; jj < count; jj++) {
      int j = items[jj].index;
      if (aabbs[j].min.x > max_x) break;
      if (!fp2_aabb_overlap(aabbs[i], aabbs[j])) continue;
      if (n < max_pairs) {
        out_pairs[n].a = i;
        out_pairs[n].b = j;
        n++;
      }
    }
  }

  free(items);
  return n;
}

int fp2_broadphase_pairs(const Fp2Aabb* aabbs, int count, Fp2Pair* out_pairs, int max_pairs, Fp2BroadphaseMode mode) {
  if (mode == FP2_BROADPHASE_SWEEP_AND_PRUNE) {
    return fp2_pairs_sweep_and_prune(aabbs, count, out_pairs, max_pairs);
  }
  return fp2_pairs_naive(aabbs, count, out_pairs, max_pairs);
}

