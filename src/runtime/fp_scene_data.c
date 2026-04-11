#include "runtime/fp_scene_data.h"

#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

static void fp_scene_spawn_ground(Fp2World* w) {
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

static void fp_scene_spawn_stack(Fp2World* w) {
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
}

static void fp_scene_spawn_bounce(Fp2World* w) {
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
  fp2_world_add_body(w, 1, fp_v2(-6.5f, 0.0f), 0.0f, fp2_shape_box(0.5f, 6.0f), 1.0f, 0.2f, 0.0f);
  fp2_world_add_body(w, 1, fp_v2(6.5f, 0.0f), 0.0f, fp2_shape_box(0.5f, 6.0f), 1.0f, 0.2f, 0.0f);
}

static void fp_scene_spawn_friction(Fp2World* w) {
  fp2_world_add_body(w, 1, fp_v2(0.0f, -2.0f), 0.35f, fp2_shape_box(8.0f, 0.3f), 1.0f, 0.9f, 0.0f);
  float mus[5] = {0.05f, 0.15f, 0.35f, 0.6f, 0.9f};
  for (int i = 0; i < 5; i++) {
    fp2_world_add_body(w, 0, fp_v2(-4.0f + (float)i * 2.0f, 2.0f), 0.0f, fp2_shape_box(0.5f, 0.5f), 1.0f, mus[i], 0.0f);
  }
}

static const FpSceneSpawnRow g_table_stack_core[] = {
    {0, -4.0f, -3.0f, 0.0f, 1, 0.5f, 0.5f, 1.0f, 0.6f, 0.05f},
    {0, -2.9f, -3.0f, 0.02f, 1, 0.5f, 0.5f, 1.0f, 0.6f, 0.05f},
    {0, -1.8f, -3.0f, 0.04f, 1, 0.5f, 0.5f, 1.0f, 0.6f, 0.05f},
};

static const FpSceneSpawnRow g_table_bounce_core[] = {
    {0, -4.5f, 2.0f, 0.0f, 0, 0.25f, 0.0f, 1.0f, 0.2f, 0.0f},
    {0, -3.5f, 2.3f, 0.0f, 0, 0.27f, 0.0f, 1.0f, 0.2f, 0.11f},
    {1, -6.5f, 0.0f, 0.0f, 1, 0.5f, 6.0f, 1.0f, 0.2f, 0.0f},
};

static const FpSceneSpawnRow g_table_friction_core[] = {
    {1, 0.0f, -2.0f, 0.35f, 1, 8.0f, 0.3f, 1.0f, 0.9f, 0.0f},
    {0, -4.0f, 2.0f, 0.0f, 1, 0.5f, 0.5f, 1.0f, 0.05f, 0.0f},
    {0, -2.0f, 2.0f, 0.0f, 1, 0.5f, 0.5f, 1.0f, 0.35f, 0.0f},
};

void fp_scene_data_apply_table(Fp2World* w, const FpSceneSpawnTable* t) {
  if (!w || !t) return;
  for (int i = 0; i < t->row_count; i++) {
    const FpSceneSpawnRow* r = &t->rows[i];
    Fp2Shape sh;
    if (r->shape_kind == 0) {
      sh = fp2_shape_circle(r->dim0);
    } else {
      sh = fp2_shape_box(r->dim0, r->dim1);
    }
    fp2_world_add_body(
        w,
        r->is_static,
        fp_v2(r->px, r->py),
        r->angle,
        sh,
        r->density,
        r->friction,
        r->restitution);
  }
}

void fp_scene_data_build_world(Fp2World* w, int demo_id) {
  if (!w) return;
  fp2_world_reset(w);
  fp_scene_spawn_ground(w);
  switch (demo_id) {
    case FP_SCENE_DEMO_STACK: fp_scene_spawn_stack(w); break;
    case FP_SCENE_DEMO_BOUNCE: fp_scene_spawn_bounce(w); break;
    case FP_SCENE_DEMO_FRICTION: fp_scene_spawn_friction(w); break;
    default: fp_scene_spawn_stack(w); break;
  }
}

const FpSceneSpawnTable* fp_scene_data_table_stack_core(void) {
  static const FpSceneSpawnTable t = {g_table_stack_core, (int)(sizeof(g_table_stack_core) / sizeof(g_table_stack_core[0]))};
  return &t;
}

const FpSceneSpawnTable* fp_scene_data_table_bounce_core(void) {
  static const FpSceneSpawnTable t = {g_table_bounce_core, (int)(sizeof(g_table_bounce_core) / sizeof(g_table_bounce_core[0]))};
  return &t;
}

const FpSceneSpawnTable* fp_scene_data_table_friction_core(void) {
  static const FpSceneSpawnTable t = {g_table_friction_core, (int)(sizeof(g_table_friction_core) / sizeof(g_table_friction_core[0]))};
  return &t;
}
