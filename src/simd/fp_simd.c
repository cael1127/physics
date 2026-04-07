#include "simd/fp_simd.h"

#if FP_SIMD && (defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__))
  #include <emmintrin.h>
#endif

float fp_simd_dot2(FpVec2 a, FpVec2 b) {
#if FP_SIMD && (defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__))
  __m128 va = _mm_set_ps(0.0f, 0.0f, a.y, a.x);
  __m128 vb = _mm_set_ps(0.0f, 0.0f, b.y, b.x);
  __m128 mul = _mm_mul_ps(va, vb);
  __m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
  __m128 sum = _mm_add_ss(mul, shuf);
  return _mm_cvtss_f32(sum);
#else
  return fp_v2_dot(a, b);
#endif
}

