#include "aero2d/fp_aero2d.h"

#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include <math.h>

static float fp_aero_frontal_area(const Fp2Shape* s) {
  switch (s->type) {
    case FP2_SHAPE_CIRCLE: {
      float r = s->u.circle.radius;
      return 2.0f * r; // characteristic width for 2D strip model
    }
    case FP2_SHAPE_BOX: {
      FpVec2 he = s->u.box.half_extents;
      return 2.0f * (he.x + he.y);
    }
    default: return 1.0f;
  }
}

void fp_aero2d_apply_forces(Fp2World* w, const Fp2Shape* shapes, const FpAero2dDesc* d, float dt) {
  if (!w || !shapes || !d || dt <= 0.0f) return;
  float rho = d->air_density;
  if (rho <= 0.0f) return;

  for (int i = 0; i < w->body_count; i++) {
    Fp2Body* b = &w->bodies[i];
    if (b->is_static) continue;

    FpVec2 vr = fp_v2_sub(d->wind, b->v);
    float speed = fp_v2_len(vr);
    if (speed < 1e-6f) continue;

    FpVec2 drag_dir = fp_v2_mul(vr, 1.0f / speed);
    float A = fp_aero_frontal_area(&shapes[i]);
    float q = 0.5f * rho * speed * speed;
    FpVec2 F = fp_v2_mul(drag_dir, q * A * d->drag_coefficient);

    if (d->lift_coefficient != 0.0f && shapes[i].type == FP2_SHAPE_BOX) {
      float fwdx = cosf(b->angle);
      float fwdy = sinf(b->angle);
      FpVec2 forward = fp_v2(fwdx, fwdy);
      float s = fp_v2_dot(fp_v2_norm(vr), fp_v2_perp(forward));
      float lift_mag = q * A * d->lift_coefficient * s;
      F = fp_v2_add(F, fp_v2_mul(fp_v2_perp(drag_dir), lift_mag));
    }

    fp2_world_apply_body_force(w, i, F, 0.0f, dt);
  }
}
