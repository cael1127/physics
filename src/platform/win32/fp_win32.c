#include "platform/win32/fp_win32.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string.h>


struct FpWin32Window {
  HWND hwnd;
  HDC back_dc;
  HBITMAP back_bmp;
  void* back_bits;
  int back_w;
  int back_h;

  LARGE_INTEGER qpc_freq;
  LARGE_INTEGER last_qpc;

  FpWin32KeyState keys;
  FpWin32KeyState keys_prev;
  int should_quit;
};

static LRESULT CALLBACK fp_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  FpWin32Window* w = (FpWin32Window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

  switch (msg) {
    case WM_CLOSE:
      if (w) w->should_quit = 1;
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
      if (w) {
        unsigned vk = (unsigned)(wparam & 0xFFu);
        w->keys.down[vk] = 1;
      }
    } break;
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      if (w) {
        unsigned vk = (unsigned)(wparam & 0xFFu);
        w->keys.down[vk] = 0;
      }
    } break;
    default: break;
  }

  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static void fp_resize_backbuffer(FpWin32Window* w, int width, int height) {
  if (!w) return;
  if (width <= 0) width = 1;
  if (height <= 0) height = 1;
  if (w->back_bmp && width == w->back_w && height == w->back_h) return;

  if (w->back_bmp) {
    DeleteObject(w->back_bmp);
    w->back_bmp = NULL;
  }

  BITMAPINFO bi;
  ZeroMemory(&bi, sizeof(bi));
  bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
  bi.bmiHeader.biWidth = width;
  bi.bmiHeader.biHeight = -height; // top-down
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 32;
  bi.bmiHeader.biCompression = BI_RGB;

  w->back_bmp = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &w->back_bits, NULL, 0);
  w->back_w = width;
  w->back_h = height;

  SelectObject(w->back_dc, w->back_bmp);
}

FpWin32Window* fp_win32_create_window(const wchar_t* title, int width, int height) {
  WNDCLASSW wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = fp_wndproc;
  wc.hInstance = GetModuleHandleW(NULL);
  wc.lpszClassName = L"FullPhysicsCWindowClass";

  RegisterClassW(&wc);

  DWORD style = WS_OVERLAPPEDWINDOW;
  RECT r = {0, 0, width, height};
  AdjustWindowRect(&r, style, FALSE);

  HWND hwnd = CreateWindowExW(
      0, wc.lpszClassName, title, style,
      CW_USEDEFAULT, CW_USEDEFAULT,
      r.right - r.left, r.bottom - r.top,
      NULL, NULL, wc.hInstance, NULL);

  if (!hwnd) return NULL;

  FpWin32Window* w = (FpWin32Window*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FpWin32Window));
  if (!w) {
    DestroyWindow(hwnd);
    return NULL;
  }

  w->hwnd = hwnd;
  SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)w);

  HDC dc = GetDC(hwnd);
  w->back_dc = CreateCompatibleDC(dc);
  ReleaseDC(hwnd, dc);

  QueryPerformanceFrequency(&w->qpc_freq);
  QueryPerformanceCounter(&w->last_qpc);

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  return w;
}

void fp_win32_destroy_window(FpWin32Window* w) {
  if (!w) return;
  if (w->back_bmp) DeleteObject(w->back_bmp);
  if (w->back_dc) DeleteDC(w->back_dc);
  if (w->hwnd) DestroyWindow(w->hwnd);
  HeapFree(GetProcessHeap(), 0, w);
}

static float fp_win32_frame_dt(FpWin32Window* w) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  LONGLONG delta = now.QuadPart - w->last_qpc.QuadPart;
  w->last_qpc = now;
  return (float)((double)delta / (double)w->qpc_freq.QuadPart);
}

static void fp_win32_poll(FpWin32Window* w) {
  MSG msg;
  while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) w->should_quit = 1;
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

void fp_win32_run_loop(FpWin32Window* w, FpWin32UpdateFn update, FpWin32RenderFn render, void* user) {
  if (!w) return;

  for (;;) {
    fp_win32_poll(w);
    if (w->should_quit) break;

    RECT cr;
    GetClientRect(w->hwnd, &cr);
    int cw = (int)(cr.right - cr.left);
    int ch = (int)(cr.bottom - cr.top);
    fp_resize_backbuffer(w, cw, ch);

    // pressed is computed each frame from prev/current
    for (int i = 0; i < 256; i++) {
      w->keys.pressed[i] = (uint8_t)(w->keys.down[i] && !w->keys_prev.down[i]);
    }
    memcpy(&w->keys_prev, &w->keys, sizeof(w->keys));

    FpWin32FrameInput in;
    in.width = cw;
    in.height = ch;
    in.dt_seconds = fp_win32_frame_dt(w);
    in.keys = w->keys;
    in.should_quit = w->should_quit;

    if (update) update(w, &in, user);
    if (render) render(w, user);

    // Blit backbuffer to window
    HDC dc = GetDC(w->hwnd);
    BitBlt(dc, 0, 0, w->back_w, w->back_h, w->back_dc, 0, 0, SRCCOPY);
    ReleaseDC(w->hwnd, dc);
  }
}

void fp_win32_get_backbuffer_size(FpWin32Window* w, int* out_w, int* out_h) {
  if (out_w) *out_w = w ? w->back_w : 0;
  if (out_h) *out_h = w ? w->back_h : 0;
}

void fp_win32_begin_draw(FpWin32Window* w) { (void)w; }
void fp_win32_end_draw(FpWin32Window* w) { (void)w; }

static uint32_t fp_rgb_to_bgr(uint32_t rgb) {
  uint32_t r = (rgb >> 16) & 0xFFu;
  uint32_t g = (rgb >> 8) & 0xFFu;
  uint32_t b = (rgb >> 0) & 0xFFu;
  return (b << 16) | (g << 8) | (r << 0);
}

void fp_win32_clear(FpWin32Window* w, uint32_t rgb) {
  if (!w || !w->back_bits) return;
  uint32_t bgr = fp_rgb_to_bgr(rgb);
  uint32_t* p = (uint32_t*)w->back_bits;
  size_t n = (size_t)w->back_w * (size_t)w->back_h;
  for (size_t i = 0; i < n; i++) p[i] = bgr;
}

void fp_win32_line(FpWin32Window* w, int x0, int y0, int x1, int y1, uint32_t rgb) {
  if (!w) return;
  HPEN pen = CreatePen(PS_SOLID, 1, (COLORREF)fp_rgb_to_bgr(rgb));
  HGDIOBJ old = SelectObject(w->back_dc, pen);
  MoveToEx(w->back_dc, x0, y0, NULL);
  LineTo(w->back_dc, x1, y1);
  SelectObject(w->back_dc, old);
  DeleteObject(pen);
}

void fp_win32_rect(FpWin32Window* w, int x0, int y0, int x1, int y1, uint32_t rgb) {
  if (!w) return;
  HPEN pen = CreatePen(PS_SOLID, 1, (COLORREF)fp_rgb_to_bgr(rgb));
  HGDIOBJ old_pen = SelectObject(w->back_dc, pen);
  HGDIOBJ old_brush = SelectObject(w->back_dc, GetStockObject(NULL_BRUSH));
  Rectangle(w->back_dc, x0, y0, x1, y1);
  SelectObject(w->back_dc, old_brush);
  SelectObject(w->back_dc, old_pen);
  DeleteObject(pen);
}

static COLORREF fp_rgb_to_colorref(uint32_t rgb) {
  uint32_t r = (rgb >> 16) & 0xFFu;
  uint32_t g = (rgb >> 8) & 0xFFu;
  uint32_t b = (rgb >> 0) & 0xFFu;
  return RGB((BYTE)r, (BYTE)g, (BYTE)b);
}

void fp_win32_fill_triangle(FpWin32Window* w, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t rgb) {
  if (!w) return;
  POINT pt[3];
  pt[0].x = x0;
  pt[0].y = y0;
  pt[1].x = x1;
  pt[1].y = y1;
  pt[2].x = x2;
  pt[2].y = y2;
  HBRUSH br = CreateSolidBrush(fp_rgb_to_colorref(rgb));
  HGDIOBJ old_br = SelectObject(w->back_dc, br);
  HPEN pen = CreatePen(PS_SOLID, 1, fp_rgb_to_colorref(rgb));
  HGDIOBJ old_pen = SelectObject(w->back_dc, pen);
  Polygon(w->back_dc, pt, 3);
  SelectObject(w->back_dc, old_pen);
  DeleteObject(pen);
  SelectObject(w->back_dc, old_br);
  DeleteObject(br);
}

void fp_win32_fill_quad(
    FpWin32Window* w, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t rgb) {
  if (!w) return;
  POINT pt[4];
  pt[0].x = x0;
  pt[0].y = y0;
  pt[1].x = x1;
  pt[1].y = y1;
  pt[2].x = x2;
  pt[2].y = y2;
  pt[3].x = x3;
  pt[3].y = y3;
  HBRUSH br = CreateSolidBrush(fp_rgb_to_colorref(rgb));
  HGDIOBJ old_br = SelectObject(w->back_dc, br);
  HPEN pen = CreatePen(PS_SOLID, 1, fp_rgb_to_colorref(rgb));
  HGDIOBJ old_pen = SelectObject(w->back_dc, pen);
  Polygon(w->back_dc, pt, 4);
  SelectObject(w->back_dc, old_pen);
  DeleteObject(pen);
  SelectObject(w->back_dc, old_br);
  DeleteObject(br);
}

void fp_win32_text_utf8(FpWin32Window* w, int x, int y, const char* utf8, uint32_t rgb) {
  if (!w || !utf8) return;
  SetBkMode(w->back_dc, TRANSPARENT);
  SetTextColor(w->back_dc, fp_rgb_to_colorref(rgb));
  HGDIOBJ oldf = SelectObject(w->back_dc, GetStockObject(ANSI_FIXED_FONT));
  TextOutA(w->back_dc, x, y, utf8, (int)strlen(utf8));
  SelectObject(w->back_dc, oldf);
}

