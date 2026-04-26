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

static int nearly(float a, float b, float eps) {
  return fabsf(a - b) <= eps;
}

int test_replay(void) {
  int fail = 0;
  const char* path = "replay_snapshot.fp2snap";

  Fp2WorldDesc wd;
  wd.max_bodies = 64;
  wd.max_contacts = 128;
  wd.max_joints = 32;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;

  Fp2World a;
  fp2_world_init(&a, &wd);
  fp2_world_reset(&a);

  fp2_world_add_body(&a, 1, fp_v2(0.0f, -4.0f), 0.0f, fp2_shape_box(12.0f, 0.5f), 1.0f, 0.8f, 0.0f);
  int b0 = fp2_world_add_body(&a, 0, fp_v2(-1.0f, 2.0f), 0.2f, fp2_shape_box(0.5f, 0.5f), 1.0f, 0.4f, 0.1f);
  int b1 = fp2_world_add_body(&a, 0, fp_v2(1.0f, 2.0f), -0.1f, fp2_shape_circle(0.35f), 1.0f, 0.4f, 0.1f);
  fp2_world_add_distance_joint(&a, b0, b1, fp_v2(0.0f, 0.0f), fp_v2(0.0f, 0.0f), 2.0f, 0.8f, 0.02f);

  for (int i = 0; i < 120; i++) {
    fp2_world_step(&a, 1.0f / 120.0f, 10, 4);
  }

  fail |= expect(fp2_world_save_snapshot(&a, path) == 1, "snapshot save should succeed");

  Fp2World b;
  fp2_world_init(&b, &wd);
  fp2_world_reset(&b);
  fail |= expect(fp2_world_load_snapshot(&b, path) == 1, "snapshot load should succeed");

  fail |= expect(a.body_count == b.body_count, "loaded world should have same body count");
  fail |= expect(a.joint_count == b.joint_count, "loaded world should have same joint count");
  for (int i = 0; i < a.body_count; i++) {
    fail |= expect(nearly(a.bodies[i].p.x, b.bodies[i].p.x, 1e-5f), "loaded body x should match snapshot");
    fail |= expect(nearly(a.bodies[i].p.y, b.bodies[i].p.y, 1e-5f), "loaded body y should match snapshot");
    fail |= expect(nearly(a.bodies[i].v.x, b.bodies[i].v.x, 1e-5f), "loaded body vx should match snapshot");
    fail |= expect(nearly(a.bodies[i].v.y, b.bodies[i].v.y, 1e-5f), "loaded body vy should match snapshot");
  }

  for (int i = 0; i < 120; i++) {
    fp2_world_step(&a, 1.0f / 120.0f, 10, 4);
    fp2_world_step(&b, 1.0f / 120.0f, 10, 4);
  }

  for (int i = 0; i < a.body_count; i++) {
    fail |= expect(nearly(a.bodies[i].p.x, b.bodies[i].p.x, 1e-3f), "replayed body x should stay close");
    fail |= expect(nearly(a.bodies[i].p.y, b.bodies[i].p.y, 1e-3f), "replayed body y should stay close");
  }

  remove(path);
  fp2_world_destroy(&a);
  fp2_world_destroy(&b);
  if (!fail) {
    printf("test_replay OK\n");
  }
  return fail;
}
