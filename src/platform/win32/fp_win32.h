#pragma once

#include <stdint.h>

typedef struct FpWin32Window FpWin32Window;

typedef struct FpWin32KeyState {
  // virtual-key indexed (0..255)
  uint8_t down[256];
  uint8_t pressed[256]; // edge: went down this frame
} FpWin32KeyState;

typedef struct FpWin32FrameInput {
  int width;
  int height;
  float dt_seconds;
  FpWin32KeyState keys;
  int should_quit;
} FpWin32FrameInput;

typedef void (*FpWin32UpdateFn)(FpWin32Window* w, const FpWin32FrameInput* in, void* user);
typedef void (*FpWin32RenderFn)(FpWin32Window* w, void* user);

FpWin32Window* fp_win32_create_window(const wchar_t* title, int width, int height);
void fp_win32_destroy_window(FpWin32Window* w);

void fp_win32_run_loop(FpWin32Window* w, FpWin32UpdateFn update, FpWin32RenderFn render, void* user);

void fp_win32_get_backbuffer_size(FpWin32Window* w, int* out_w, int* out_h);

// Drawing (GDI)
void fp_win32_begin_draw(FpWin32Window* w);
void fp_win32_end_draw(FpWin32Window* w);
void fp_win32_clear(FpWin32Window* w, uint32_t rgb);
void fp_win32_line(FpWin32Window* w, int x0, int y0, int x1, int y1, uint32_t rgb);
void fp_win32_rect(FpWin32Window* w, int x0, int y0, int x1, int y1, uint32_t rgb);

