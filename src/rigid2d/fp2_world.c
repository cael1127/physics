#include "rigid2d/fp2_world.h"

#include "rigid2d/fp2_broadphase.h"
#include "rigid2d/fp2_collision.h"

#include "core/fp_assert.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#if defined(_MSC_VER)
  #include <malloc.h>
  #define FP2_ALLOCA _alloca
#else
  #include <alloca.h>
  #define FP2_ALLOCA alloca
#endif

static float fp2_cross2(FpVec2 a, FpVec2 b) { return a.x * b.y - a.y * b.x; }

static int fp2_aabb_overlap(Fp2Aabb a, Fp2Aabb b) {
  if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
  if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
  return 1;
}

static int fp2_point_in_shape(Fp2Shape s, FpTransform2 xf, FpVec2 p) {
  if (s.type == FP2_SHAPE_CIRCLE) {
    FpVec2 d = fp_v2_sub(p, xf.p);
    float r = s.u.circle.radius;
    return fp_v2_len2(d) <= (r * r);
  }
  if (s.type == FP2_SHAPE_BOX) {
    FpVec2 local = fp_xf2_tmul(xf, p);
    FpVec2 he = s.u.box.half_extents;
    if (local.x < -he.x || local.x > he.x) return 0;
    if (local.y < -he.y || local.y > he.y) return 0;
    return 1;
  }
  return 0;
}

static Fp2RayCastHit fp2_raycast_miss(void) {
  Fp2RayCastHit h;
  h.hit = 0;
  h.body_index = -1;
  h.fraction = 0.0f;
  h.point = fp_v2(0.0f, 0.0f);
  h.normal = fp_v2(0.0f, 1.0f);
  return h;
}

static int fp2_raycast_circle(
    FpVec2 origin,
    FpVec2 dir_norm,
    float max_dist,
    FpVec2 center,
    float radius,
    float* out_t,
    FpVec2* out_normal) {
  FpVec2 m = fp_v2_sub(origin, center);
  float b = fp_v2_dot(m, dir_norm);
  float c = fp_v2_dot(m, m) - radius * radius;
  if (c > 0.0f && b > 0.0f) return 0;

  float disc = b * b - c;
  if (disc < 0.0f) return 0;

  float t = -b - sqrtf(disc);
  if (t < 0.0f) t = 0.0f;
  if (t > max_dist) return 0;

  FpVec2 hit_point = fp_v2_mad(origin, t, dir_norm);
  FpVec2 n = fp_v2_sub(hit_point, center);
  n = fp_v2_norm(n);
  if (fp_v2_len2(n) < 1e-8f) n = fp_v2(1.0f, 0.0f);

  *out_t = t;
  *out_normal = n;
  return 1;
}

static int fp2_raycast_box(
    FpVec2 origin,
    FpVec2 dir_norm,
    float max_dist,
    FpTransform2 xf,
    FpVec2 he,
    float* out_t,
    FpVec2* out_normal) {
  FpVec2 o = fp_xf2_tmul(xf, origin);
  FpVec2 d = fp_m22_tmul_v2(xf.R, dir_norm);

  float tmin = 0.0f;
  float tmax = max_dist;
  FpVec2 normal_local = fp_v2(0.0f, 0.0f);

  const float eps = 1e-8f;
  for (int axis = 0; axis < 2; axis++) {
    float o_axis = (axis == 0) ? o.x : o.y;
    float d_axis = (axis == 0) ? d.x : d.y;
    float min_a = (axis == 0) ? -he.x : -he.y;
    float max_a = (axis == 0) ? he.x : he.y;

    if (fabsf(d_axis) < eps) {
      if (o_axis < min_a || o_axis > max_a) return 0;
      continue;
    }

    float inv_d = 1.0f / d_axis;
    float t1 = (min_a - o_axis) * inv_d;
    float t2 = (max_a - o_axis) * inv_d;
    float near_t = t1;
    float far_t = t2;
    FpVec2 near_normal = fp_v2(0.0f, 0.0f);
    if (axis == 0) near_normal = fp_v2(-1.0f, 0.0f);
    else near_normal = fp_v2(0.0f, -1.0f);
    if (t1 > t2) {
      near_t = t2;
      far_t = t1;
      near_normal = fp_v2_mul(near_normal, -1.0f);
    }

    if (near_t > tmin) {
      tmin = near_t;
      normal_local = near_normal;
    }
    if (far_t < tmax) tmax = far_t;
    if (tmin > tmax) return 0;
  }

  float t = tmin;
  if (t < 0.0f) t = 0.0f;
  if (t > max_dist) return 0;

  *out_t = t;
  *out_normal = fp_m22_mul_v2(xf.R, normal_local);
  return 1;
}

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
  if (w->desc.max_joints <= 0) w->desc.max_joints = w->desc.max_bodies;
  w->bodies = (Fp2Body*)calloc((size_t)desc->max_bodies, sizeof(Fp2Body));
  w->shapes = (Fp2Shape*)calloc((size_t)desc->max_bodies, sizeof(Fp2Shape));
  w->xforms = (FpTransform2*)calloc((size_t)desc->max_bodies, sizeof(FpTransform2));
  w->aabbs = (Fp2Aabb*)calloc((size_t)desc->max_bodies, sizeof(Fp2Aabb));

  w->contacts = (Fp2Contact*)calloc((size_t)desc->max_contacts, sizeof(Fp2Contact));
  w->prev_contacts = (Fp2Contact*)calloc((size_t)desc->max_contacts, sizeof(Fp2Contact));
  w->joints = (Fp2DistanceJoint*)calloc((size_t)w->desc.max_joints, sizeof(Fp2DistanceJoint));
  w->revolute_joints = (Fp2RevoluteJoint*)calloc((size_t)w->desc.max_joints, sizeof(Fp2RevoluteJoint));
}

void fp2_world_destroy(Fp2World* w) {
  if (!w) return;
  free(w->bodies);
  free(w->shapes);
  free(w->xforms);
  free(w->aabbs);
  free(w->contacts);
  free(w->prev_contacts);
  free(w->joints);
  free(w->revolute_joints);
  memset(w, 0, sizeof(*w));
}

void fp2_world_reset(Fp2World* w) {
  w->body_count = 0;
  w->contact_count = 0;
  w->prev_contact_count = 0;
  w->joint_count = 0;
  w->revolute_joint_count = 0;
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

int fp2_world_add_distance_joint(
    Fp2World* w,
    int a,
    int b,
    FpVec2 local_anchor_a,
    FpVec2 local_anchor_b,
    float rest_length,
    float stiffness,
    float damping) {
  if (!w) return -1;
  if (a < 0 || b < 0 || a >= w->body_count || b >= w->body_count) return -1;
  if (w->joint_count >= w->desc.max_joints) return -1;

  int id = w->joint_count++;
  Fp2DistanceJoint* j = &w->joints[id];
  j->a = a;
  j->b = b;
  j->local_anchor_a = local_anchor_a;
  j->local_anchor_b = local_anchor_b;
  j->rest_length = rest_length;
  j->stiffness = stiffness;
  j->damping = damping;
  return id;
}

int fp2_world_add_revolute_joint(
    Fp2World* w,
    int a,
    int b,
    FpVec2 local_anchor_a,
    FpVec2 local_anchor_b,
    float stiffness,
    float damping,
    int motor_enabled,
    float motor_speed,
    float max_motor_torque) {
  if (!w) return -1;
  if (a < 0 || b < 0 || a >= w->body_count || b >= w->body_count) return -1;
  if (w->revolute_joint_count >= w->desc.max_joints) return -1;

  int id = w->revolute_joint_count++;
  Fp2RevoluteJoint* j = &w->revolute_joints[id];
  j->a = a;
  j->b = b;
  j->local_anchor_a = local_anchor_a;
  j->local_anchor_b = local_anchor_b;
  j->stiffness = stiffness;
  j->damping = damping;
  j->motor_enabled = motor_enabled ? 1 : 0;
  j->motor_speed = motor_speed;
  j->max_motor_torque = max_motor_torque;
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
  int pair_count = fp2_broadphase_pairs(w->aabbs, w->body_count, pairs, max_pairs, w->desc.broadphase_mode);

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
    c->impulse_n[0] = 0.0f;
    c->impulse_n[1] = 0.0f;
    c->impulse_t[0] = 0.0f;
    c->impulse_t[1] = 0.0f;

    int prev = fp2_find_prev_contact(w, a, b);
    if (prev >= 0) {
      c->impulse_n[0] = w->prev_contacts[prev].impulse_n[0];
      c->impulse_n[1] = w->prev_contacts[prev].impulse_n[1];
      c->impulse_t[0] = w->prev_contacts[prev].impulse_t[0];
      c->impulse_t[1] = w->prev_contacts[prev].impulse_t[1];
    }
  }
}

void fp2_world_apply_body_force(Fp2World* w, int body_index, FpVec2 force, float torque_z, float dt) {
  if (!w || body_index < 0 || body_index >= w->body_count) return;
  Fp2Body* b = &w->bodies[body_index];
  if (b->is_static) return;
  b->v = fp_v2_mad(b->v, dt * b->inv_mass, force);
  b->w += dt * torque_z * b->inv_inertia;
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
  fp2_solve_distance_joints(w->bodies, w->joints, w->joint_count, dt, pos_iters);
  fp2_solve_revolute_joints(w->bodies, w->revolute_joints, w->revolute_joint_count, dt, pos_iters);

  // Integrate positions
  fp2_integrate_pos(w->bodies, w->body_count, dt);
}

Fp2RayCastHit fp2_world_raycast_closest(
    const Fp2World* w,
    FpVec2 origin,
    FpVec2 direction,
    float max_distance,
    int include_static) {
  if (!w || max_distance <= 0.0f) return fp2_raycast_miss();

  float dir_len = fp_v2_len(direction);
  if (dir_len <= 1e-8f) return fp2_raycast_miss();
  FpVec2 dir_norm = fp_v2_mul(direction, 1.0f / dir_len);

  Fp2RayCastHit best = fp2_raycast_miss();
  float best_t = max_distance;

  for (int i = 0; i < w->body_count; i++) {
    if (!include_static && w->bodies[i].is_static) continue;

    FpTransform2 xf;
    xf.p = w->bodies[i].p;
    xf.R = fp_m22_rot(w->bodies[i].angle);

    float t = 0.0f;
    FpVec2 n = fp_v2(0.0f, 1.0f);
    int hit = 0;
    Fp2Shape s = w->shapes[i];
    if (s.type == FP2_SHAPE_CIRCLE) {
      hit = fp2_raycast_circle(origin, dir_norm, max_distance, xf.p, s.u.circle.radius, &t, &n);
    } else if (s.type == FP2_SHAPE_BOX) {
      hit = fp2_raycast_box(origin, dir_norm, max_distance, xf, s.u.box.half_extents, &t, &n);
    }

    if (!hit || t > best_t) continue;
    best_t = t;
    best.hit = 1;
    best.body_index = i;
    best.fraction = max_distance > 0.0f ? (t / max_distance) : 0.0f;
    best.point = fp_v2_mad(origin, t, dir_norm);
    best.normal = fp_v2_norm(n);
  }

  return best;
}

int fp2_world_query_point(const Fp2World* w, FpVec2 point, int include_static) {
  if (!w) return -1;

  int best_idx = -1;
  float best_d2 = 0.0f;
  for (int i = 0; i < w->body_count; i++) {
    if (!include_static && w->bodies[i].is_static) continue;
    FpTransform2 xf;
    xf.p = w->bodies[i].p;
    xf.R = fp_m22_rot(w->bodies[i].angle);
    if (!fp2_point_in_shape(w->shapes[i], xf, point)) continue;

    float d2 = fp_v2_len2(fp_v2_sub(point, xf.p));
    if (best_idx < 0 || d2 < best_d2) {
      best_idx = i;
      best_d2 = d2;
    }
  }
  return best_idx;
}

int fp2_world_query_aabb(
    const Fp2World* w,
    Fp2Aabb query,
    int include_static,
    int* out_body_indices,
    int max_out) {
  if (!w || max_out < 0) return 0;

  int written = 0;
  for (int i = 0; i < w->body_count; i++) {
    if (!include_static && w->bodies[i].is_static) continue;

    FpTransform2 xf;
    xf.p = w->bodies[i].p;
    xf.R = fp_m22_rot(w->bodies[i].angle);
    Fp2Aabb body_aabb = fp2_aabb_from_shape(w->shapes[i], xf);
    if (!fp2_aabb_overlap(query, body_aabb)) continue;

    if (written < max_out && out_body_indices) {
      out_body_indices[written] = i;
    }
    if (written < max_out) written++;
  }
  return written;
}

int fp2_world_save_snapshot(const Fp2World* w, const char* path) {
  if (!w || !path) return 0;
  FILE* f = fopen(path, "w");
  if (!f) return 0;

  fprintf(f, "FP2SNAP 2\n");
  fprintf(
      f,
      "DESC %d %d %d %.9g %.9g %d\n",
      w->desc.max_bodies,
      w->desc.max_contacts,
      w->desc.max_joints,
      (double)w->desc.gravity.x,
      (double)w->desc.gravity.y,
      (int)w->desc.broadphase_mode);
  fprintf(f, "COUNTS %d %d %d\n", w->body_count, w->joint_count, w->revolute_joint_count);

  for (int i = 0; i < w->body_count; i++) {
    const Fp2Body* b = &w->bodies[i];
    const Fp2Shape* s = &w->shapes[i];
    fprintf(
        f,
        "BODY %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %d %.9g %.9g\n",
        i,
        b->is_static,
        (double)b->p.x,
        (double)b->p.y,
        (double)b->angle,
        (double)b->v.x,
        (double)b->v.y,
        (double)b->w,
        (double)b->inv_mass,
        (double)b->inv_inertia,
        (double)b->friction,
        (double)b->restitution,
        s->type,
        (double)(s->type == FP2_SHAPE_CIRCLE ? s->u.circle.radius : s->u.box.half_extents.x),
        (double)(s->type == FP2_SHAPE_CIRCLE ? 0.0f : s->u.box.half_extents.y));
  }

  for (int i = 0; i < w->joint_count; i++) {
    const Fp2DistanceJoint* j = &w->joints[i];
    fprintf(
        f,
        "JOINT %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
        i,
        j->a,
        j->b,
        (double)j->local_anchor_a.x,
        (double)j->local_anchor_a.y,
        (double)j->local_anchor_b.x,
        (double)j->local_anchor_b.y,
        (double)j->rest_length,
        (double)j->stiffness,
        (double)j->damping);
  }
  for (int i = 0; i < w->revolute_joint_count; i++) {
    const Fp2RevoluteJoint* j = &w->revolute_joints[i];
    fprintf(
        f,
        "RJOINT %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %d %.9g %.9g\n",
        i,
        j->a,
        j->b,
        (double)j->local_anchor_a.x,
        (double)j->local_anchor_a.y,
        (double)j->local_anchor_b.x,
        (double)j->local_anchor_b.y,
        (double)j->stiffness,
        (double)j->damping,
        j->motor_enabled,
        (double)j->motor_speed,
        (double)j->max_motor_torque);
  }

  fclose(f);
  return 1;
}

int fp2_world_load_snapshot(Fp2World* w, const char* path) {
  if (!w || !path) return 0;
  FILE* f = fopen(path, "r");
  if (!f) return 0;

  char magic[16] = {0};
  int version = 0;
  if (fscanf(f, "%15s %d", magic, &version) != 2) {
    fclose(f);
    return 0;
  }
  if (strcmp(magic, "FP2SNAP") != 0 || (version != 1 && version != 2)) {
    fclose(f);
    return 0;
  }

  int max_bodies = 0, max_contacts = 0, max_joints = 0, broadphase_mode = 0;
  float gx = 0.0f, gy = 0.0f;
  if (fscanf(f, " DESC %d %d %d %f %f %d", &max_bodies, &max_contacts, &max_joints, &gx, &gy, &broadphase_mode) != 6) {
    fclose(f);
    return 0;
  }
  if (max_bodies > w->desc.max_bodies || max_contacts > w->desc.max_contacts || max_joints > w->desc.max_joints) {
    fclose(f);
    return 0;
  }

  int body_count = 0, joint_count = 0, revolute_joint_count = 0;
  if (version == 1) {
    if (fscanf(f, " COUNTS %d %d", &body_count, &joint_count) != 2) {
      fclose(f);
      return 0;
    }
  } else {
    if (fscanf(f, " COUNTS %d %d %d", &body_count, &joint_count, &revolute_joint_count) != 3) {
      fclose(f);
      return 0;
    }
  }
  if (body_count > w->desc.max_bodies || joint_count > w->desc.max_joints || revolute_joint_count > w->desc.max_joints) {
    fclose(f);
    return 0;
  }

  w->desc.gravity = fp_v2(gx, gy);
  w->desc.broadphase_mode = (Fp2BroadphaseMode)broadphase_mode;
  w->body_count = body_count;
  w->joint_count = joint_count;
  w->revolute_joint_count = revolute_joint_count;
  w->contact_count = 0;
  w->prev_contact_count = 0;

  for (int i = 0; i < body_count; i++) {
    int idx = 0, is_static = 0, shape_type = 0;
    float px = 0.0f, py = 0.0f, angle = 0.0f, vx = 0.0f, vy = 0.0f, w_ang = 0.0f;
    float inv_mass = 0.0f, inv_inertia = 0.0f, friction = 0.0f, restitution = 0.0f, shape_a = 0.0f, shape_b = 0.0f;
    if (fscanf(
            f,
            " BODY %d %d %f %f %f %f %f %f %f %f %f %f %d %f %f",
            &idx,
            &is_static,
            &px,
            &py,
            &angle,
            &vx,
            &vy,
            &w_ang,
            &inv_mass,
            &inv_inertia,
            &friction,
            &restitution,
            &shape_type,
            &shape_a,
            &shape_b) != 15) {
      fclose(f);
      return 0;
    }
    if (idx < 0 || idx >= body_count) {
      fclose(f);
      return 0;
    }

    Fp2Body* b = &w->bodies[idx];
    b->is_static = is_static;
    b->p = fp_v2(px, py);
    b->angle = angle;
    b->v = fp_v2(vx, vy);
    b->w = w_ang;
    b->inv_mass = inv_mass;
    b->inv_inertia = inv_inertia;
    b->friction = friction;
    b->restitution = restitution;

    if (shape_type == FP2_SHAPE_CIRCLE) {
      w->shapes[idx] = fp2_shape_circle(shape_a);
    } else {
      w->shapes[idx] = fp2_shape_box(shape_a, shape_b);
    }
  }

  for (int i = 0; i < joint_count; i++) {
    int idx = 0, a = 0, b = 0;
    float lax = 0.0f, lay = 0.0f, lbx = 0.0f, lby = 0.0f, rest = 0.0f, stiff = 0.0f, damp = 0.0f;
    if (fscanf(
            f,
            " JOINT %d %d %d %f %f %f %f %f %f %f",
            &idx,
            &a,
            &b,
            &lax,
            &lay,
            &lbx,
            &lby,
            &rest,
            &stiff,
            &damp) != 10) {
      fclose(f);
      return 0;
    }
    if (idx < 0 || idx >= joint_count) {
      fclose(f);
      return 0;
    }
    w->joints[idx].a = a;
    w->joints[idx].b = b;
    w->joints[idx].local_anchor_a = fp_v2(lax, lay);
    w->joints[idx].local_anchor_b = fp_v2(lbx, lby);
    w->joints[idx].rest_length = rest;
    w->joints[idx].stiffness = stiff;
    w->joints[idx].damping = damp;
  }
  for (int i = 0; i < revolute_joint_count; i++) {
    int idx = 0, a = 0, b = 0, motor_enabled = 0;
    float lax = 0.0f, lay = 0.0f, lbx = 0.0f, lby = 0.0f, stiff = 0.0f, damp = 0.0f, motor_speed = 0.0f, max_motor_torque = 0.0f;
    if (fscanf(
            f,
            " RJOINT %d %d %d %f %f %f %f %f %f %d %f %f",
            &idx,
            &a,
            &b,
            &lax,
            &lay,
            &lbx,
            &lby,
            &stiff,
            &damp,
            &motor_enabled,
            &motor_speed,
            &max_motor_torque) != 12) {
      fclose(f);
      return 0;
    }
    if (idx < 0 || idx >= revolute_joint_count) {
      fclose(f);
      return 0;
    }
    w->revolute_joints[idx].a = a;
    w->revolute_joints[idx].b = b;
    w->revolute_joints[idx].local_anchor_a = fp_v2(lax, lay);
    w->revolute_joints[idx].local_anchor_b = fp_v2(lbx, lby);
    w->revolute_joints[idx].stiffness = stiff;
    w->revolute_joints[idx].damping = damp;
    w->revolute_joints[idx].motor_enabled = motor_enabled;
    w->revolute_joints[idx].motor_speed = motor_speed;
    w->revolute_joints[idx].max_motor_torque = max_motor_torque;
  }

  fclose(f);
  return 1;
}

