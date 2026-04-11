#pragma once

typedef struct Fp2World Fp2World;

typedef enum FpSceneDemoId {
  FP_SCENE_DEMO_STACK = 1,
  FP_SCENE_DEMO_BOUNCE = 2,
  FP_SCENE_DEMO_FRICTION = 3
} FpSceneDemoId;

typedef struct FpSceneSpawnRow {
  int is_static;
  float px, py, angle;
  int shape_kind; // 0 circle, 1 box
  float dim0;     // radius or hx
  float dim1;     // hy for box (ignored for circle)
  float density;
  float friction;
  float restitution;
} FpSceneSpawnRow;

typedef struct FpSceneSpawnTable {
  const FpSceneSpawnRow* rows;
  int row_count;
} FpSceneSpawnTable;

void fp_scene_data_apply_table(Fp2World* w, const FpSceneSpawnTable* t);
void fp_scene_data_build_world(Fp2World* w, int demo_id);

// Small declarative tables for tests / tools (subset of full demos).
const FpSceneSpawnTable* fp_scene_data_table_stack_core(void);
const FpSceneSpawnTable* fp_scene_data_table_bounce_core(void);
const FpSceneSpawnTable* fp_scene_data_table_friction_core(void);
