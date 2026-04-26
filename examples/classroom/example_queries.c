#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include <stdio.h>

int main(void) {
  Fp2WorldDesc wd;
  wd.max_bodies = 32;
  wd.max_contacts = 64;
  wd.max_joints = 16;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;

  Fp2World w;
  fp2_world_init(&w, &wd);
  fp2_world_reset(&w);
  fp2_world_add_body(&w, 1, fp_v2(0.0f, 0.0f), 0.0f, fp2_shape_box(1.0f, 1.0f), 1.0f, 0.6f, 0.0f);
  fp2_world_add_body(&w, 0, fp_v2(2.0f, 0.0f), 0.0f, fp2_shape_circle(0.5f), 1.0f, 0.4f, 0.0f);

  int hit = fp2_world_query_point(&w, fp_v2(0.2f, 0.1f), 1);
  printf("Point query hit body index: %d\n", hit);

  Fp2Aabb q;
  q.min = fp_v2(-2.0f, -2.0f);
  q.max = fp_v2(2.5f, 2.0f);
  int ids[8];
  int n = fp2_world_query_aabb(&w, q, 1, ids, 8);
  printf("AABB query overlaps: %d bodies\n", n);

  fp2_world_destroy(&w);
  return 0;
}
