#include "rigid2d/fp2_collision.h"

#include <math.h>

static float fp2_clampf(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

static Fp2Manifold fp2_miss(void) {
  Fp2Manifold m;
  m.hit = 0;
  m.normal = fp_v2(0, 1);
  m.penetration = 0.0f;
  m.cp_count = 0;
  m.cps[0].p = fp_v2(0, 0);
  m.cps[0].separation = 0.0f;
  m.cps[1].p = fp_v2(0, 0);
  m.cps[1].separation = 0.0f;
  return m;
}

static Fp2Manifold fp2_circle_circle(float ra, FpVec2 pa, float rb, FpVec2 pb) {
  FpVec2 d = fp_v2_sub(pb, pa);
  float dist2 = fp_v2_len2(d);
  float r = ra + rb;
  if (dist2 > r * r) return fp2_miss();
  float dist = sqrtf(dist2);
  Fp2Manifold m;
  m.hit = 1;
  if (dist > 1e-8f) {
    m.normal = fp_v2_mul(d, 1.0f / dist);
  } else {
    m.normal = fp_v2(1.0f, 0.0f);
  }
  m.penetration = r - dist;
  m.cp_count = 1;
  m.cps[0].p = fp_v2_add(pa, fp_v2_mul(m.normal, ra));
  m.cps[0].separation = -m.penetration;
  m.cps[1].p = m.cps[0].p;
  m.cps[1].separation = m.cps[0].separation;
  return m;
}

static void fp2_box_axes(FpTransform2 xf, FpVec2* ax, FpVec2* ay) {
  *ax = xf.R.c0;
  *ay = xf.R.c1;
}

static float fp2_project_box(FpVec2 center, FpVec2 ax, FpVec2 ay, FpVec2 he, FpVec2 n) {
  // projection radius of OBB onto axis n
  float r = fabsf(fp_v2_dot(ax, n)) * he.x + fabsf(fp_v2_dot(ay, n)) * he.y;
  float c = fp_v2_dot(center, n);
  (void)c;
  return r;
}

static Fp2Manifold fp2_box_box(Fp2Box a, FpTransform2 xa, Fp2Box b, FpTransform2 xb) {
  // SAT on 4 axes (a.x, a.y, b.x, b.y)
  FpVec2 ax, ay, bx, by;
  fp2_box_axes(xa, &ax, &ay);
  fp2_box_axes(xb, &bx, &by);

  FpVec2 d = fp_v2_sub(xb.p, xa.p);

  FpVec2 axes[4] = {ax, ay, bx, by};
  float best_pen = 1e30f;
  FpVec2 best_axis = ax;

  for (int i = 0; i < 4; i++) {
    FpVec2 n = fp_v2_norm(axes[i]);
    float ra = fp2_project_box(xa.p, ax, ay, a.half_extents, n);
    float rb = fp2_project_box(xb.p, bx, by, b.half_extents, n);
    float dist = fabsf(fp_v2_dot(d, n));
    float pen = (ra + rb) - dist;
    if (pen < 0.0f) return fp2_miss();
    if (pen < best_pen) {
      best_pen = pen;
      best_axis = n;
      // Ensure best_axis points from A to B
      if (fp_v2_dot(d, best_axis) < 0.0f) best_axis = fp_v2_mul(best_axis, -1.0f);
    }
  }

  Fp2Manifold m;
  m.hit = 1;
  m.normal = best_axis;
  m.penetration = best_pen;
  m.cp_count = 2;
  // Approximate 2-point patch along tangent for better stack stability than a single point.
  FpVec2 t = fp_v2_norm(fp_v2_perp(m.normal));
  float extent = fminf(a.half_extents.x + a.half_extents.y, b.half_extents.x + b.half_extents.y) * 0.35f;
  FpVec2 base = fp_v2_mul(fp_v2_add(xa.p, xb.p), 0.5f);
  base = fp_v2_sub(base, fp_v2_mul(m.normal, 0.5f * m.penetration));
  m.cps[0].p = fp_v2_add(base, fp_v2_mul(t, extent));
  m.cps[1].p = fp_v2_sub(base, fp_v2_mul(t, extent));
  m.cps[0].separation = -m.penetration;
  m.cps[1].separation = -m.penetration;
  return m;
}

static Fp2Manifold fp2_circle_box(float r, FpVec2 pc, Fp2Box box, FpTransform2 xb, int circle_is_a) {
  // circle vs OBB: transform circle center into box local space
  FpVec2 local = fp_xf2_tmul(xb, pc);
  FpVec2 he = box.half_extents;
  FpVec2 closest = fp_v2(fp2_clampf(local.x, -he.x, he.x), fp2_clampf(local.y, -he.y, he.y));
  FpVec2 diff = fp_v2_sub(local, closest);
  float dist2 = fp_v2_len2(diff);
  if (dist2 > r * r) return fp2_miss();

  float dist = sqrtf(dist2);
  FpVec2 n_local;
  if (dist > 1e-8f) n_local = fp_v2_mul(diff, 1.0f / dist);
  else n_local = fp_v2(1.0f, 0.0f);

  // normal in world
  FpVec2 n_world = fp_m22_mul_v2(xb.R, n_local);
  FpVec2 p_world = fp_xf2_mul(xb, closest);

  Fp2Manifold m;
  m.hit = 1;
  m.penetration = r - dist;
  // normal should be from A to B
  if (circle_is_a) {
    // A=circle, B=box: normal points from circle to box => opposite of n_world
    m.normal = fp_v2_mul(n_world, -1.0f);
  } else {
    // A=box, B=circle: normal points from box to circle => n_world
    m.normal = n_world;
  }
  m.cp_count = 1;
  m.cps[0].p = p_world;
  m.cps[0].separation = -m.penetration;
  m.cps[1].p = p_world;
  m.cps[1].separation = -m.penetration;
  return m;
}

Fp2Manifold fp2_collide(Fp2Shape a, FpTransform2 xa, Fp2Shape b, FpTransform2 xb) {
  if (a.type == FP2_SHAPE_CIRCLE && b.type == FP2_SHAPE_CIRCLE) {
    return fp2_circle_circle(a.u.circle.radius, xa.p, b.u.circle.radius, xb.p);
  }
  if (a.type == FP2_SHAPE_BOX && b.type == FP2_SHAPE_BOX) {
    return fp2_box_box(a.u.box, xa, b.u.box, xb);
  }
  if (a.type == FP2_SHAPE_CIRCLE && b.type == FP2_SHAPE_BOX) {
    return fp2_circle_box(a.u.circle.radius, xa.p, b.u.box, xb, 1);
  }
  if (a.type == FP2_SHAPE_BOX && b.type == FP2_SHAPE_CIRCLE) {
    return fp2_circle_box(b.u.circle.radius, xb.p, a.u.box, xa, 0);
  }
  return fp2_miss();
}

