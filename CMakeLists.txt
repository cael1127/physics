cmake_minimum_required(VERSION 3.20)

project(FullPhysicsC LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(FP_SIMD "Enable optional SIMD intrinsics" ON)
option(FP_DETERMINISTIC "Enable strict floating-point settings for reproducible runs" ON)

add_library(fullphysics_core STATIC
  src/core/fp_assert.h
  src/core/fp_math.h
  src/core/fp_math.c
  src/core/fp_time.h
  src/core/fp_time.c
  src/core/fp_time_win32.c
  src/core/fp_arena.h
  src/core/fp_arena.c
  src/core/fp_log.h
  src/core/fp_log.c
  src/core/fp_timestep.h
  src/core/fp_timestep.c
)
target_include_directories(fullphysics_core PUBLIC src)
target_compile_definitions(fullphysics_core PRIVATE _CRT_SECURE_NO_WARNINGS)

add_library(fullphysics_rigid2d STATIC
  src/rigid2d/fp2_world.h
  src/rigid2d/fp2_world.c
  src/rigid2d/fp2_shape.h
  src/rigid2d/fp2_shape.c
  src/rigid2d/fp2_collision.h
  src/rigid2d/fp2_collision.c
  src/rigid2d/fp2_broadphase.h
  src/rigid2d/fp2_broadphase.c
  src/rigid2d/fp2_solver.h
  src/rigid2d/fp2_solver.c
)
target_include_directories(fullphysics_rigid2d PUBLIC src)
target_link_libraries(fullphysics_rigid2d PUBLIC fullphysics_core)

if(WIN32)
  add_library(fullphysics_platform_win32 STATIC
    src/platform/win32/fp_win32.h
    src/platform/win32/fp_win32.c
  )
  target_include_directories(fullphysics_platform_win32 PUBLIC src)
  target_compile_definitions(fullphysics_platform_win32 PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

add_library(fullphysics_simd STATIC
  src/simd/fp_simd.h
  src/simd/fp_simd.c
)
target_include_directories(fullphysics_simd PUBLIC src)
target_link_libraries(fullphysics_simd PUBLIC fullphysics_core)
if(FP_SIMD)
  target_compile_definitions(fullphysics_simd PUBLIC FP_SIMD=1)
else()
  target_compile_definitions(fullphysics_simd PUBLIC FP_SIMD=0)
endif()

add_library(fullphysics_fluid2d STATIC
  src/fluid2d/fp_fluid2d.h
  src/fluid2d/fp_fluid2d.c
)
target_include_directories(fullphysics_fluid2d PUBLIC src)
target_link_libraries(fullphysics_fluid2d PUBLIC fullphysics_core fullphysics_rigid2d)

add_library(fullphysics_particles2d STATIC
  src/particles2d/fp_particles2d.h
  src/particles2d/fp_particles2d.c
)
target_include_directories(fullphysics_particles2d PUBLIC src)
target_link_libraries(fullphysics_particles2d PUBLIC fullphysics_core fullphysics_rigid2d)

add_library(fullphysics_aero2d STATIC
  src/aero2d/fp_aero2d.h
  src/aero2d/fp_aero2d.c
)
target_include_directories(fullphysics_aero2d PUBLIC src)
target_link_libraries(fullphysics_aero2d PUBLIC fullphysics_core fullphysics_rigid2d)

if(WIN32)
  add_library(fullphysics_draw STATIC
    src/draw/fp_camera2.h
    src/draw/fp_camera2.c
    src/draw/fp_draw2d.h
    src/draw/fp_draw2d.c
    src/draw/fp_draw_win32.c
  )
  target_include_directories(fullphysics_draw PUBLIC src)
  target_link_libraries(fullphysics_draw PUBLIC fullphysics_core fullphysics_platform_win32)
endif()

add_library(fullphysics_runtime STATIC
  src/runtime/fp_scene.h
  src/runtime/fp_scene.c
  src/runtime/fp_scene_data.h
  src/runtime/fp_scene_data.c
)
target_include_directories(fullphysics_runtime PUBLIC src)
target_link_libraries(fullphysics_runtime PUBLIC
  fullphysics_core
  fullphysics_rigid2d
  fullphysics_fluid2d
  fullphysics_particles2d
  fullphysics_aero2d
)

add_executable(fullphysics_headless
  tools/fullphysics_headless.c
)
target_include_directories(fullphysics_headless PRIVATE src)
target_compile_definitions(fullphysics_headless PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(fullphysics_headless PRIVATE
  fullphysics_core
  fullphysics_rigid2d
  fullphysics_fluid2d
  fullphysics_particles2d
  fullphysics_aero2d
  fullphysics_runtime
)

if(WIN32)
  add_executable(fullphysics_app WIN32
    src/app/main.c
    src/app/demo_scenes.h
    src/app/demo_scenes.c
    src/app/debug_draw.h
    src/app/debug_draw.c
  )
  target_include_directories(fullphysics_app PRIVATE src)
  target_compile_definitions(fullphysics_app PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(fullphysics_app PRIVATE
    fullphysics_core
    fullphysics_rigid2d
    fullphysics_platform_win32
    fullphysics_simd
    fullphysics_runtime
    fullphysics_draw
  )
endif()

if(MSVC)
  target_compile_options(fullphysics_core PRIVATE /W4)
  target_compile_options(fullphysics_rigid2d PRIVATE /W4)
  target_compile_options(fullphysics_simd PRIVATE /W4)
  target_compile_options(fullphysics_fluid2d PRIVATE /W4)
  target_compile_options(fullphysics_particles2d PRIVATE /W4)
  target_compile_options(fullphysics_aero2d PRIVATE /W4)
  target_compile_options(fullphysics_runtime PRIVATE /W4)
  if(WIN32)
    target_compile_options(fullphysics_platform_win32 PRIVATE /W4)
    target_compile_options(fullphysics_draw PRIVATE /W4)
    target_compile_options(fullphysics_app PRIVATE /W4)
  endif()
else()
  target_compile_options(fullphysics_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_rigid2d PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_simd PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_fluid2d PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_particles2d PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_aero2d PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_runtime PRIVATE -Wall -Wextra -Wpedantic)
  if(WIN32)
    target_compile_options(fullphysics_platform_win32 PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(fullphysics_draw PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(fullphysics_app PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()

enable_testing()
add_executable(fullphysics_tests
  tests/test_main.c
  tests/test_collision.c
  tests/test_determinism.c
  tests/test_simd.c
  tests/test_fluid.c
  tests/test_aero.c
  tests/test_scene.c
  tests/test_raycast.c
  tests/test_queries.c
  tests/test_constraints.c
  tests/test_replay.c
  tests/test_revolute.c
)
target_include_directories(fullphysics_tests PRIVATE src)
target_compile_definitions(fullphysics_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(fullphysics_tests PRIVATE
  fullphysics_core
  fullphysics_rigid2d
  fullphysics_simd
  fullphysics_fluid2d
  fullphysics_particles2d
  fullphysics_aero2d
  fullphysics_runtime
)
add_test(NAME fullphysics_tests COMMAND fullphysics_tests)

add_executable(fullphysics_determinism
  tests/test_determinism_only.c
  tests/test_determinism.c
)
target_include_directories(fullphysics_determinism PRIVATE src)
target_compile_definitions(fullphysics_determinism PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(fullphysics_determinism PRIVATE
  fullphysics_core
  fullphysics_rigid2d
)
add_test(NAME fullphysics_determinism COMMAND fullphysics_determinism)

set(FP_ALL_BUILD_TARGETS
  fullphysics_core
  fullphysics_rigid2d
  fullphysics_simd
  fullphysics_fluid2d
  fullphysics_particles2d
  fullphysics_aero2d
  fullphysics_runtime
  fullphysics_headless
  fullphysics_tests
  fullphysics_determinism
)
if(WIN32)
  list(APPEND FP_ALL_BUILD_TARGETS
    fullphysics_platform_win32
    fullphysics_draw
    fullphysics_app
  )
endif()

foreach(tgt IN LISTS FP_ALL_BUILD_TARGETS)
  if(FP_DETERMINISTIC)
    target_compile_definitions(${tgt} PUBLIC FP_DETERMINISTIC=1)
    if(MSVC)
      target_compile_options(${tgt} PRIVATE /fp:strict)
    else()
      target_compile_options(${tgt} PRIVATE -ffp-contract=off -fno-fast-math)
    endif()
  else()
    target_compile_definitions(${tgt} PUBLIC FP_DETERMINISTIC=0)
  endif()
endforeach()
