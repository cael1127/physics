#include "app/demo_scenes.h"

static void demo_add_ground(Fp2World* w) {
  fp2_world_add_body(
      w,
      1,
      fp_v2(0.0f, -4.0f),
      0.0f,
      fp2_shape_box(14.0f, 0.5f),
      1.0f,
      0.8f,
      0.0f);
}

void demo_build_scene(Fp2World* w, DemoSceneId id) {
  fp2_world_reset(w);

  demo_add_ground(w);

  if (id == DEMO_SCENE_STACK) {
    // stack of boxes
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        float px = -4.0f + (float)x * 1.1f;
        float py = -3.0f + (float)y * 1.1f;
        fp2_world_add_body(
            w, 0, fp_v2(px, py), 0.02f * (float)(x - y),
            fp2_shape_box(0.5f, 0.5f),
            1.0f,
            0.6f,
            0.05f);
      }
    }
  } else if (id == DEMO_SCENE_BOUNCE) {
    // circles with varying restitution
    for (int i = 0; i < 10; i++) {
      float r = 0.25f + 0.02f * (float)i;
      float e = (float)i / 9.0f;
      fp2_world_add_body(
          w, 0, fp_v2(-4.5f + (float)i, 2.0f + 0.3f * (float)i), 0.0f,
          fp2_shape_circle(r),
          1.0f,
          0.2f,
          e);
    }
    // side walls
    fp2_world_add_body(w, 1, fp_v2(-6.5f, 0.0f), 0.0f, fp2_shape_box(0.5f, 6.0f), 1.0f, 0.2f, 0.0f);
    fp2_world_add_body(w, 1, fp_v2(6.5f, 0.0f), 0.0f, fp2_shape_box(0.5f, 6.0f), 1.0f, 0.2f, 0.0f);
  } else if (id == DEMO_SCENE_FRICTION) {
    // ramp + sliding boxes
    fp2_world_add_body(w, 1, fp_v2(0.0f, -2.0f), 0.35f, fp2_shape_box(8.0f, 0.3f), 1.0f, 0.9f, 0.0f);

    float mus[5] = {0.05f, 0.15f, 0.35f, 0.6f, 0.9f};
    for (int i = 0; i < 5; i++) {
      fp2_world_add_body(w, 0, fp_v2(-4.0f + (float)i * 2.0f, 2.0f), 0.0f, fp2_shape_box(0.5f, 0.5f), 1.0f, mus[i], 0.0f);
    }
  } else {
    // default
    for (int i = 0; i < 8; i++) {
      fp2_world_add_body(w, 0, fp_v2(-2.0f + 0.6f * (float)i, 1.0f + 0.4f * (float)i), 0.0f, fp2_shape_circle(0.3f), 1.0f, 0.3f, 0.3f);
    }
  }
}

