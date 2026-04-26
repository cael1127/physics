#include "rigid2d/fp2_world.h"

#include <math.h>
#include <stdio.h>

static int nearly(float a, float b, float eps) { return fabsf(a - b) <= eps; }

static void build_scene(Fp2World* w) {
  fp2_world_reset(w);
  fp2_world_add_body(w, 1, fp_v2(0.0f, -4.0f), 0.0f, fp2_shape_box(14.0f, 0.5f), 1.0f, 0.8f, 0.0f);
  for (int y = 0; y < 6; y++) {
    for (int x = 0; x < 4; x++) {
      fp2_world_add_body(w, 0, fp_v2(-2.0f + (float)x * 1.1f, -3.0f + (float)y * 1.1f), 0.0f,
                         fp2_shape_box(0.5f, 0.5f), 1.0f, 0.6f, 0.05f);
    }
  }
}

static void run(Fp2World* w) {
  build_scene(w);
  for (int i = 0; i < 600; i++) {
    fp2_world_step(w, 1.0f / 60.0f, 10, 4);
  }
}

int test_determinism(void) {
  Fp2WorldDesc wd;
  wd.max_bodies = 512;
  wd.max_contacts = 2048;
  wd.max_joints = 256;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;

  Fp2World a, b;
  fp2_world_init(&a, &wd);
  fp2_world_init(&b, &wd);

  run(&a);
  run(&b);

  int fail = 0;
  if (a.body_count != b.body_count) fail = 1;
  for (int i = 0; i < a.body_count && !fail; i++) {
    if (!nearly(a.bodies[i].p.x, b.bodies[i].p.x, 1e-4f)) fail = 1;
    if (!nearly(a.bodies[i].p.y, b.bodies[i].p.y, 1e-4f)) fail = 1;
  }

  fp2_world_destroy(&a);
  fp2_world_destroy(&b);

  if (fail) {
    fprintf(stderr, "FAIL: determinism mismatch\n");
    return 1;
  }
  printf("test_determinism OK\n");
  return 0;
}

