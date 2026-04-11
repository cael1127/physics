#pragma once

// Umbrella include for consumers linking multiple FullPhysicsC modules.

#include "core/fp_math.h"
#include "core/fp_timestep.h"
#include "core/fp_arena.h"
#include "core/fp_log.h"

#include "rigid2d/fp2_world.h"
#include "rigid2d/fp2_shape.h"

#include "fluid2d/fp_fluid2d.h"
#include "particles2d/fp_particles2d.h"
#include "aero2d/fp_aero2d.h"

#include "runtime/fp_scene_data.h"
#include "runtime/fp_scene.h"

#include "draw/fp_camera2.h"
#include "draw/fp_draw2d.h"
