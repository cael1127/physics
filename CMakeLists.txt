cmake_minimum_required(VERSION 3.20)

project(FullPhysicsC LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(FP_SIMD "Enable optional SIMD intrinsics" ON)

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

add_library(fullphysics_platform_win32 STATIC
  src/platform/win32/fp_win32.h
  src/platform/win32/fp_win32.c
)
target_include_directories(fullphysics_platform_win32 PUBLIC src)
target_compile_definitions(fullphysics_platform_win32 PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

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

add_executable(fullphysics_app
  src/app/main.c
  src/app/demo_scenes.h
  src/app/demo_scenes.c
  src/app/debug_draw.h
  src/app/debug_draw.c
)
target_include_directories(fullphysics_app PRIVATE src)
target_link_libraries(fullphysics_app PRIVATE
  fullphysics_core
  fullphysics_rigid2d
  fullphysics_platform_win32
  fullphysics_simd
)

if(MSVC)
  target_compile_options(fullphysics_core PRIVATE /W4)
  target_compile_options(fullphysics_rigid2d PRIVATE /W4)
  target_compile_options(fullphysics_platform_win32 PRIVATE /W4)
  target_compile_options(fullphysics_simd PRIVATE /W4)
  target_compile_options(fullphysics_app PRIVATE /W4)
else()
  target_compile_options(fullphysics_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_rigid2d PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_platform_win32 PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_simd PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fullphysics_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(fullphysics_tests
  tests/test_main.c
  tests/test_collision.c
  tests/test_determinism.c
  tests/test_simd.c
)
target_include_directories(fullphysics_tests PRIVATE src)
target_link_libraries(fullphysics_tests PRIVATE fullphysics_core fullphysics_rigid2d fullphysics_simd)
add_test(NAME fullphysics_tests COMMAND fullphysics_tests)

