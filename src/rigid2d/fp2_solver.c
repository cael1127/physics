#include "rigid2d/fp2_solver.h"

#include <math.h>

static float fp2_clampf(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

static float fp2_cross2(FpVec2 a, FpVec2 b) { return a.x * b.y - a.y * b.x; }
static FpVec2 fp2_cross_sv(float s, FpVec2 v) { return fp_v2(-s * v.y, s * v.x); }

void fp2_integrate_vel(Fp2Body* bodies, int count, FpVec2 gravity, float dt) {
  for (int i = 0; i < count; i++) {
    if (bodies[i].is_static) continue;
    bodies[i].v = fp_v2_add(bodies[i].v, fp_v2_mul(gravity, dt));
  }
}

void fp2_integrate_pos(Fp2Body* bodies, int count, float dt) {
  for (int i = 0; i < count; i++) {
    if (bodies[i].is_static) continue;
    bodies[i].p = fp_v2_mad(bodies[i].p, dt, bodies[i].v);
    bodies[i].angle += dt * bodies[i].w;
  }
}

static void fp2_apply_impulse(Fp2Body* A, Fp2Body* B, FpVec2 ra, FpVec2 rb, FpVec2 P) {
  A->v = fp_v2_sub(A->v, fp_v2_mul(P, A->inv_mass));
  A->w -= A->inv_inertia * fp2_cross2(ra, P);
  B->v = fp_v2_add(B->v, fp_v2_mul(P, B->inv_mass));
  B->w += B->inv_inertia * fp2_cross2(rb, P);
}

static float fp2_effective_mass(Fp2Body* A, Fp2Body* B, FpVec2 ra, FpVec2 rb, FpVec2 n) {
  float rnA = fp2_cross2(ra, n);
  float rnB = fp2_cross2(rb, n);
  float k = A->inv_mass + B->inv_mass + rnA * rnA * A->inv_inertia + rnB * rnB * B->inv_inertia;
  if (k <= 0.0f) return 0.0f;
  return 1.0f / k;
}

void fp2_solve_contacts(Fp2Body* bodies, const FpTransform2* xforms, Fp2Contact* contacts, int contact_count, const Fp2SolveDesc* desc) {
  const float baumgarte = 0.2f;
  const float slop = 0.01f;

  // Warm start
  for (int i = 0; i < contact_count; i++) {
    Fp2Contact* c = &contacts[i];
    if (!c->m.hit) continue;
    Fp2Body* A = &bodies[c->a];
    Fp2Body* B = &bodies[c->b];
    FpVec2 n = c->m.normal;
    FpVec2 t = fp_v2_perp(n);
    int cp_count = c->m.cp_count > 2 ? 2 : c->m.cp_count;
    for (int cp = 0; cp < cp_count; cp++) {
      FpVec2 p = c->m.cps[cp].p;
      FpVec2 ra = fp_v2_sub(p, xforms[c->a].p);
      FpVec2 rb = fp_v2_sub(p, xforms[c->b].p);
      FpVec2 P = fp_v2_add(fp_v2_mul(n, c->impulse_n[cp]), fp_v2_mul(t, c->impulse_t[cp]));
      fp2_apply_impulse(A, B, ra, rb, P);
    }
  }

  // Velocity iterations
  for (int it = 0; it < desc->velocity_iterations; it++) {
    for (int i = 0; i < contact_count; i++) {
      Fp2Contact* c = &contacts[i];
      if (!c->m.hit) continue;
      Fp2Body* A = &bodies[c->a];
      Fp2Body* B = &bodies[c->b];
      FpVec2 n = c->m.normal;
      FpVec2 t = fp_v2_perp(n);
      int cp_count = c->m.cp_count > 2 ? 2 : c->m.cp_count;
      for (int cp = 0; cp < cp_count; cp++) {
        FpVec2 p = c->m.cps[cp].p;
        FpVec2 ra = fp_v2_sub(p, xforms[c->a].p);
        FpVec2 rb = fp_v2_sub(p, xforms[c->b].p);

        FpVec2 va = fp_v2_add(A->v, fp2_cross_sv(A->w, ra));
        FpVec2 vb = fp_v2_add(B->v, fp2_cross_sv(B->w, rb));
        FpVec2 dv = fp_v2_sub(vb, va);
        float vn = fp_v2_dot(dv, n);

        float e = (A->restitution + B->restitution) * 0.5f;
        float bounce = 0.0f;
        if (vn < -1.0f) bounce = -e * vn;

        float mn = fp2_effective_mass(A, B, ra, rb, n);
        float lambda_n = mn * (-(vn) + bounce);
        float new_impulse_n = c->impulse_n[cp] + lambda_n;
        if (new_impulse_n < 0.0f) new_impulse_n = 0.0f;
        lambda_n = new_impulse_n - c->impulse_n[cp];
        c->impulse_n[cp] = new_impulse_n;

        FpVec2 Pn = fp_v2_mul(n, lambda_n);
        fp2_apply_impulse(A, B, ra, rb, Pn);

        // Friction
        va = fp_v2_add(A->v, fp2_cross_sv(A->w, ra));
        vb = fp_v2_add(B->v, fp2_cross_sv(B->w, rb));
        dv = fp_v2_sub(vb, va);

        float vt = fp_v2_dot(dv, t);
        float mt = fp2_effective_mass(A, B, ra, rb, t);
        float lambda_t = mt * (-vt);

        float mu = (A->friction + B->friction) * 0.5f;
        float max_f = mu * c->impulse_n[cp];
        float new_impulse_t = fp2_clampf(c->impulse_t[cp] + lambda_t, -max_f, max_f);
        lambda_t = new_impulse_t - c->impulse_t[cp];
        c->impulse_t[cp] = new_impulse_t;

        FpVec2 Pt = fp_v2_mul(t, lambda_t);
        fp2_apply_impulse(A, B, ra, rb, Pt);
      }
    }
  }

  // Position iterations (Baumgarte-style correction)
  for (int it = 0; it < desc->position_iterations; it++) {
    for (int i = 0; i < contact_count; i++) {
      Fp2Contact* c = &contacts[i];
      if (!c->m.hit) continue;
      Fp2Body* A = &bodies[c->a];
      Fp2Body* B = &bodies[c->b];

      float pen = c->m.penetration;
      float C = fmaxf(pen - slop, 0.0f);
      if (C <= 0.0f) continue;

      FpVec2 n = c->m.normal;
      float inv_mass_sum = A->inv_mass + B->inv_mass;
      if (inv_mass_sum <= 0.0f) continue;

      float corr = (baumgarte * C);
      FpVec2 P = fp_v2_mul(n, corr / inv_mass_sum);
      if (!A->is_static) A->p = fp_v2_sub(A->p, fp_v2_mul(P, A->inv_mass));
      if (!B->is_static) B->p = fp_v2_add(B->p, fp_v2_mul(P, B->inv_mass));
    }
  }
}

void fp2_solve_distance_joints(Fp2Body* bodies, Fp2DistanceJoint* joints, int joint_count, float dt, int iterations) {
  if (!bodies || !joints || joint_count <= 0 || iterations <= 0 || dt <= 0.0f) return;

  for (int it = 0; it < iterations; it++) {
    for (int i = 0; i < joint_count; i++) {
      Fp2DistanceJoint* j = &joints[i];
      Fp2Body* A = &bodies[j->a];
      Fp2Body* B = &bodies[j->b];

      FpVec2 wa = fp_m22_mul_v2(fp_m22_rot(A->angle), j->local_anchor_a);
      FpVec2 wb = fp_m22_mul_v2(fp_m22_rot(B->angle), j->local_anchor_b);
      FpVec2 pa = fp_v2_add(A->p, wa);
      FpVec2 pb = fp_v2_add(B->p, wb);
      FpVec2 d = fp_v2_sub(pb, pa);
      float dist = fp_v2_len(d);
      if (dist <= 1e-6f) continue;

      FpVec2 n = fp_v2_mul(d, 1.0f / dist);
      float C = dist - j->rest_length;
      float inv_mass = A->inv_mass + B->inv_mass;
      if (inv_mass <= 0.0f) continue;

      float beta = j->stiffness > 0.0f ? j->stiffness : 0.7f;
      float impulse = -(beta * C) / inv_mass;
      FpVec2 P = fp_v2_mul(n, impulse);

      if (!A->is_static) A->p = fp_v2_sub(A->p, fp_v2_mul(P, A->inv_mass));
      if (!B->is_static) B->p = fp_v2_add(B->p, fp_v2_mul(P, B->inv_mass));

      // Lightweight damping along the joint axis.
      FpVec2 rel_v = fp_v2_sub(B->v, A->v);
      float rel_n = fp_v2_dot(rel_v, n);
      float damp = j->damping;
      if (damp > 0.0f) {
        float jd = -damp * rel_n / inv_mass;
        FpVec2 Pd = fp_v2_mul(n, jd);
        if (!A->is_static) A->v = fp_v2_sub(A->v, fp_v2_mul(Pd, A->inv_mass));
        if (!B->is_static) B->v = fp_v2_add(B->v, fp_v2_mul(Pd, B->inv_mass));
      }
    }
  }
}

void fp2_solve_revolute_joints(Fp2Body* bodies, Fp2RevoluteJoint* joints, int joint_count, float dt, int iterations) {
  if (!bodies || !joints || joint_count <= 0 || iterations <= 0 || dt <= 0.0f) return;

  for (int it = 0; it < iterations; it++) {
    for (int i = 0; i < joint_count; i++) {
      Fp2RevoluteJoint* j = &joints[i];
      Fp2Body* A = &bodies[j->a];
      Fp2Body* B = &bodies[j->b];

      FpVec2 wa = fp_m22_mul_v2(fp_m22_rot(A->angle), j->local_anchor_a);
      FpVec2 wb = fp_m22_mul_v2(fp_m22_rot(B->angle), j->local_anchor_b);
      FpVec2 pa = fp_v2_add(A->p, wa);
      FpVec2 pb = fp_v2_add(B->p, wb);

      // Positional part: keep anchors coincident.
      FpVec2 e = fp_v2_sub(pb, pa);
      float inv_mass = A->inv_mass + B->inv_mass;
      if (inv_mass > 0.0f) {
        float beta = j->stiffness > 0.0f ? j->stiffness : 0.7f;
        FpVec2 corr = fp_v2_mul(e, -beta / inv_mass);
        if (!A->is_static) A->p = fp_v2_add(A->p, fp_v2_mul(corr, A->inv_mass));
        if (!B->is_static) B->p = fp_v2_sub(B->p, fp_v2_mul(corr, B->inv_mass));
      }

      // Damping on relative angular velocity.
      float ang_rel = B->w - A->w;
      float inv_i = A->inv_inertia + B->inv_inertia;
      if (inv_i > 0.0f && j->damping > 0.0f) {
        float jd = -(j->damping * ang_rel) / inv_i;
        if (!A->is_static) A->w -= jd * A->inv_inertia;
        if (!B->is_static) B->w += jd * B->inv_inertia;
      }

      // Optional velocity motor.
      if (j->motor_enabled && inv_i > 0.0f) {
        float target_rel = j->motor_speed;
        float rel_err = (B->w - A->w) - target_rel;
        float lambda = -rel_err / inv_i;
        float max_impulse = j->max_motor_torque * dt;
        lambda = fp2_clampf(lambda, -max_impulse, max_impulse);
        if (!A->is_static) A->w -= lambda * A->inv_inertia;
        if (!B->is_static) B->w += lambda * B->inv_inertia;
      }
    }
  }
}

