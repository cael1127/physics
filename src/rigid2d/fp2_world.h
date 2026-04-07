#pragma once

#include "core/fp_math.h"
#include "rigid2d/fp2_shape.h"
#include "rigid2d/fp2_solver.h"

typedef struct Fp2WorldDesc {
  int max_bodies;
  int max_contacts;
  FpVec2 gravity;
} Fp2WorldDesc;

typedef struct Fp2World {
  Fp2WorldDesc desc;

  int body_count;
  Fp2Body* bodies;
  Fp2Shape* shapes;
  FpTransform2* xforms;
  Fp2Aabb* aabbs;

  int contact_count;
  Fp2Contact* contacts;
  int prev_contact_count;
  Fp2Contact* prev_contacts;
} Fp2World;

void fp2_world_init(Fp2World* w, const Fp2WorldDesc* desc);
void fp2_world_destroy(Fp2World* w);
void fp2_world_reset(Fp2World* w);

int fp2_world_add_body(Fp2World* w, int is_static, FpVec2 p, float angle, Fp2Shape shape, float density, float friction, float restitution);

void fp2_world_step(Fp2World* w, float dt, int vel_iters, int pos_iters);

