#include "app/debug_draw.h"

#include "fluid2d/fp_fluid2d.h"
#include "particles2d/fp_particles2d.h"
#include "runtime/fp_scene.h"
#include "rigid2d/fp2_world.h"

#include <math.h>
#include <stdio.h>

void dbg2d_init(DebugDraw2D* d, FpWin32Window* win) {
  fp_camera2_init_default(&d->cam);
  d->draw.ops = fp_draw2d_win32_ops();
  d->draw.ctx = win;
}

static void dbg_line_world(DebugDraw2D* d, int sw, int sh, FpVec2 a, FpVec2 b, uint32_t rgb) {
  int ax, ay, bx, by;
  fp_camera2_world_to_screen(&d->cam, sw, sh, a, &ax, &ay);
  fp_camera2_world_to_screen(&d->cam, sw, sh, b, &bx, &by);
  fp_draw2d_line(&d->draw, ax, ay, bx, by, rgb);
}

static void dbg_circle_outline(DebugDraw2D* d, int sw, int sh, FpVec2 c, float r, uint32_t rgb) {
  const int seg = 24;
  FpVec2 prev = fp_v2(c.x + r, c.y);
  for (int i = 1; i <= seg; i++) {
    float t = (float)i * (2.0f * 3.14159265f / (float)seg);
    FpVec2 cur = fp_v2(c.x + r * cosf(t), c.y + r * sinf(t));
    dbg_line_world(d, sw, sh, prev, cur, rgb);
    prev = cur;
  }
}

static void dbg_circle_fill(DebugDraw2D* d, int sw, int sh, FpVec2 c, float r, uint32_t rgb) {
  int cx, cy;
  fp_camera2_world_to_screen(&d->cam, sw, sh, c, &cx, &cy);
  int pr = (int)lrintf(r * d->cam.pixels_per_meter * d->cam.zoom);
  if (pr < 2) pr = 2;
  int prevx = cx + pr, prevy = cy;
  const int seg = 16;
  for (int i = 1; i <= seg; i++) {
    float t = (float)i * (2.0f * 3.14159265f / (float)seg);
    int px = cx + (int)lrintf(cosf(t) * (float)pr);
    int py = cy + (int)lrintf(sinf(t) * (float)pr);
    fp_draw2d_tri_fill(&d->draw, cx, cy, prevx, prevy, px, py, rgb);
    prevx = px;
    prevy = py;
  }
}

static void dbg_box_fill(DebugDraw2D* d, int sw, int sh, FpTransform2 xf, FpVec2 he, uint32_t rgb) {
  FpVec2 v0 = fp_xf2_mul(xf, fp_v2(-he.x, -he.y));
  FpVec2 v1 = fp_xf2_mul(xf, fp_v2(he.x, -he.y));
  FpVec2 v2 = fp_xf2_mul(xf, fp_v2(he.x, he.y));
  FpVec2 v3 = fp_xf2_mul(xf, fp_v2(-he.x, he.y));
  int x0, y0, x1, y1, x2, y2, x3, y3;
  fp_camera2_world_to_screen(&d->cam, sw, sh, v0, &x0, &y0);
  fp_camera2_world_to_screen(&d->cam, sw, sh, v1, &x1, &y1);
  fp_camera2_world_to_screen(&d->cam, sw, sh, v2, &x2, &y2);
  fp_camera2_world_to_screen(&d->cam, sw, sh, v3, &x3, &y3);
  fp_draw2d_quad_fill(&d->draw, x0, y0, x1, y1, x2, y2, x3, y3, rgb);
}

static void dbg_box_outline(DebugDraw2D* d, int sw, int sh, FpTransform2 xf, FpVec2 he, uint32_t rgb) {
  FpVec2 v0 = fp_xf2_mul(xf, fp_v2(-he.x, -he.y));
  FpVec2 v1 = fp_xf2_mul(xf, fp_v2(he.x, -he.y));
  FpVec2 v2 = fp_xf2_mul(xf, fp_v2(he.x, he.y));
  FpVec2 v3 = fp_xf2_mul(xf, fp_v2(-he.x, he.y));
  dbg_line_world(d, sw, sh, v0, v1, rgb);
  dbg_line_world(d, sw, sh, v1, v2, rgb);
  dbg_line_world(d, sw, sh, v2, v3, rgb);
  dbg_line_world(d, sw, sh, v3, v0, rgb);
}

static uint32_t dbg_heat(float t) {
  t = t > 1.0f ? 1.0f : t;
  uint8_t r = (uint8_t)(40.0f + t * 200.0f);
  uint8_t g = (uint8_t)(30.0f + (1.0f - t) * 180.0f);
  uint8_t b = (uint8_t)(60.0f + (1.0f - t) * 160.0f);
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void dbg2d_draw_scene(
    DebugDraw2D* d,
    FpScene* scene,
    int width,
    int height,
    float fps,
    const char* scene_label) {
  if (!d || !scene) return;

  fp_draw2d_clear(&d->draw, 0x101418);

  for (int gx = -20; gx <= 20; gx++) {
    FpVec2 a = fp_v2((float)gx, -10.0f);
    FpVec2 b = fp_v2((float)gx, 10.0f);
    dbg_line_world(d, width, height, a, b, 0x1f2a33);
  }
  for (int gy = -10; gy <= 10; gy++) {
    FpVec2 a = fp_v2(-20.0f, (float)gy);
    FpVec2 b = fp_v2(20.0f, (float)gy);
    dbg_line_world(d, width, height, a, b, 0x1f2a33);
  }

  if (scene->params.show_wind_overlay) {
    FpVec2 wv = scene->params.wind;
    float len = fp_v2_len(wv);
    if (len > 0.05f) {
      FpVec2 dir = fp_v2_mul(wv, 1.0f / len);
      for (int gy = -8; gy <= 8; gy += 2) {
        for (int gx = -15; gx <= 15; gx += 3) {
          FpVec2 p = fp_v2((float)gx, (float)gy);
          FpVec2 q = fp_v2_mad(p, 0.35f * len, dir);
          dbg_line_world(d, width, height, p, q, 0x4466aa);
        }
      }
    }
  }

  if (scene->fluid_inited && scene->params.show_fluid) {
    const FpFluid2d* f = &scene->fluid;
    float h = f->desc.cell_size;
    for (int j = 0; j < f->ny; j++) {
      for (int i = 0; i < f->nx; i++) {
        int idx = fp_fluid2d_index(f, i, j);
        float dye = f->dye[idx];
        if (dye < 0.002f) continue;
        FpVec2 c0 = fp_v2(f->desc.origin.x + (float)i * h, f->desc.origin.y + (float)j * h);
        FpVec2 c1 = fp_v2(c0.x + h, c0.y);
        FpVec2 c2 = fp_v2(c0.x + h, c0.y + h);
        FpVec2 c3 = fp_v2(c0.x, c0.y + h);
        uint32_t col = dbg_heat(dye);
        int x0, y0, x1, y1, x2, y2, x3, y3;
        fp_camera2_world_to_screen(&d->cam, width, height, c0, &x0, &y0);
        fp_camera2_world_to_screen(&d->cam, width, height, c1, &x1, &y1);
        fp_camera2_world_to_screen(&d->cam, width, height, c2, &x2, &y2);
        fp_camera2_world_to_screen(&d->cam, width, height, c3, &x3, &y3);
        fp_draw2d_quad_fill(&d->draw, x0, y0, x1, y1, x2, y2, x3, y3, col);
      }
    }
  }

  const Fp2World* world = &scene->world;
  for (int i = 0; i < world->body_count; i++) {
    const Fp2Body* b = &world->bodies[i];
    FpTransform2 xf;
    xf.p = b->p;
    xf.R = fp_m22_rot(b->angle);

    uint32_t fill = b->is_static ? 0x2a3d4a : 0x5a9eb5;
    uint32_t line = b->is_static ? 0x4a6070 : 0xa0d8e8;
    const Fp2Shape* s = &world->shapes[i];
    if (s->type == FP2_SHAPE_CIRCLE) {
      dbg_circle_fill(d, width, height, b->p, s->u.circle.radius, fill);
      dbg_circle_outline(d, width, height, b->p, s->u.circle.radius, line);
    } else if (s->type == FP2_SHAPE_BOX) {
      dbg_box_fill(d, width, height, xf, s->u.box.half_extents, fill);
      dbg_box_outline(d, width, height, xf, s->u.box.half_extents, line);
    }
  }

  if (scene->params.show_contacts) {
    for (int i = 0; i < world->contact_count; i++) {
      const Fp2Contact* c = &world->contacts[i];
      if (!c->m.hit) continue;
      FpVec2 n = c->m.normal;
      int cp_count = c->m.cp_count > 2 ? 2 : c->m.cp_count;
      for (int cp = 0; cp < cp_count; cp++) {
        FpVec2 p = c->m.cps[cp].p;
        dbg_line_world(d, width, height, p, fp_v2_mad(p, 0.25f, n), 0xffc857);
      }
    }
  }

  if (scene->particles_inited) {
    FpParticles2d* ps = &scene->particles;
    for (int i = 0; i < ps->count; i++) {
      const FpParticle2d* o = &ps->items[i];
      dbg_circle_fill(d, width, height, o->p, o->radius, o->color_rgb);
    }
  }

  char buf[512];
  snprintf(
      buf,
      sizeof(buf),
      "FullPhysicsC | %s | FPS %.0f\n"
      "wind (%.2f, %.2f) | grav x%.2f | fluid %s | contacts %s | wind viz %s\n"
      "[Space] pause  [R] reset  [1-3] scene  [F] fluid  [C] contacts  [W] wind  [+/-] grav\n"
      "[D] dye  [P] particles",
      scene_label ? scene_label : "?",
      (double)fps,
      (double)scene->params.wind.x,
      (double)scene->params.wind.y,
      (double)scene->params.gravity_scale,
      scene->params.show_fluid ? "on" : "off",
      scene->params.show_contacts ? "on" : "off",
      scene->params.show_wind_overlay ? "on" : "off");
  fp_draw2d_text(&d->draw, 10, 12, buf, 0xdddddd);
}
