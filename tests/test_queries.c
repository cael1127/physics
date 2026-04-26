#include "rigid2d/fp2_world.h"

#include <stdio.h>

static int expect(int cond, const char* msg) {
  if (!cond) {
    fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
  }
  return 0;
}

int test_queries(void) {
  int fail = 0;

  Fp2World w;
  Fp2WorldDesc wd;
  wd.max_bodies = 32;
  wd.max_contacts = 64;
  wd.max_joints = 16;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;
  fp2_world_init(&w, &wd);
  fp2_world_reset(&w);

  int static_box = fp2_world_add_body(
      &w, 1, fp_v2(0.0f, 0.0f), 0.0f, fp2_shape_box(1.0f, 1.0f), 1.0f, 0.6f, 0.0f);
  int dynamic_circle = fp2_world_add_body(
      &w, 0, fp_v2(3.0f, 0.0f), 0.0f, fp2_shape_circle(0.5f), 1.0f, 0.5f, 0.0f);
  int dynamic_box = fp2_world_add_body(
      &w, 0, fp_v2(0.0f, 3.0f), 0.4f, fp2_shape_box(0.8f, 0.4f), 1.0f, 0.5f, 0.0f);

  {
    int hit = fp2_world_query_point(&w, fp_v2(0.0f, 0.0f), 1);
    fail |= expect(hit == static_box, "point query should hit static box when static included");
  }
  {
    int hit = fp2_world_query_point(&w, fp_v2(0.0f, 0.0f), 0);
    fail |= expect(hit == -1, "point query should miss when only static body contains point");
  }
  {
    int hit = fp2_world_query_point(&w, fp_v2(3.0f, 0.0f), 0);
    fail |= expect(hit == dynamic_circle, "point query should hit dynamic circle");
  }
  {
    int hit = fp2_world_query_point(&w, fp_v2(0.0f, 3.0f), 0);
    fail |= expect(hit == dynamic_box, "point query should hit rotated dynamic box");
  }

  {
    int ids[4] = { -1, -1, -1, -1 };
    Fp2Aabb q;
    q.min = fp_v2(-1.2f, -1.2f);
    q.max = fp_v2(1.2f, 1.2f);
    int n = fp2_world_query_aabb(&w, q, 1, ids, 4);
    fail |= expect(n == 1, "aabb query should report one overlap near origin");
    fail |= expect(ids[0] == static_box, "aabb query should return static box index");
  }
  {
    int ids[4] = { -1, -1, -1, -1 };
    Fp2Aabb q;
    q.min = fp_v2(-1.2f, -1.2f);
    q.max = fp_v2(4.0f, 1.2f);
    int n = fp2_world_query_aabb(&w, q, 0, ids, 4);
    fail |= expect(n == 1, "aabb query excluding static should only hit dynamic circle");
    fail |= expect(ids[0] == dynamic_circle, "aabb query should include dynamic circle index");
  }
  {
    int ids[1] = { -1 };
    Fp2Aabb q;
    q.min = fp_v2(-10.0f, -10.0f);
    q.max = fp_v2(10.0f, 10.0f);
    int n = fp2_world_query_aabb(&w, q, 1, ids, 1);
    fail |= expect(n == 1, "aabb query should cap writes to output capacity");
  }

  fp2_world_destroy(&w);
  if (!fail) {
    printf("test_queries OK\n");
  }
  return fail;
}
