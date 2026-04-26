# Pinned Metric Baselines

FullPhysicsC can pin **headless metrics CSVs** for regression testing against a known-good run. This is **optional** and heavier to maintain than the CI **self-consistency** check (two identical runs on the same machine).

## When To Use Pinned Baselines

- You need a fixed reference for a **paper** or **assignment autograder**.
- You are stabilizing a **release** and want to freeze observable metrics for a small scenario set.

## Record A Baseline

1. Build `fullphysics_headless` in Release.
2. Run `bench/record_baseline.py` with the exact headless args you want to pin.

Example (Windows paths):

```powershell
python bench/record_baseline.py `
  --exe .\build\Release\fullphysics_headless.exe `
  --headless-args "[\"--demo\",\"1\",\"--steps\",\"120\",\"--dt\",\"0.016666667\",\"--wind-x\",\"0.25\",\"--gravity-scale\",\"1.0\"]" `
  --out-csv bench/baselines/headless_stack_120win.csv `
  --out-meta bench/baselines/headless_stack_120win.meta.json
```

The metadata JSON includes `sha256_csv` and `git_sha` for audit trails.

## Verify Against A Baseline

Use `bench/verify_pinned_baseline.py` with a tolerances map (see `bench/compare_runs.py`).

```powershell
python bench/verify_pinned_baseline.py `
  --exe .\build\Release\fullphysics_headless.exe `
  --headless-args "[\"--demo\",\"1\",\"--steps\",\"120\",\"--dt\",\"0.016666667\",\"--wind-x\",\"0.25\",\"--gravity-scale\",\"1.0\"]" `
  --baseline-csv bench/baselines/headless_stack_120win.csv `
  --thresholds-json "{\"com_y\":{\"rmse\":0.02},\"kinetic_energy\":{\"max_abs\":0.5}}"
```

Choose tolerances to reflect acceptable numeric drift (compiler updates, small physics changes).

## Relationship To CI

- CI runs **self-consistency** (two identical headless invocations) to catch nondeterminism cheaply.
- Pinned baselines are **not** required in CI by default; enable them when you are ready to commit and maintain them.
