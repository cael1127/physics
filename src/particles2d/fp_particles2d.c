#include "particles2d/fp_particles2d.h"

#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include "core/fp_assert.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

void fp_particles2d_init(FpParticles2d* ps, const FpParticles2dDesc* desc) {
  memset(ps, 0, sizeof(*ps));
  ps->desc = *desc;
  ps->cap = desc->max_particles > 0 ? desc->max_particles : 256;
  ps->items = (FpParticle2d*)calloc((size_t)ps->cap, sizeof(FpParticle2d));
}

void fp_particles2d_destroy(FpParticles2d* ps) {
  if (!ps) return;
  free(ps->items);
  memset(ps, 0, sizeof(*ps));
}

void fp_particles2d_clear(FpParticles2d* ps) {
  if (!ps) return;
  ps->count = 0;
}

void fp_particles2d_emit(FpParticles2d* ps, FpVec2 p, FpVec2 v, float radius, float life, uint32_t color_rgb) {
  if (!ps || ps->count >= ps->cap || life <= 0.0f) return;
  int idx = ps->count++;
  FpParticle2d* o = &ps->items[idx];
  o->p = p;
  o->v = v;
  o->life = life;
  o->radius = radius > 1e-4f ? radius : 0.05f;
  o->color_rgb = color_rgb;
}

static int fp_particle_circle_vs_aabb(FpVec2 p, float r, Fp2Aabb box) {
  float cx = p.x < box.min.x ? box.min.x : (p.x > box.max.x ? box.max.x : p.x);
  float cy = p.y < box.min.y ? box.min.y : (p.y > box.max.y ? box.max.y : p.y);
  float dx = p.x - cx;
  float dy = p.y - cy;
  return (dx * dx + dy * dy) < r * r;
}

static void fp_particle_resolve_aabb(FpParticle2d* o, Fp2Aabb box) {
  if (!fp_particle_circle_vs_aabb(o->p, o->radius, box)) return;
  float cx = o->p.x < box.min.x ? box.min.x : (o->p.x > box.max.x ? box.max.x : o->p.x);
  float cy = o->p.y < box.min.y ? box.min.y : (o->p.y > box.max.y ? box.max.y : o->p.y);
  float dx = o->p.x - cx;
  float dy = o->p.y - cy;
  float len = sqrtf(dx * dx + dy * dy);
  if (len < 1e-6f) {
    float sx = (box.min.x + box.max.x) * 0.5f;
    o->p.x = sx > o->p.x ? box.min.x - o->radius : box.max.x + o->radius;
    o->v.x *= -0.3f;
    return;
  }
  dx /= len;
  dy /= len;
  float pen = o->radius - len + 1e-4f;
  o->p.x += dx * pen;
  o->p.y += dy * pen;
  float vn = o->v.x * dx + o->v.y * dy;
  if (vn < 0.0f) {
    o->v.x -= 1.6f * vn * dx;
    o->v.y -= 1.6f * vn * dy;
  }
}

void fp_particles2d_step(FpParticles2d* ps, const Fp2World* w, const Fp2Shape* shapes, FpVec2 wind, float dt) {
  if (!ps || !w || !shapes || dt <= 0.0f) return;
  float drag = ps->desc.linear_drag * dt;
  if (drag > 0.99f) drag = 0.99f;

  int wn = 0;
  for (int i = 0; i < ps->count; i++) {
    FpParticle2d* o = &ps->items[i];
    o->life -= dt;
    if (o->life <= 0.0f) continue;

    FpVec2 acc = ps->desc.gravity;
    o->v = fp_v2_mad(o->v, dt, acc);
    o->v = fp_v2_mad(o->v, dt, wind);
    o->v = fp_v2_mul(o->v, 1.0f - drag);

    o->p = fp_v2_mad(o->p, dt, o->v);

    for (int b = 0; b < w->body_count; b++) {
      FpTransform2 xf;
      xf.p = w->bodies[b].p;
      xf.R = fp_m22_rot(w->bodies[b].angle);
      Fp2Aabb box = fp2_aabb_from_shape(shapes[b], xf);
      fp_particle_resolve_aabb(o, box);
    }

    ps->items[wn++] = *o;
  }
  ps->count = wn;
}
