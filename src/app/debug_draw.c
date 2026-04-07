#include "app/debug_draw.h"

#include <math.h>

static int dbg2d_sx(const DebugDraw2D* d, int w, float x) {
  float px = (x - d->camera_center.x) * d->pixels_per_meter + 0.5f * (float)w;
  return (int)lrintf(px);
}

static int dbg2d_sy(const DebugDraw2D* d, int h, float y) {
  // world y up; screen y down
  float py = (-(y - d->camera_center.y)) * d->pixels_per_meter + 0.5f * (float)h;
  return (int)lrintf(py);
}

static void dbg2d_line_world(const DebugDraw2D* d, FpWin32Window* w, int sw, int sh, FpVec2 a, FpVec2 b, uint32_t rgb) {
  fp_win32_line(w, dbg2d_sx(d, sw, a.x), dbg2d_sy(d, sh, a.y), dbg2d_sx(d, sw, b.x), dbg2d_sy(d, sh, b.y), rgb);
}

static void dbg2d_circle(const DebugDraw2D* d, FpWin32Window* w, int sw, int sh, FpVec2 c, float r, uint32_t rgb) {
  const int seg = 24;
  FpVec2 prev = fp_v2(c.x + r, c.y);
  for (int i = 1; i <= seg; i++) {
    float t = (float)i * (2.0f * 3.14159265f / (float)seg);
    FpVec2 cur = fp_v2(c.x + r * cosf(t), c.y + r * sinf(t));
    dbg2d_line_world(d, w, sw, sh, prev, cur, rgb);
    prev = cur;
  }
}

static void dbg2d_box(const DebugDraw2D* d, FpWin32Window* w, int sw, int sh, FpTransform2 xf, FpVec2 he, uint32_t rgb) {
  FpVec2 v0 = fp_xf2_mul(xf, fp_v2(-he.x, -he.y));
  FpVec2 v1 = fp_xf2_mul(xf, fp_v2(he.x, -he.y));
  FpVec2 v2 = fp_xf2_mul(xf, fp_v2(he.x, he.y));
  FpVec2 v3 = fp_xf2_mul(xf, fp_v2(-he.x, he.y));
  dbg2d_line_world(d, w, sw, sh, v0, v1, rgb);
  dbg2d_line_world(d, w, sw, sh, v1, v2, rgb);
  dbg2d_line_world(d, w, sw, sh, v2, v3, rgb);
  dbg2d_line_world(d, w, sw, sh, v3, v0, rgb);
}

void dbg2d_init(DebugDraw2D* d) {
  d->pixels_per_meter = 60.0f;
  d->camera_center = fp_v2(0.0f, 0.0f);
}

void dbg2d_draw_world(DebugDraw2D* d, FpWin32Window* w, const Fp2World* world, int width, int height) {
  // background
  fp_win32_clear(w, 0x101418);

  // ground grid
  for (int gx = -20; gx <= 20; gx++) {
    FpVec2 a = fp_v2((float)gx, -10.0f);
    FpVec2 b = fp_v2((float)gx, 10.0f);
    dbg2d_line_world(d, w, width, height, a, b, 0x1f2a33);
  }
  for (int gy = -10; gy <= 10; gy++) {
    FpVec2 a = fp_v2(-20.0f, (float)gy);
    FpVec2 b = fp_v2(20.0f, (float)gy);
    dbg2d_line_world(d, w, width, height, a, b, 0x1f2a33);
  }

  for (int i = 0; i < world->body_count; i++) {
    const Fp2Body* b = &world->bodies[i];
    FpTransform2 xf;
    xf.p = b->p;
    xf.R = fp_m22_rot(b->angle);

    uint32_t col = b->is_static ? 0x3a5566 : 0x88c0d0;
    const Fp2Shape* s = &world->shapes[i];
    if (s->type == FP2_SHAPE_CIRCLE) {
      dbg2d_circle(d, w, width, height, b->p, s->u.circle.radius, col);
    } else if (s->type == FP2_SHAPE_BOX) {
      dbg2d_box(d, w, width, height, xf, s->u.box.half_extents, col);
    }
  }

  // contacts
  for (int i = 0; i < world->contact_count; i++) {
    const Fp2Contact* c = &world->contacts[i];
    if (!c->m.hit) continue;
    FpVec2 p = c->m.cp.p;
    FpVec2 n = c->m.normal;
    dbg2d_line_world(d, w, width, height, p, fp_v2_mad(p, 0.25f, n), 0xffc857);
  }
}

