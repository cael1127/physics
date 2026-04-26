# Extension Guide

This guide describes how to add new constraints, forces, or solver features safely.

## Add A New Constraint Type

1. Define data in `src/rigid2d/fp2_solver.h`.
2. Add solve routine in `src/rigid2d/fp2_solver.c`.
3. Expose world-level API in `src/rigid2d/fp2_world.h`.
4. Integrate stepping in `src/rigid2d/fp2_world.c`.
5. Add test case in `tests/`.

## Add A New Query

1. Add API in `fp2_world.h`.
2. Implement robust bounds checks in `fp2_world.c`.
3. Add deterministic regression test.

## Add A New Benchmark Profile

1. Create `profiles/<name>.json`.
2. Validate against `profiles/schema.json`.
3. Run with `python bench/run_benchmark.py --profile ...`.
4. Document expected artifacts and acceptance thresholds.

## Definition Of Done

- New behavior has tests.
- Docs updated in Learn/Use/Contribute paths.
- CI passes including determinism.
