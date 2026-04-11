#include "fluid2d/fp_fluid2d.h"

#include <math.h>
#include <stdio.h>

static float test_fluid_sum_dye(const FpFluid2d* fl) {
  float s = 0.0f;
  for (int j = 0; j < fl->ny; j++) {
    for (int i = 0; i < fl->nx; i++) {
      s += fl->dye[fp_fluid2d_index(fl, i, j)];
    }
  }
  return s;
}

int test_fluid(void) {
  FpFluid2d f;
  FpFluid2dDesc d;
  d.nx = 40;
  d.ny = 32;
  d.cell_size = 0.1f;
  d.origin = fp_v2(0.0f, 0.0f);
  d.dye_diffusion = 0.0f;
  d.jacobi_diffuse_iters = 0;

  fp_fluid2d_init(&f, &d);
  fp_fluid2d_reset(&f);

  fp_fluid2d_add_dye_gaussian(&f, fp_v2(2.0f, 1.6f), 0.25f, 1.0f);

  float s0 = test_fluid_sum_dye(&f);
  fp_fluid2d_set_wind(&f, fp_v2(0.0f, 0.0f));
  for (int k = 0; k < 30; k++) {
    fp_fluid2d_step(&f, 1.0f / 60.0f);
  }
  float s1 = test_fluid_sum_dye(&f);
  if (fabsf(s1 - s0) > 1e-2f * (s0 + 1.0f)) {
    fprintf(stderr, "test_fluid: dye sum drifted too much (%.6f vs %.6f)\n", (double)s0, (double)s1);
    fp_fluid2d_destroy(&f);
    return 1;
  }

  fp_fluid2d_reset(&f);
  fp_fluid2d_add_dye_gaussian(&f, fp_v2(1.0f, 1.6f), 0.2f, 1.0f);
  fp_fluid2d_set_wind(&f, fp_v2(3.0f, 0.0f));

  float cx0 = 0.0f, w0 = 0.0f;
  for (int j = 0; j < f.ny; j++) {
    for (int i = 0; i < f.nx; i++) {
      float d0 = f.dye[fp_fluid2d_index(&f, i, j)];
      cx0 += (float)i * d0;
      w0 += d0;
    }
  }
  cx0 /= (w0 + 1e-6f);

  for (int k = 0; k < 40; k++) {
    fp_fluid2d_step(&f, 1.0f / 60.0f);
  }

  float cx1 = 0.0f, w1 = 0.0f;
  for (int j = 0; j < f.ny; j++) {
    for (int i = 0; i < f.nx; i++) {
      float d0 = f.dye[fp_fluid2d_index(&f, i, j)];
      cx1 += (float)i * d0;
      w1 += d0;
    }
  }
  cx1 /= (w1 + 1e-6f);

  if (!(cx1 > cx0 + 0.5f)) {
    fprintf(stderr, "test_fluid: dye COM did not advect with wind (%.3f -> %.3f)\n", (double)cx0, (double)cx1);
    fp_fluid2d_destroy(&f);
    return 1;
  }

  fp_fluid2d_destroy(&f);
  printf("test_fluid OK\n");
  return 0;
}
