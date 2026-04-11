#pragma once

#include <stdint.h>

#include "core/fp_math.h"

typedef struct Fp2World Fp2World;
typedef struct Fp2Shape Fp2Shape;

typedef struct FpParticle2d {
  FpVec2 p;
  FpVec2 v;
  float life;   // seconds remaining
  float radius;
  uint32_t color_rgb;
} FpParticle2d;

typedef struct FpParticles2dDesc {
  int max_particles;
  FpVec2 gravity;
  float linear_drag; // per second
} FpParticles2dDesc;

typedef struct FpParticles2d {
  FpParticles2dDesc desc;
  int count;
  int cap;
  FpParticle2d* items;
} FpParticles2d;

void fp_particles2d_init(FpParticles2d* ps, const FpParticles2dDesc* desc);
void fp_particles2d_destroy(FpParticles2d* ps);
void fp_particles2d_clear(FpParticles2d* ps);

void fp_particles2d_emit(FpParticles2d* ps, FpVec2 p, FpVec2 v, float radius, float life, uint32_t color_rgb);

// Integrate with gravity/drag; bounce against world AABBs (one-way rigid coupling).
void fp_particles2d_step(FpParticles2d* ps, const Fp2World* w, const Fp2Shape* shapes, FpVec2 wind, float dt);
