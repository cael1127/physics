# Classroom Labs

These labs are designed for a 2D physics curriculum using FullPhysicsC.

## Lab A: Integration Error

- Compare trajectories under different `dt`.
- Explain why fixed timesteps improve stability and repeatability.
- Reference: `docs/learn/01_integration.md`.

## Lab B: Contact Manifold Behavior

- Observe single-point vs two-point manifold behavior.
- Evaluate stack settling and jitter.
- Reference: `docs/learn/02_collision_and_contacts.md`.

## Lab C: Constraint Tuning

- Tune distance joint stiffness/damping.
- Measure convergence and oscillation.
- Reference: `tests/test_constraints.c`.

## Lab D: Reproducible Experiments

- Run `profiles/smoke.json`.
- Save and load snapshots.
- Package benchmark artifacts for grading.
- Reference: `bench/README.md` and `tests/test_replay.c`.
