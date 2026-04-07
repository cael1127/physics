#pragma once

#include "rigid2d/fp2_world.h"

typedef enum DemoSceneId {
  DEMO_SCENE_STACK = 1,
  DEMO_SCENE_BOUNCE = 2,
  DEMO_SCENE_FRICTION = 3
} DemoSceneId;

void demo_build_scene(Fp2World* w, DemoSceneId id);

