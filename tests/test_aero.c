#include "aero2d/fp_aero2d.h"
#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include <math.h>
#include <stdio.h>

int test_aero(void) {
  Fp2World w;
  Fp2WorldDesc wd;
  wd.max_bodies = 32;
  wd.max_contacts = 64;
  wd.max_joints = 16;
  wd.gravity = fp_v2(0.0f, 0.0f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;
  fp2_world_init(&w, &wd);

  fp2_world_add_body(&w, 0, fp_v2(0.0f, 0.0f), 0.0f, fp2_shape_circle(0.5f), 1.0f, 0.2f, 0.0f);

  FpAero2dDesc ad;
  ad.wind = fp_v2(6.0f, 0.0f);
  ad.air_density = 0.8f;
  ad.drag_coefficient = 0.5f;
  ad.lift_coefficient = 0.0f;

  float vx0 = w.bodies[0].v.x;
  fp_aero2d_apply_forces(&w, w.shapes, &ad, 1.0f / 60.0f);
  float vx1 = w.bodies[0].v.x;

  if (!(vx1 > vx0 + 0.01f)) {
    fprintf(stderr, "test_aero: expected drag to accelerate body toward wind (vx %.6f -> %.6f)\n", (double)vx0, (double)vx1);
    fp2_world_destroy(&w);
    return 1;
  }

  fp2_world_add_body(&w, 0, fp_v2(0.0f, 2.0f), 0.0f, fp2_shape_circle(0.5f), 1.0f, 0.2f, 0.0f);
  w.bodies[1].v = fp_v2(0.0f, 0.0f);
  fp_aero2d_apply_forces(&w, w.shapes, &ad, 1.0f / 60.0f);
  if (fabsf(w.bodies[1].w) > 1e-3f) {
    fprintf(stderr, "test_aero: symmetric circle should get negligible torque from pure drag\n");
    fp2_world_destroy(&w);
    return 1;
  }

  fp2_world_destroy(&w);
  printf("test_aero OK\n");
  return 0;
}
