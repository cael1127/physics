#pragma once

#include "core/fp_math.h"

typedef struct Fp2World Fp2World;
typedef struct Fp2Shape Fp2Shape;

typedef struct FpAero2dDesc {
  FpVec2 wind;             // world-space wind velocity (m/s)
  float air_density;       // kg/m^2 scale for 2D (dimensionally loose; tuned for feel)
  float drag_coefficient;  // Cd scale per body
  float lift_coefficient;  // small lift from relative flow vs body forward (x-axis) angle
} FpAero2dDesc;

// Apply drag and simplified lift in world frame before rigid velocity integration.
void fp_aero2d_apply_forces(Fp2World* w, const Fp2Shape* shapes, const FpAero2dDesc* d, float dt);
