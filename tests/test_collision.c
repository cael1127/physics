#include "rigid2d/fp2_collision.h"

#include <math.h>
#include <stdio.h>

static int expect(int cond, const char* msg) {
  if (!cond) {
    fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
  }
  return 0;
}

int test_collision(void) {
  int fail = 0;

  {
    Fp2Shape a = fp2_shape_circle(1.0f);
    Fp2Shape b = fp2_shape_circle(1.0f);
    FpTransform2 xa = { fp_v2(0,0), fp_m22_rot(0) };
    FpTransform2 xb = { fp_v2(1.5f,0), fp_m22_rot(0) };
    Fp2Manifold m = fp2_collide(a, xa, b, xb);
    fail |= expect(m.hit == 1, "circle-circle should hit");
    fail |= expect(m.penetration > 0.0f, "circle-circle penetration > 0");
  }

  {
    Fp2Shape a = fp2_shape_circle(1.0f);
    Fp2Shape b = fp2_shape_circle(1.0f);
    FpTransform2 xa = { fp_v2(0,0), fp_m22_rot(0) };
    FpTransform2 xb = { fp_v2(3.1f,0), fp_m22_rot(0) };
    Fp2Manifold m = fp2_collide(a, xa, b, xb);
    fail |= expect(m.hit == 0, "circle-circle should miss");
  }

  {
    Fp2Shape a = fp2_shape_box(1.0f, 1.0f);
    Fp2Shape b = fp2_shape_box(1.0f, 1.0f);
    FpTransform2 xa = { fp_v2(0,0), fp_m22_rot(0.2f) };
    FpTransform2 xb = { fp_v2(1.5f,0.2f), fp_m22_rot(-0.1f) };
    Fp2Manifold m = fp2_collide(a, xa, b, xb);
    fail |= expect(m.hit == 1, "box-box should hit");
  }

  {
    Fp2Shape circle = fp2_shape_circle(1.0f);
    Fp2Shape box = fp2_shape_box(1.0f, 1.0f);
    FpTransform2 xc = { fp_v2(1.9f,0.0f), fp_m22_rot(0) };
    FpTransform2 xb = { fp_v2(0.0f,0.0f), fp_m22_rot(0.4f) };
    Fp2Manifold m = fp2_collide(circle, xc, box, xb);
    fail |= expect(m.hit == 1, "circle-box should hit");
  }

  if (!fail) {
    printf("test_collision OK\n");
  }
  return fail;
}

