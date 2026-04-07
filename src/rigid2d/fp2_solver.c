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
    FpVec2 p = c->m.cp.p;
    FpVec2 ra = fp_v2_sub(p, xforms[c->a].p);
    FpVec2 rb = fp_v2_sub(p, xforms[c->b].p);

    FpVec2 n = c->m.normal;
    FpVec2 t = fp_v2_perp(n);
    FpVec2 P = fp_v2_add(fp_v2_mul(n, c->impulse_n), fp_v2_mul(t, c->impulse_t));
    fp2_apply_impulse(A, B, ra, rb, P);
  }

  // Velocity iterations
  for (int it = 0; it < desc->velocity_iterations; it++) {
    for (int i = 0; i < contact_count; i++) {
      Fp2Contact* c = &contacts[i];
      if (!c->m.hit) continue;
      Fp2Body* A = &bodies[c->a];
      Fp2Body* B = &bodies[c->b];

      FpVec2 p = c->m.cp.p;
      FpVec2 ra = fp_v2_sub(p, xforms[c->a].p);
      FpVec2 rb = fp_v2_sub(p, xforms[c->b].p);

      FpVec2 va = fp_v2_add(A->v, fp2_cross_sv(A->w, ra));
      FpVec2 vb = fp_v2_add(B->v, fp2_cross_sv(B->w, rb));
      FpVec2 dv = fp_v2_sub(vb, va);

      FpVec2 n = c->m.normal;
      float vn = fp_v2_dot(dv, n);

      float e = (A->restitution + B->restitution) * 0.5f;
      float bounce = 0.0f;
      if (vn < -1.0f) bounce = -e * vn;

      float mn = fp2_effective_mass(A, B, ra, rb, n);
      float lambda_n = mn * (-(vn) + bounce);
      float new_impulse_n = c->impulse_n + lambda_n;
      if (new_impulse_n < 0.0f) new_impulse_n = 0.0f;
      lambda_n = new_impulse_n - c->impulse_n;
      c->impulse_n = new_impulse_n;

      FpVec2 Pn = fp_v2_mul(n, lambda_n);
      fp2_apply_impulse(A, B, ra, rb, Pn);

      // Friction
      va = fp_v2_add(A->v, fp2_cross_sv(A->w, ra));
      vb = fp_v2_add(B->v, fp2_cross_sv(B->w, rb));
      dv = fp_v2_sub(vb, va);

      FpVec2 t = fp_v2_perp(n);
      float vt = fp_v2_dot(dv, t);
      float mt = fp2_effective_mass(A, B, ra, rb, t);
      float lambda_t = mt * (-vt);

      float mu = (A->friction + B->friction) * 0.5f;
      float max_f = mu * c->impulse_n;
      float new_impulse_t = fp2_clampf(c->impulse_t + lambda_t, -max_f, max_f);
      lambda_t = new_impulse_t - c->impulse_t;
      c->impulse_t = new_impulse_t;

      FpVec2 Pt = fp_v2_mul(t, lambda_t);
      fp2_apply_impulse(A, B, ra, rb, Pt);
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

