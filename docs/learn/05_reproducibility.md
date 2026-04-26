# 05 Reproducibility And Replay

Goal: produce rerunnable simulation artifacts for research and classroom grading.

## Concepts

- Deterministic build + fixed runtime stepping.
- Profile-driven benchmark execution with captured metadata.
- World snapshot save/load for replayable state transitions.

## Code Landmarks

- `docs/reproducibility/determinism_contract.md`
- `bench/run_benchmark.py`
- `profiles/smoke.json`
- `src/rigid2d/fp2_world.h` (`fp2_world_save_snapshot`, `fp2_world_load_snapshot`)
- `tests/test_replay.c`

## Mini Lab

1. Run smoke benchmark profile.
2. Save a world snapshot.
3. Reload and continue simulation from the snapshot.

Expected result: replayed runs stay numerically close and produce auditable artifacts.
