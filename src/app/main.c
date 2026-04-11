#include "platform/win32/fp_win32.h"

#include "app/debug_draw.h"
#include "app/demo_scenes.h"
#include "particles2d/fp_particles2d.h"
#include "runtime/fp_scene.h"

#include "core/fp_math.h"
#include "core/fp_timestep.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <math.h>
#include <stdlib.h>

typedef struct AppState {
  FpScene scene;
  DemoSceneId demo;
  FpFixedTimestep ts;
  DebugDraw2D dbg;
  float fps_ema;
} AppState;

static void app_rebuild_scene(AppState* s) {
  fp_scene_load_demo(&s->scene, (int)s->demo);
}

static void app_update(FpWin32Window* w, const FpWin32FrameInput* in, void* user) {
  AppState* s = (AppState*)user;

  float inst_fps = in->dt_seconds > 1e-6f ? (1.0f / in->dt_seconds) : 0.0f;
  s->fps_ema = s->fps_ema <= 0.0f ? inst_fps : (s->fps_ema * 0.92f + inst_fps * 0.08f);

  if (in->keys.pressed[VK_ESCAPE]) {
    PostQuitMessage(0);
  }
  if (in->keys.pressed[' ']) {
    fp_fixed_toggle_paused(&s->ts);
  }
  if (in->keys.pressed['R']) {
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['1']) {
    s->demo = DEMO_SCENE_STACK;
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['2']) {
    s->demo = DEMO_SCENE_BOUNCE;
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['3']) {
    s->demo = DEMO_SCENE_FRICTION;
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['F']) {
    s->scene.params.show_fluid = !s->scene.params.show_fluid;
  }
  if (in->keys.pressed['C']) {
    s->scene.params.show_contacts = !s->scene.params.show_contacts;
  }
  if (in->keys.pressed['W']) {
    s->scene.params.show_wind_overlay = !s->scene.params.show_wind_overlay;
  }
  if (in->keys.pressed['O']) {
    s->scene.params.gravity_scale -= 0.05f;
    if (s->scene.params.gravity_scale < 0.05f) s->scene.params.gravity_scale = 0.05f;
  }
  if (in->keys.pressed['L']) {
    s->scene.params.gravity_scale += 0.05f;
    if (s->scene.params.gravity_scale > 3.0f) s->scene.params.gravity_scale = 3.0f;
  }
  if (in->keys.pressed['Q']) {
    s->scene.params.wind.x -= 0.25f;
  }
  if (in->keys.pressed['E']) {
    s->scene.params.wind.x += 0.25f;
  }
  if (in->keys.pressed['D']) {
    fp_scene_add_dye_at_world(&s->scene, fp_v2(0.0f, 0.5f), 1.2f);
  }
  if (in->keys.pressed['P']) {
    if (s->scene.particles_inited) {
      for (int k = 0; k < 8; k++) {
        float a = (float)k * (6.2831853f / 8.0f);
        fp_particles2d_emit(
            &s->scene.particles,
            fp_v2(-1.0f + 0.2f * (float)k, 3.0f),
            fp_v2(0.4f * cosf(a), 0.6f + 0.4f * sinf(a)),
            0.08f,
            2.5f,
            0xff8866);
      }
    }
  }

  int steps = fp_fixed_advance(&s->ts, (double)in->dt_seconds);
  for (int i = 0; i < steps; i++) {
    fp_scene_step(&s->scene, (float)s->ts.dt, 10, 4);
  }

  if (s->scene.fluid_inited && in->keys.down['H']) {
    fp_scene_add_dye_at_world(&s->scene, fp_v2(1.2f * sinf((float)in->dt_seconds * 60.0f), 1.0f), 0.08f * s->scene.params.fluid_emit_rate);
  }

  (void)w;
}

static const char* app_scene_label(DemoSceneId d) {
  switch (d) {
    case DEMO_SCENE_STACK: return "stack";
    case DEMO_SCENE_BOUNCE: return "bounce";
    case DEMO_SCENE_FRICTION: return "friction";
    default: return "?";
  }
}

static void app_render(FpWin32Window* w, void* user) {
  AppState* s = (AppState*)user;
  int bw = 0, bh = 0;
  fp_win32_get_backbuffer_size(w, &bw, &bh);
  dbg2d_draw_scene(&s->dbg, &s->scene, bw, bh, s->fps_ema, app_scene_label(s->demo));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)pCmdLine;
  (void)nCmdShow;

  FpWin32Window* win = fp_win32_create_window(L"FullPhysicsC - Multi-physics demo", 1100, 720);
  if (!win) return 1;

  AppState s;
  memset(&s, 0, sizeof(s));
  s.demo = DEMO_SCENE_STACK;
  fp_fixed_init(&s.ts, 1.0 / 60.0, 0.1);
  dbg2d_init(&s.dbg, win);

  FpSceneDesc desc;
  fp_scene_desc_default(&desc);
  fp_scene_init(&s.scene, &desc);
  app_rebuild_scene(&s);

  fp_win32_run_loop(win, app_update, app_render, &s);

  fp_scene_destroy(&s.scene);
  fp_win32_destroy_window(win);
  return 0;
}
