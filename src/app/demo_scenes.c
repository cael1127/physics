#include "app/demo_scenes.h"

#include "runtime/fp_scene_data.h"

void demo_build_scene(Fp2World* w, DemoSceneId id) {
  fp_scene_data_build_world(w, (int)id);
}
