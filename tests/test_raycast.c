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

static int expect_near(float a, float b, float eps, const char* msg) {
  return expect(fabsf(a - b) <= eps, msg);
}

int test_raycast(void) {
  int fail = 0;

  Fp2World w;
  Fp2WorldDesc wd;
  wd.max_bodies = 16;
  wd.max_contacts = 16;
  wd.max_joints = 8;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;
  fp2_world_init(&w, &wd);
  fp2_world_reset(&w);

  int static_box = fp2_world_add_body(
      &w,
      1,
      fp_v2(0.0f, 0.0f),
      0.0f,
      fp2_shape_box(1.0f, 1.0f),
      1.0f,
      0.5f,
      0.0f);

  int dynamic_circle = fp2_world_add_body(
      &w,
      0,
      fp_v2(3.0f, 0.0f),
      0.0f,
      fp2_shape_circle(0.5f),
      1.0f,
      0.5f,
      0.0f);

  {
    Fp2RayCastHit h = fp2_world_raycast_closest(&w, fp_v2(-4.0f, 0.0f), fp_v2(1.0f, 0.0f), 10.0f, 1);
    fail |= expect(h.hit == 1, "raycast should hit when static bodies are included");
    fail |= expect(h.body_index == static_box, "closest hit should be the static box first");
    fail |= expect_near(h.point.x, -1.0f, 1e-3f, "box hit x should be near left face");
    fail |= expect_near(h.normal.x, -1.0f, 1e-3f, "box hit normal should face left");
  }

  {
    Fp2RayCastHit h = fp2_world_raycast_closest(&w, fp_v2(-4.0f, 0.0f), fp_v2(1.0f, 0.0f), 10.0f, 0);
    fail |= expect(h.hit == 1, "raycast should still hit dynamic body when static excluded");
    fail |= expect(h.body_index == dynamic_circle, "dynamic circle should be closest eligible hit");
    fail |= expect_near(h.point.x, 2.5f, 1e-3f, "circle hit x should be near first tangent point");
    fail |= expect_near(h.normal.x, -1.0f, 1e-3f, "circle hit normal should face left");
  }

  {
    Fp2RayCastHit h = fp2_world_raycast_closest(&w, fp_v2(-4.0f, 3.0f), fp_v2(1.0f, 0.0f), 10.0f, 1);
    fail |= expect(h.hit == 0, "raycast should miss if segment does not cross bodies");
  }

  {
    Fp2RayCastHit h = fp2_world_raycast_closest(&w, fp_v2(0.0f, 0.0f), fp_v2(0.0f, 0.0f), 10.0f, 1);
    fail |= expect(h.hit == 0, "zero-length direction should report miss");
  }

  fp2_world_destroy(&w);
  if (!fail) {
    printf("test_raycast OK\n");
  }
  return fail;
}
