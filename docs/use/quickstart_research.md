# Research Quickstart

## 1) Configure deterministic build

```powershell
cmake -S . -B build -A x64 -DFP_DETERMINISTIC=ON -DFP_SIMD=ON
cmake --build build --config Release
```

## 2) Run deterministic regression

```powershell
ctest --test-dir build -C Release -R fullphysics_determinism --output-on-failure
```

## 3) Run profile-driven smoke benchmark

```powershell
python bench/run_benchmark.py --profile profiles/smoke.json --out artifacts/bench
```

## 4) Replay snapshot workflow

- Save/load APIs:
  - `fp2_world_save_snapshot(...)`
  - `fp2_world_load_snapshot(...)`
- See `tests/test_replay.c` for end-to-end usage.

## 5) Headless metrics for experiments

Run reproducible non-visual experiments and export metrics:

```powershell
.\build\Release\fullphysics_headless.exe --demo 1 --steps 900 --dt 0.016666667 --csv artifacts\headless.csv --snapshot-out artifacts\headless.fp2snap
```

Predefined benchmark profiles:

- `profiles/headless_stack_metrics.json`
- `profiles/headless_revolute_motor.json`

## 6) Compare runs and generate report

```powershell
python bench/compare_runs.py --a artifacts\run_a.csv --b artifacts\run_b.csv --out-json artifacts\compare.json
python bench/render_report.py --artifacts-root artifacts\bench --out-md artifacts\bench\report.md
```

## 7) Optional pinned baselines

See `docs/reproducibility/baselines.md` for recording golden CSVs with metadata when you need strict regression or classroom autograding.
