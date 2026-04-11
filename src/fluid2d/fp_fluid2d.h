#pragma once

#include <stdint.h>

#include "core/fp_math.h"

typedef struct Fp2World Fp2World;

typedef struct FpFluid2dDesc {
  int nx;
  int ny;
  float cell_size;   // world meters per cell
  FpVec2 origin;     // world position of grid corner (cell 0,0 center offset by half cell if needed)
  float dye_diffusion;
  int jacobi_diffuse_iters;
} FpFluid2dDesc;

typedef struct FpFluid2d {
  FpFluid2dDesc desc;
  int nx;
  int ny;
  float* u;       // velocity x at cell center, nx*ny
  float* v;
  float* dye;
  float* dye_tmp;
  uint8_t* solid; // 1 = blocked (rigid obstacle)
} FpFluid2d;

void fp_fluid2d_init(FpFluid2d* f, const FpFluid2dDesc* desc);
void fp_fluid2d_destroy(FpFluid2d* f);
void fp_fluid2d_reset(FpFluid2d* f);

// Set velocity from global wind; zero inside solid mask.
void fp_fluid2d_set_wind(FpFluid2d* f, FpVec2 wind);

// Rasterize dynamic + static bodies into solid mask (world AABB overlap).
void fp_fluid2d_sync_solids_from_world(FpFluid2d* f, const Fp2World* w);

// Dye advection + optional diffusion. dt in seconds.
void fp_fluid2d_step(FpFluid2d* f, float dt);

// Add dye at world position (e.g. pointer / emitter).
void fp_fluid2d_add_dye_gaussian(FpFluid2d* f, FpVec2 world_pos, float radius_world, float amount);

int fp_fluid2d_index(const FpFluid2d* f, int ix, int iy);
FpVec2 fp_fluid2d_cell_center_world(const FpFluid2d* f, int ix, int iy);
void fp_fluid2d_world_to_cell(const FpFluid2d* f, FpVec2 world, float* out_ix, float* out_iy);
