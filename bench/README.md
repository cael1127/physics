# Benchmark Pipeline

FullPhysicsC benchmarks are profile-driven so runs are reproducible and comparable.

## Run

```powershell
python bench/run_benchmark.py --profile profiles/smoke.json --out artifacts/bench
```

## Profile Format

- Schema: `profiles/schema.json`
- A profile defines:
  - `name`
  - `runs`
  - `command` as either:
    - one command array, or
    - platform-specific command map (`windows`, `linux`, `macos`, `default`)

## Produced Artifacts

Each run creates a timestamped folder containing:

- `profile.json`
- `environment.json` (OS/tooling metadata + git SHA if available)
- `run_XXX.json`
- `run_XXX.stdout.log`
- `run_XXX.stderr.log`
- `summary.json`

These files are intended for publication-grade reproducibility and CI audit trails.

## Headless Experiment Runner

`fullphysics_headless` is a CLI executable for deterministic, non-visual runs that emits CSV metrics:

- `step,time,kinetic_energy,com_y,contact_count,body_count`

Example (Windows):

```powershell
.\build\Release\fullphysics_headless.exe --demo 1 --steps 600 --dt 0.016666667 --csv artifacts\run.csv --snapshot-out artifacts\run.fp2snap
```

## Parameter Sweeps

Use `bench/run_sweep.py` to run cartesian sweeps over CLI parameters:

```powershell
python bench/run_sweep.py `
  --exe .\build\Release\fullphysics_headless.exe `
  --out-csv artifacts\sweep.csv `
  --base-args "[\"--demo\",\"1\",\"--steps\",\"240\",\"--dt\",\"0.016666667\"]" `
  --grid wind-x=0,0.5,1.0 `
  --grid gravity-scale=0.8,1.0,1.2
```

## Run Comparison

Compare two headless CSV runs and enforce drift thresholds:

```powershell
python bench/compare_runs.py `
  --a artifacts\run_a.csv `
  --b artifacts\run_b.csv `
  --columns kinetic_energy,com_y,contact_count `
  --thresholds-json "{\"com_y\":{\"rmse\":0.02},\"kinetic_energy\":{\"max_abs\":2.0}}" `
  --out-json artifacts\compare.json
```

If thresholds are provided, non-zero exit code indicates regression.

## Markdown Report

Render a markdown table for all benchmark summaries in an artifact root:

```powershell
python bench/render_report.py --artifacts-root artifacts/bench --out-md artifacts/bench/report.md
```

## CI Self-Consistency Check

Continuous integration runs `fullphysics_headless` twice with identical parameters and requires **bitwise-identical metrics** between the two CSV exports (zero tolerances on `kinetic_energy`, `com_y`, `contact_count`). This catches accidental nondeterminism in the simulation loop without committing per-OS golden CSV baselines.

## Optional Pinned Baselines

For publication or autograding, you can record and check in golden CSVs plus metadata:

- Guide: `docs/reproducibility/baselines.md`
- Record: `bench/record_baseline.py`
- Verify: `bench/verify_pinned_baseline.py`
