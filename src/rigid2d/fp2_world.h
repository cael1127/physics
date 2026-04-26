#pragma once

#include "core/fp_math.h"
#include "rigid2d/fp2_broadphase.h"
#include "rigid2d/fp2_shape.h"
#include "rigid2d/fp2_solver.h"

typedef struct Fp2WorldDesc {
  int max_bodies;
  int max_contacts;
  int max_joints;
  FpVec2 gravity;
  Fp2BroadphaseMode broadphase_mode;
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

  int joint_count;
  Fp2DistanceJoint* joints;
  int revolute_joint_count;
  Fp2RevoluteJoint* revolute_joints;
} Fp2World;

typedef struct Fp2RayCastHit {
  int hit;
  int body_index;
  float fraction; // 0..1 along the tested ray segment
  FpVec2 point;
  FpVec2 normal;  // outward normal at impact
} Fp2RayCastHit;

void fp2_world_init(Fp2World* w, const Fp2WorldDesc* desc);
void fp2_world_destroy(Fp2World* w);
void fp2_world_reset(Fp2World* w);

int fp2_world_add_body(Fp2World* w, int is_static, FpVec2 p, float angle, Fp2Shape shape, float density, float friction, float restitution);
int fp2_world_add_distance_joint(
    Fp2World* w,
    int a,
    int b,
    FpVec2 local_anchor_a,
    FpVec2 local_anchor_b,
    float rest_length,
    float stiffness,
    float damping);
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
    float max_motor_torque);

void fp2_world_step(Fp2World* w, float dt, int vel_iters, int pos_iters);

// World-space force at center of mass; torque about +z (screen out). Skips static bodies.
void fp2_world_apply_body_force(Fp2World* w, int body_index, FpVec2 force, float torque_z, float dt);

// Casts a segment from origin to origin + direction * max_distance and returns closest hit.
Fp2RayCastHit fp2_world_raycast_closest(
    const Fp2World* w,
    FpVec2 origin,
    FpVec2 direction,
    float max_distance,
    int include_static);

// Returns the index of a body containing `point`, or -1 if none.
// If multiple bodies contain the point, returns the one with closest body center.
int fp2_world_query_point(const Fp2World* w, FpVec2 point, int include_static);

// Writes overlapping body indices to `out_body_indices` (up to `max_out`) and returns number written.
int fp2_world_query_aabb(
    const Fp2World* w,
    Fp2Aabb query,
    int include_static,
    int* out_body_indices,
    int max_out);

// Snapshot format is line-oriented text for readability and reproducibility.
int fp2_world_save_snapshot(const Fp2World* w, const char* path);
int fp2_world_load_snapshot(Fp2World* w, const char* path);

