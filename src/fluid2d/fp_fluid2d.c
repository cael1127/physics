#include "fluid2d/fp_fluid2d.h"

#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include "core/fp_assert.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static int fp_fluid_clampi(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static float fp_clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

int fp_fluid2d_index(const FpFluid2d* f, int ix, int iy) {
  FP_ASSERT(ix >= 0 && ix < f->nx && iy >= 0 && iy < f->ny);
  return iy * f->nx + ix;
}

FpVec2 fp_fluid2d_cell_center_world(const FpFluid2d* f, int ix, int iy) {
  float h = f->desc.cell_size;
  float x = f->desc.origin.x + ((float)ix + 0.5f) * h;
  float y = f->desc.origin.y + ((float)iy + 0.5f) * h;
  return fp_v2(x, y);
}

void fp_fluid2d_world_to_cell(const FpFluid2d* f, FpVec2 world, float* out_ix, float* out_iy) {
  float h = f->desc.cell_size;
  float gx = (world.x - f->desc.origin.x) / h - 0.5f;
  float gy = (world.y - f->desc.origin.y) / h - 0.5f;
  if (out_ix) *out_ix = gx;
  if (out_iy) *out_iy = gy;
}

void fp_fluid2d_init(FpFluid2d* f, const FpFluid2dDesc* desc) {
  memset(f, 0, sizeof(*f));
  f->desc = *desc;
  f->nx = desc->nx;
  f->ny = desc->ny;
  FP_ASSERT(f->nx > 2 && f->ny > 2);
  size_t n = (size_t)f->nx * (size_t)f->ny;
  f->u = (float*)calloc(n, sizeof(float));
  f->v = (float*)calloc(n, sizeof(float));
  f->dye = (float*)calloc(n, sizeof(float));
  f->dye_tmp = (float*)calloc(n, sizeof(float));
  f->solid = (uint8_t*)calloc(n, sizeof(uint8_t));
}

void fp_fluid2d_destroy(FpFluid2d* f) {
  if (!f) return;
  free(f->u);
  free(f->v);
  free(f->dye);
  free(f->dye_tmp);
  free(f->solid);
  memset(f, 0, sizeof(*f));
}

void fp_fluid2d_reset(FpFluid2d* f) {
  if (!f) return;
  size_t n = (size_t)f->nx * (size_t)f->ny;
  memset(f->u, 0, n * sizeof(float));
  memset(f->v, 0, n * sizeof(float));
  memset(f->dye, 0, n * sizeof(float));
  memset(f->dye_tmp, 0, n * sizeof(float));
  memset(f->solid, 0, n * sizeof(uint8_t));
}

void fp_fluid2d_set_wind(FpFluid2d* f, FpVec2 wind) {
  size_t n = (size_t)f->nx * (size_t)f->ny;
  for (size_t i = 0; i < n; i++) {
    if (f->solid[i]) {
      f->u[i] = 0.0f;
      f->v[i] = 0.0f;
    } else {
      f->u[i] = wind.x;
      f->v[i] = wind.y;
    }
  }
}

static float fp_fluid_sample_bilinear(const float* field, int nx, int ny, float gx, float gy) {
  gx = fp_clampf(gx, 0.0f, (float)nx - 1.001f);
  gy = fp_clampf(gy, 0.0f, (float)ny - 1.001f);
  int i0 = (int)floorf(gx);
  int j0 = (int)floorf(gy);
  int i1 = fp_fluid_clampi(i0 + 1, 0, nx - 1);
  int j1 = fp_fluid_clampi(j0 + 1, 0, ny - 1);
  i0 = fp_fluid_clampi(i0, 0, nx - 1);
  j0 = fp_fluid_clampi(j0, 0, ny - 1);
  float tx = gx - (float)i0;
  float ty = gy - (float)j0;
  float f00 = field[j0 * nx + i0];
  float f10 = field[j0 * nx + i1];
  float f01 = field[j1 * nx + i0];
  float f11 = field[j1 * nx + i1];
  float a = f00 * (1.0f - tx) + f10 * tx;
  float b = f01 * (1.0f - tx) + f11 * tx;
  return a * (1.0f - ty) + b * ty;
}

void fp_fluid2d_add_dye_gaussian(FpFluid2d* f, FpVec2 world_pos, float radius_world, float amount) {
  if (radius_world <= 0.0f || amount == 0.0f) return;
  float r2 = radius_world * radius_world;
  for (int j = 0; j < f->ny; j++) {
    for (int i = 0; i < f->nx; i++) {
      FpVec2 c = fp_fluid2d_cell_center_world(f, i, j);
      FpVec2 d = fp_v2_sub(c, world_pos);
      float d2 = fp_v2_dot(d, d);
      if (d2 > r2 * 4.0f) continue;
      float w = expf(-d2 / (2.0f * r2));
      int idx = fp_fluid2d_index(f, i, j);
      if (!f->solid[idx]) f->dye[idx] += amount * w;
    }
  }
}

void fp_fluid2d_sync_solids_from_world(FpFluid2d* f, const Fp2World* w) {
  if (!f || !w) return;
  memset(f->solid, 0, (size_t)f->nx * (size_t)f->ny);
  for (int j = 0; j < f->ny; j++) {
    for (int i = 0; i < f->nx; i++) {
      FpVec2 c = fp_fluid2d_cell_center_world(f, i, j);
      for (int b = 0; b < w->body_count; b++) {
        FpTransform2 xf;
        xf.p = w->bodies[b].p;
        xf.R = fp_m22_rot(w->bodies[b].angle);
        Fp2Aabb box = fp2_aabb_from_shape(w->shapes[b], xf);
        if (c.x >= box.min.x && c.x <= box.max.x && c.y >= box.min.y && c.y <= box.max.y) {
          f->solid[fp_fluid2d_index(f, i, j)] = 1;
          break;
        }
      }
    }
  }
}

static void fp_fluid_diffuse_dye(FpFluid2d* f) {
  int iters = f->desc.jacobi_diffuse_iters;
  if (iters <= 0 || f->desc.dye_diffusion <= 0.0f) return;
  float a = f->desc.dye_diffusion;
  float c = 1.0f + 4.0f * a;
  int nx = f->nx, ny = f->ny;
  for (int iter = 0; iter < iters; iter++) {
    memcpy(f->dye_tmp, f->dye, (size_t)nx * (size_t)ny * sizeof(float));
    for (int j = 1; j < ny - 1; j++) {
      for (int i = 1; i < nx - 1; i++) {
        int idx = j * nx + i;
        if (f->solid[idx]) {
          f->dye[idx] = 0.0f;
          continue;
        }
        float sum = 0.0f;
        sum += f->dye_tmp[idx - 1];
        sum += f->dye_tmp[idx + 1];
        sum += f->dye_tmp[idx - nx];
        sum += f->dye_tmp[idx + nx];
        f->dye[idx] = (f->dye_tmp[idx] + a * sum) / c;
      }
    }
  }
}

void fp_fluid2d_step(FpFluid2d* f, float dt) {
  if (!f || dt <= 0.0f) return;
  int nx = f->nx, ny = f->ny;
  float h = f->desc.cell_size;

  fp_fluid_diffuse_dye(f);

  memcpy(f->dye_tmp, f->dye, (size_t)nx * (size_t)ny * sizeof(float));

  for (int j = 1; j < ny - 1; j++) {
    for (int i = 1; i < nx - 1; i++) {
      int idx = j * nx + i;
      if (f->solid[idx]) {
        f->dye[idx] = 0.0f;
        continue;
      }
      float gx = (float)i;
      float gy = (float)j;
      float u = f->u[idx];
      float v = f->v[idx];
      float src_x = gx - (u * dt) / h;
      float src_y = gy - (v * dt) / h;
      float nv = fp_fluid_sample_bilinear(f->dye_tmp, nx, ny, src_x, src_y);
      f->dye[idx] = nv;
    }
  }

  for (int j = 0; j < ny; j++) {
    for (int i = 0; i < nx; i++) {
      int idx = j * nx + i;
      if (f->solid[idx]) f->dye[idx] = 0.0f;
    }
  }
}
