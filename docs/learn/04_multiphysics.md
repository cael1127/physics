# 04 Multi-Physics Composition

Goal: inspect how rigid, fluid, aero, and particles are orchestrated in one step loop.

## Concepts

- Keep subsystem responsibilities separate.
- Use explicit step order for predictable coupling.
- Parameterize coupling (wind, gravity scale, emit rate) at runtime.

## Code Landmarks

- `src/runtime/fp_scene.h`
- `src/runtime/fp_scene.c`
- `src/aero2d/fp_aero2d.c`
- `src/fluid2d/fp_fluid2d.c`
- `src/particles2d/fp_particles2d.c`

## Mini Lab

1. Toggle fluid (`F`) and wind overlay (`W`).
2. Adjust wind with Q/E.
3. Emit dye with D/H and particles with P.

Expected result: consistent coupling with clear visualization of subsystem interactions.
