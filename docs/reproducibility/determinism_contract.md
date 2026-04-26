# Determinism And Reproducibility Contract

This document defines what FullPhysicsC guarantees today and how runs must be configured to be reproducible for research and teaching.

## Determinism Levels

- **Level 1: same-process determinism**
  - Same executable, same initial state, same fixed timestep and solver iteration counts.
  - Expected result: identical simulation trajectory within floating-point tolerance.
- **Level 2: cross-build reproducibility**
  - Same source revision and benchmark profile, deterministic compiler flags enabled.
  - Expected result: numerically close trajectories with bounded drift.
- **Level 3: publishable experiment reproducibility**
  - Full artifact set (profile, environment metadata, output files) is captured and versioned.
  - Expected result: another user can rerun and audit results.

## Required Runtime Rules

1. Use a **fixed dt** for stepping (for example `1/60`), never variable frame dt directly in physics.
2. Keep body creation order deterministic.
3. Use identical solver settings (`vel_iters`, `pos_iters`) across runs.
4. Disable nondeterministic randomization in experiment setup unless the seed is fixed and recorded.
5. Record engine revision and benchmark profile with outputs.

## Build Requirements

The CMake option `FP_DETERMINISTIC=ON` is enabled by default.

- MSVC: `/fp:strict`
- GCC/Clang-compatible: `-ffp-contract=off -fno-fast-math`
- Compile definition exposed to all targets: `FP_DETERMINISTIC=1`

If strict determinism is not required for a local exploratory run:

```powershell
cmake -S . -B build -A x64 -DFP_DETERMINISTIC=OFF
```

## Validation Requirements

- `tests/test_determinism.c` must pass.
- CI must execute deterministic test scenarios on at least:
  - Windows + MSVC
  - Linux + GCC or Clang
- Any solver/collision change must include a determinism regression test update.

## Research Artifact Checklist

Every published experiment should include:

- commit SHA
- benchmark profile file
- compiler/toolchain version
- OS/CPU metadata
- simulation outputs and summary metrics

This contract is intentionally strict for research and classroom reproducibility, and may evolve with new solver features.
