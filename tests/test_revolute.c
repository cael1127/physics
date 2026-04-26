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

int test_revolute(void) {
  int fail = 0;

  Fp2WorldDesc wd;
  wd.max_bodies = 16;
  wd.max_contacts = 32;
  wd.max_joints = 16;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;

  Fp2World w;
  fp2_world_init(&w, &wd);
  fp2_world_reset(&w);

  int ground = fp2_world_add_body(&w, 1, fp_v2(0.0f, 0.0f), 0.0f, fp2_shape_circle(0.1f), 1.0f, 0.5f, 0.0f);
  int arm = fp2_world_add_body(&w, 0, fp_v2(0.0f, -1.0f), 0.0f, fp2_shape_box(0.15f, 1.0f), 1.0f, 0.4f, 0.0f);

  int jid = fp2_world_add_revolute_joint(
      &w,
      ground,
      arm,
      fp_v2(0.0f, 0.0f),
      fp_v2(0.0f, 1.0f),
      0.9f,
      0.05f,
      1,
      2.0f,
      8.0f);
  fail |= expect(jid >= 0, "revolute joint creation should succeed");

  for (int i = 0; i < 240; i++) {
    fp2_world_step(&w, 1.0f / 120.0f, 8, 6);
  }

  FpVec2 pa = w.bodies[ground].p;
  FpVec2 arm_anchor_world = fp_v2_add(w.bodies[arm].p, fp_m22_mul_v2(fp_m22_rot(w.bodies[arm].angle), fp_v2(0.0f, 1.0f)));
  FpVec2 err = fp_v2_sub(arm_anchor_world, pa);
  float err_len = sqrtf(fp_v2_len2(err));
  fail |= expect(err_len < 0.2f, "revolute joint anchors should stay close");
  fail |= expect(fabsf(w.bodies[arm].angle) > 0.2f, "revolute motor should rotate dynamic body");

  fp2_world_destroy(&w);
  if (!fail) {
    printf("test_revolute OK\n");
  }
  return fail;
}
