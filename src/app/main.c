#include "platform/win32/fp_win32.h"

#include "app/debug_draw.h"
#include "app/demo_scenes.h"

#include "core/fp_timestep.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct AppState {
  Fp2World world;
  DemoSceneId scene;
  FpFixedTimestep ts;
  DebugDraw2D dbg;
} AppState;

static void app_rebuild_scene(AppState* s) {
  demo_build_scene(&s->world, s->scene);
}

static void app_update(FpWin32Window* w, const FpWin32FrameInput* in, void* user) {
  (void)w;
  AppState* s = (AppState*)user;

  if (in->keys.pressed[VK_ESCAPE]) {
    // request quit by sending WM_CLOSE
    // handled by window proc via DestroyWindow; simplest: do nothing here.
  }
  if (in->keys.pressed[' ']) {
    fp_fixed_toggle_paused(&s->ts);
  }
  if (in->keys.pressed['R']) {
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['1']) {
    s->scene = DEMO_SCENE_STACK;
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['2']) {
    s->scene = DEMO_SCENE_BOUNCE;
    app_rebuild_scene(s);
  }
  if (in->keys.pressed['3']) {
    s->scene = DEMO_SCENE_FRICTION;
    app_rebuild_scene(s);
  }

  int steps = fp_fixed_advance(&s->ts, (double)in->dt_seconds);
  for (int i = 0; i < steps; i++) {
    fp2_world_step(&s->world, (float)s->ts.dt, 10, 4);
  }
}

static void app_render(FpWin32Window* w, void* user) {
  AppState* s = (AppState*)user;
  int bw = 0, bh = 0;
  fp_win32_get_backbuffer_size(w, &bw, &bh);
  dbg2d_draw_world(&s->dbg, w, &s->world, bw, bh);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)pCmdLine;
  (void)nCmdShow;

  FpWin32Window* win = fp_win32_create_window(L"FullPhysicsC - Rigid2D Demo", 1100, 720);
  if (!win) return 1;

  AppState s;
  s.scene = DEMO_SCENE_STACK;
  fp_fixed_init(&s.ts, 1.0 / 60.0, 0.1);
  dbg2d_init(&s.dbg);

  Fp2WorldDesc wd;
  wd.max_bodies = 2048;
  wd.max_contacts = 4096;
  wd.gravity = fp_v2(0.0f, -9.81f);
  fp2_world_init(&s.world, &wd);
  app_rebuild_scene(&s);

  fp_win32_run_loop(win, app_update, app_render, &s);

  fp2_world_destroy(&s.world);
  fp_win32_destroy_window(win);
  return 0;
}

