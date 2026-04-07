#include "simd/fp_simd.h"

#include <math.h>
#include <stdio.h>

static int nearly(float a, float b, float eps) { return fabsf(a - b) <= eps; }

int test_simd(void) {
  // Even if SIMD is disabled, this still exercises the wrapper.
  float worst = 0.0f;
  for (int i = -50; i <= 50; i++) {
    for (int j = -50; j <= 50; j++) {
      FpVec2 a = fp_v2((float)i * 0.17f, (float)j * -0.23f);
      FpVec2 b = fp_v2((float)j * 0.11f, (float)i * 0.07f);
      float ref = fp_v2_dot(a, b);
      float got = fp_simd_dot2(a, b);
      float err = fabsf(ref - got);
      if (err > worst) worst = err;
      if (!nearly(ref, got, 1e-6f)) {
        fprintf(stderr, "FAIL: simd dot mismatch ref=%g got=%g\n", ref, got);
        return 1;
      }
    }
  }
  printf("test_simd OK (worst_abs_err=%g)\n", worst);
  return 0;
}

