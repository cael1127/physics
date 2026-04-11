#pragma once

#include "runtime/fp_scene_data.h"

typedef FpSceneDemoId DemoSceneId;

#define DEMO_SCENE_STACK FP_SCENE_DEMO_STACK
#define DEMO_SCENE_BOUNCE FP_SCENE_DEMO_BOUNCE
#define DEMO_SCENE_FRICTION FP_SCENE_DEMO_FRICTION

#include "rigid2d/fp2_world.h"

void demo_build_scene(Fp2World* w, DemoSceneId id);
