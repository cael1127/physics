# 01 Integration And Timestepping

Goal: understand why FullPhysicsC uses fixed timesteps and semi-implicit integration.

## Concepts

- Use fixed `dt` for stable, reproducible stepping.
- Integrate velocity before position in each step.
- Keep physics stepping separate from render frame rate.

## Code Landmarks

- `src/core/fp_timestep.h`
- `src/core/fp_timestep.c`
- `src/rigid2d/fp2_solver.c` (`fp2_integrate_vel`, `fp2_integrate_pos`)

## Mini Lab

1. Run the app and toggle pause with Space.
2. Change gravity scale with O/L and observe velocity integration effects.
3. Repeat the same input sequence twice and compare outcomes.

Expected result: with fixed stepping, trajectories are stable and repeatable.
