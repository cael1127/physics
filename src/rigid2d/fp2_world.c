#include "rigid2d/fp2_world.h"

#include "rigid2d/fp2_broadphase.h"
#include "rigid2d/fp2_collision.h"

#include "core/fp_assert.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(_MSC_VER)
  #include <malloc.h>
  #define FP2_ALLOCA _alloca
#else
  #include <alloca.h>
  #define FP2_ALLOCA alloca
#endif

static float fp2_cross2(FpVec2 a, FpVec2 b) { return a.x * b.y - a.y * b.x; }

static float fp2_shape_area(Fp2Shape s) {
  switch (s.type) {
    case FP2_SHAPE_CIRCLE: {
      float r = s.u.circle.radius;
      const float pi = 3.14159265358979323846f;
      return pi * r * r;
    }
    case FP2_SHAPE_BOX: {
      FpVec2 he = s.u.box.half_extents;
      return (2.0f * he.x) * (2.0f * he.y);
    }
    default: return 0.0f;
  }
}

static float fp2_shape_inertia(Fp2Shape s, float mass) {
  switch (s.type) {
    case FP2_SHAPE_CIRCLE: {
      float r = s.u.circle.radius;
      return 0.5f * mass * r * r;
    }
    case FP2_SHAPE_BOX: {
      // rectangle about center: I = (1/12) m (w^2 + h^2)
      float w = 2.0f * s.u.box.half_extents.x;
      float h = 2.0f * s.u.box.half_extents.y;
      return (mass * (w * w + h * h)) / 12.0f;
    }
    default: return 0.0f;
  }
}

static void fp2_world_compute_xforms(Fp2World* w) {
  for (int i = 0; i < w->body_count; i++) {
    w->xforms[i].p = w->bodies[i].p;
    w->xforms[i].R = fp_m22_rot(w->bodies[i].angle);
    w->aabbs[i] = fp2_aabb_from_shape(w->shapes[i], w->xforms[i]);
  }
}

void fp2_world_init(Fp2World* w, const Fp2WorldDesc* desc) {
  memset(w, 0, sizeof(*w));
  w->desc = *desc;
  w->bodies = (Fp2Body*)calloc((size_t)desc->max_bodies, sizeof(Fp2Body));
  w->shapes = (Fp2Shape*)calloc((size_t)desc->max_bodies, sizeof(Fp2Shape));
  w->xforms = (FpTransform2*)calloc((size_t)desc->max_bodies, sizeof(FpTransform2));
  w->aabbs = (Fp2Aabb*)calloc((size_t)desc->max_bodies, sizeof(Fp2Aabb));

  w->contacts = (Fp2Contact*)calloc((size_t)desc->max_contacts, sizeof(Fp2Contact));
  w->prev_contacts = (Fp2Contact*)calloc((size_t)desc->max_contacts, sizeof(Fp2Contact));
}

void fp2_world_destroy(Fp2World* w) {
  if (!w) return;
  free(w->bodies);
  free(w->shapes);
  free(w->xforms);
  free(w->aabbs);
  free(w->contacts);
  free(w->prev_contacts);
  memset(w, 0, sizeof(*w));
}

void fp2_world_reset(Fp2World* w) {
  w->body_count = 0;
  w->contact_count = 0;
  w->prev_contact_count = 0;
}

int fp2_world_add_body(Fp2World* w, int is_static, FpVec2 p, float angle, Fp2Shape shape, float density, float friction, float restitution) {
  FP_ASSERT(w->body_count < w->desc.max_bodies);
  int id = w->body_count++;

  Fp2Body* b = &w->bodies[id];
  memset(b, 0, sizeof(*b));
  b->is_static = is_static ? 1 : 0;
  b->p = p;
  b->angle = angle;
  b->v = fp_v2(0, 0);
  b->w = 0.0f;
  b->friction = friction;
  b->restitution = restitution;

  w->shapes[id] = shape;

  if (b->is_static) {
    b->inv_mass = 0.0f;
    b->inv_inertia = 0.0f;
  } else {
    float area = fp2_shape_area(shape);
    float mass = density * area;
    float inertia = fp2_shape_inertia(shape, mass);
    b->inv_mass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    b->inv_inertia = (inertia > 0.0f) ? (1.0f / inertia) : 0.0f;
  }

  return id;
}

static int fp2_find_prev_contact(Fp2World* w, int a, int b) {
  for (int i = 0; i < w->prev_contact_count; i++) {
    if (w->prev_contacts[i].a == a && w->prev_contacts[i].b == b) return i;
    if (w->prev_contacts[i].a == b && w->prev_contacts[i].b == a) return i;
  }
  return -1;
}

static void fp2_world_build_contacts(Fp2World* w) {
  // swap prev/current
  Fp2Contact* tmp = w->prev_contacts;
  w->prev_contacts = w->contacts;
  w->contacts = tmp;
  w->prev_contact_count = w->contact_count;
  w->contact_count = 0;

  fp2_world_compute_xforms(w);

  // Potential pairs
  const int max_pairs = w->desc.max_contacts * 2;
  Fp2Pair* pairs = (Fp2Pair*)FP2_ALLOCA((size_t)max_pairs * sizeof(Fp2Pair));
  int pair_count = fp2_broadphase_pairs(w->aabbs, w->body_count, pairs, max_pairs);

  for (int i = 0; i < pair_count; i++) {
    int a = pairs[i].a;
    int b = pairs[i].b;
    if (w->bodies[a].is_static && w->bodies[b].is_static) continue;

    Fp2Manifold m = fp2_collide(w->shapes[a], w->xforms[a], w->shapes[b], w->xforms[b]);
    if (!m.hit) continue;
    if (w->contact_count >= w->desc.max_contacts) break;

    Fp2Contact* c = &w->contacts[w->contact_count++];
    c->a = a;
    c->b = b;
    c->m = m;
    c->impulse_n = 0.0f;
    c->impulse_t = 0.0f;

    int prev = fp2_find_prev_contact(w, a, b);
    if (prev >= 0) {
      c->impulse_n = w->prev_contacts[prev].impulse_n;
      c->impulse_t = w->prev_contacts[prev].impulse_t;
    }
  }
}

void fp2_world_step(Fp2World* w, float dt, int vel_iters, int pos_iters) {
  fp2_world_build_contacts(w);

  // Integrate velocities with gravity
  fp2_integrate_vel(w->bodies, w->body_count, w->desc.gravity, dt);

  // Solve constraints with xforms from pre-step positions
  Fp2SolveDesc sd;
  sd.dt = dt;
  sd.velocity_iterations = vel_iters;
  sd.position_iterations = pos_iters;
  sd.gravity = w->desc.gravity;
  fp2_solve_contacts(w->bodies, w->xforms, w->contacts, w->contact_count, &sd);

  // Integrate positions
  fp2_integrate_pos(w->bodies, w->body_count, dt);
}

