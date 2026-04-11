#include "runtime/fp_scene.h"

#include "runtime/fp_scene_data.h"

#include <string.h>

void fp_scene_params_default(FpSceneParams* p) {
  memset(p, 0, sizeof(*p));
  p->wind = fp_v2(0.0f, 0.0f);
  p->gravity_scale = 1.0f;
  p->fluid_emit_rate = 2.0f;
  p->aero_air_density = 0.4f;
  p->aero_drag = 0.35f;
  p->aero_lift = 0.0f;
  p->show_contacts = 1;
  p->show_fluid = 1;
  p->show_wind_overlay = 1;
}

void fp_scene_desc_default(FpSceneDesc* d) {
  memset(d, 0, sizeof(*d));
  d->world.max_bodies = 2048;
  d->world.max_contacts = 4096;
  d->world.gravity = fp_v2(0.0f, -9.81f);

  d->enable_fluid = 1;
  d->fluid.nx = 72;
  d->fluid.ny = 48;
  d->fluid.cell_size = 0.12f;
  d->fluid.origin = fp_v2(-4.5f, -4.0f);
  d->fluid.dye_diffusion = 0.12f;
  d->fluid.jacobi_diffuse_iters = 4;

  d->enable_particles = 1;
  d->particles.max_particles = 4096;
  d->particles.gravity = fp_v2(0.0f, -9.81f);
  d->particles.linear_drag = 0.35f;

  d->aero.wind = fp_v2(0.0f, 0.0f);
  d->aero.air_density = 0.4f;
  d->aero.drag_coefficient = 0.35f;
  d->aero.lift_coefficient = 0.0f;
}

void fp_scene_init(FpScene* s, const FpSceneDesc* desc) {
  memset(s, 0, sizeof(*s));
  s->desc = *desc;
  fp_scene_params_default(&s->params);
  s->params.wind = desc->aero.wind;
  s->params.aero_air_density = desc->aero.air_density;
  s->params.aero_drag = desc->aero.drag_coefficient;
  s->params.aero_lift = desc->aero.lift_coefficient;

  fp2_world_init(&s->world, &desc->world);

  if (desc->enable_fluid) {
    fp_fluid2d_init(&s->fluid, &desc->fluid);
    s->fluid_inited = 1;
  }
  if (desc->enable_particles) {
    fp_particles2d_init(&s->particles, &desc->particles);
    s->particles_inited = 1;
  }
}

void fp_scene_destroy(FpScene* s) {
  if (!s) return;
  fp2_world_destroy(&s->world);
  if (s->fluid_inited) fp_fluid2d_destroy(&s->fluid);
  if (s->particles_inited) fp_particles2d_destroy(&s->particles);
  memset(s, 0, sizeof(*s));
}

void fp_scene_load_demo(FpScene* s, int demo_id) {
  if (!s) return;
  fp_scene_data_build_world(&s->world, demo_id);
  if (s->fluid_inited) {
    fp_fluid2d_reset(&s->fluid);
  }
  if (s->particles_inited) {
    fp_particles2d_clear(&s->particles);
  }
}

void fp_scene_add_dye_at_world(FpScene* s, FpVec2 world_pos, float amount) {
  if (!s || !s->fluid_inited) return;
  fp_fluid2d_add_dye_gaussian(&s->fluid, world_pos, 0.35f, amount);
}

void fp_scene_step(FpScene* s, float dt, int vel_iters, int pos_iters) {
  if (!s || dt <= 0.0f) return;

  FpVec2 saved_gravity = s->world.desc.gravity;
  s->world.desc.gravity = fp_v2_mul(saved_gravity, s->params.gravity_scale);

  FpAero2dDesc ad = s->desc.aero;
  ad.wind = s->params.wind;
  ad.air_density = s->params.aero_air_density;
  ad.drag_coefficient = s->params.aero_drag;
  ad.lift_coefficient = s->params.aero_lift;

  if (s->fluid_inited) {
    fp_fluid2d_sync_solids_from_world(&s->fluid, &s->world);
    fp_fluid2d_set_wind(&s->fluid, s->params.wind);
  }

  fp_aero2d_apply_forces(&s->world, s->world.shapes, &ad, dt);

  if (s->fluid_inited) {
    fp_fluid2d_step(&s->fluid, dt);
  }

  fp2_world_step(&s->world, dt, vel_iters, pos_iters);

  if (s->particles_inited) {
    FpVec2 pg = fp_v2_mul(s->desc.particles.gravity, s->params.gravity_scale);
    FpVec2 saved_pg = s->particles.desc.gravity;
    s->particles.desc.gravity = pg;
    fp_particles2d_step(&s->particles, &s->world, s->world.shapes, s->params.wind, dt);
    s->particles.desc.gravity = saved_pg;
  }

  s->world.desc.gravity = saved_gravity;
}
