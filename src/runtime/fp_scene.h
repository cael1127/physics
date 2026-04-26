#pragma once

#include "aero2d/fp_aero2d.h"
#include "core/fp_math.h"
#include "fluid2d/fp_fluid2d.h"
#include "particles2d/fp_particles2d.h"
#include "rigid2d/fp2_world.h"

typedef struct FpSceneParams {
  FpVec2 wind;
  float gravity_scale;
  float fluid_emit_rate;
  float aero_air_density;
  float aero_drag;
  float aero_lift;
  int show_contacts;
  int show_fluid;
  int show_wind_overlay;
} FpSceneParams;

typedef struct FpSceneDesc {
  Fp2WorldDesc world;
  int enable_fluid;
  FpFluid2dDesc fluid;
  int enable_particles;
  FpParticles2dDesc particles;
  FpAero2dDesc aero;
} FpSceneDesc;

typedef struct FpScene {
  FpSceneDesc desc;
  FpSceneParams params;
  Fp2World world;
  FpFluid2d fluid;
  FpParticles2d particles;
  int fluid_inited;
  int particles_inited;
} FpScene;

void fp_scene_params_default(FpSceneParams* p);
void fp_scene_desc_default(FpSceneDesc* d);

void fp_scene_init(FpScene* s, const FpSceneDesc* desc);
void fp_scene_destroy(FpScene* s);

// Rebuild rigid bodies from declarative demo id (see runtime/fp_scene_data.h).
void fp_scene_load_demo(FpScene* s, int demo_id);

// Documented order: sync fluid solids → fluid wind → aero on rigids → fluid step → rigid step → particles.
void fp_scene_step(FpScene* s, float dt, int vel_iters, int pos_iters);

void fp_scene_add_dye_at_world(FpScene* s, FpVec2 world_pos, float amount);
int fp_scene_save_snapshot(const FpScene* s, const char* path);
int fp_scene_load_snapshot(FpScene* s, const char* path);
