#include "rigid2d/fp2_world.h"

#include <math.h>
#include <stdio.h>

static int expect(int cond, const char* msg) {
  if (!cond) {
    fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
  }
  return 0;
}

int test_constraints(void) {
  int fail = 0;

  Fp2WorldDesc wd;
  wd.max_bodies = 8;
  wd.max_contacts = 32;
  wd.max_joints = 8;
  wd.gravity = fp_v2(0.0f, 0.0f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;
  Fp2World w;
  fp2_world_init(&w, &wd);
  fp2_world_reset(&w);

  int a = fp2_world_add_body(&w, 0, fp_v2(0.0f, 0.0f), 0.0f, fp2_shape_circle(0.2f), 1.0f, 0.5f, 0.0f);
  int b = fp2_world_add_body(&w, 0, fp_v2(3.0f, 0.0f), 0.0f, fp2_shape_circle(0.2f), 1.0f, 0.5f, 0.0f);
  int j = fp2_world_add_distance_joint(&w, a, b, fp_v2(0.0f, 0.0f), fp_v2(0.0f, 0.0f), 1.5f, 0.9f, 0.05f);
  fail |= expect(j >= 0, "distance joint creation should succeed");

  for (int i = 0; i < 240; i++) {
    fp2_world_step(&w, 1.0f / 120.0f, 6, 6);
  }

  FpVec2 d = fp_v2_sub(w.bodies[b].p, w.bodies[a].p);
  float dist = sqrtf(fp_v2_len2(d));
  fail |= expect(fabsf(dist - 1.5f) < 0.15f, "distance joint should converge near target rest length");

  fp2_world_destroy(&w);
  if (!fail) {
    printf("test_constraints OK\n");
  }
  return fail;
}
