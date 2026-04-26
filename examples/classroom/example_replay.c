#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include <stdio.h>

int main(void) {
  const char* snapshot = "example_replay.fp2snap";

  Fp2WorldDesc wd;
  wd.max_bodies = 32;
  wd.max_contacts = 64;
  wd.max_joints = 16;
  wd.gravity = fp_v2(0.0f, -9.81f);
  wd.broadphase_mode = FP2_BROADPHASE_SWEEP_AND_PRUNE;

  Fp2World a, b;
  fp2_world_init(&a, &wd);
  fp2_world_init(&b, &wd);
  fp2_world_reset(&a);
  fp2_world_reset(&b);

  fp2_world_add_body(&a, 1, fp_v2(0.0f, -2.0f), 0.0f, fp2_shape_box(8.0f, 0.4f), 1.0f, 0.8f, 0.0f);
  fp2_world_add_body(&a, 0, fp_v2(0.0f, 2.5f), 0.2f, fp2_shape_box(0.5f, 0.5f), 1.0f, 0.6f, 0.1f);

  for (int i = 0; i < 120; i++) {
    fp2_world_step(&a, 1.0f / 120.0f, 10, 4);
  }

  if (!fp2_world_save_snapshot(&a, snapshot)) {
    fprintf(stderr, "Save failed\n");
    return 1;
  }
  if (!fp2_world_load_snapshot(&b, snapshot)) {
    fprintf(stderr, "Load failed\n");
    return 1;
  }

  printf("Replay load success. Loaded %d bodies.\n", b.body_count);
  fp2_world_destroy(&a);
  fp2_world_destroy(&b);
  return 0;
}
