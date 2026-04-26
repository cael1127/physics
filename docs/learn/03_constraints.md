# 03 Constraints And Joints

Goal: understand distance and revolute constraints as first-class extension points.

## Concepts

- A distance joint preserves spacing between two bodies.
- A revolute joint keeps anchor points coincident and supports optional motorized angular velocity.
- Constraint solving combines positional correction and damping.
- Joint APIs should be composable with contact solving.

## Code Landmarks

- `src/rigid2d/fp2_solver.h` (`Fp2DistanceJoint`)
- `src/rigid2d/fp2_solver.h` (`Fp2DistanceJoint`, `Fp2RevoluteJoint`)
- `src/rigid2d/fp2_solver.c` (`fp2_solve_distance_joints`, `fp2_solve_revolute_joints`)
- `src/rigid2d/fp2_world.h` (`fp2_world_add_distance_joint`, `fp2_world_add_revolute_joint`)

## Mini Lab

1. Review `tests/test_constraints.c` and `tests/test_revolute.c`.
2. Change stiffness and damping values.
3. Measure convergence to `rest_length`.

Expected result: higher stiffness converges faster, damping reduces oscillation.
