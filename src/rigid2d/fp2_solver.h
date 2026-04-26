#pragma once

#include "core/fp_math.h"
#include "rigid2d/fp2_collision.h"

typedef struct Fp2Body {
  int is_static;
  FpVec2 p;
  float angle;
  FpVec2 v;
  float w;

  float inv_mass;
  float inv_inertia;

  float friction;
  float restitution;
} Fp2Body;

typedef struct Fp2Contact {
  int a;
  int b;
  Fp2Manifold m;
  float impulse_n[2];
  float impulse_t[2];
} Fp2Contact;

typedef struct Fp2DistanceJoint {
  int a;
  int b;
  FpVec2 local_anchor_a;
  FpVec2 local_anchor_b;
  float rest_length;
  float stiffness;
  float damping;
} Fp2DistanceJoint;

typedef struct Fp2RevoluteJoint {
  int a;
  int b;
  FpVec2 local_anchor_a;
  FpVec2 local_anchor_b;
  float stiffness;
  float damping;
  int motor_enabled;
  float motor_speed;
  float max_motor_torque;
} Fp2RevoluteJoint;

typedef struct Fp2SolveDesc {
  float dt;
  int velocity_iterations;
  int position_iterations;
  FpVec2 gravity;
} Fp2SolveDesc;

void fp2_integrate_vel(Fp2Body* bodies, int count, FpVec2 gravity, float dt);
void fp2_solve_contacts(Fp2Body* bodies, const FpTransform2* xforms, Fp2Contact* contacts, int contact_count, const Fp2SolveDesc* desc);
void fp2_solve_distance_joints(Fp2Body* bodies, Fp2DistanceJoint* joints, int joint_count, float dt, int iterations);
void fp2_solve_revolute_joints(Fp2Body* bodies, Fp2RevoluteJoint* joints, int joint_count, float dt, int iterations);
void fp2_integrate_pos(Fp2Body* bodies, int count, float dt);

