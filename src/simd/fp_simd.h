#pragma once

#include "core/fp_math.h"

// Optional SIMD helpers. When disabled, these map to scalar.

float fp_simd_dot2(FpVec2 a, FpVec2 b);

