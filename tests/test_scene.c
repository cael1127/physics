#include "runtime/fp_scene_data.h"
#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include <stdio.h>

int test_scene(void) {
  Fp2World w;
  Fp2WorldDesc wd;
  wd.max_bodies = 64;
  wd.max_contacts = 128;
  wd.gravity = fp_v2(0.0f, -9.81f);
  fp2_world_init(&w, &wd);

  fp2_world_reset(&w);
  fp2_world_add_body(
      &w,
      1,
      fp_v2(0.0f, -4.0f),
      0.0f,
      fp2_shape_box(14.0f, 0.5f),
      1.0f,
      0.8f,
      0.0f);

  const FpSceneSpawnTable* t = fp_scene_data_table_stack_core();
  fp_scene_data_apply_table(&w, t);

  if (w.body_count < 4) {
    fprintf(stderr, "test_scene: expected multiple bodies from spawn table\n");
    fp2_world_destroy(&w);
    return 1;
  }

  fp2_world_destroy(&w);
  printf("test_scene OK\n");
  return 0;
}
