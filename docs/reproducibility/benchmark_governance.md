# Benchmark Governance

This document defines how FullPhysicsC benchmarks are versioned, reviewed, and published.

## Goals

- Fair comparisons across revisions.
- Transparent benchmark configuration.
- Reproducible artifact outputs for audits and papers.

## Benchmark Definition

A benchmark run is defined by:

- source revision (git SHA)
- benchmark profile (`profiles/*.json`)
- toolchain and platform metadata
- produced artifacts (`profile.json`, `environment.json`, logs, `summary.json`)

## Change Management

- Any profile change must be reviewed like code.
- Breaking profile semantics requires:
  - schema update (`profiles/schema.json`)
  - migration note in PR description
  - updated docs in this file
- Benchmark claims in PRs should include artifact paths.

## Fairness Rules

- Use fixed timesteps and deterministic build flags for core comparisons.
- Keep benchmark scope stable when comparing revisions.
- Report both success/failure and runtime stats.

## Publication Rules

- Prefer publishing raw artifact bundles over screenshots-only summaries.
- Include commit SHA and profile in reports or papers.
- If data is filtered, document filter rules.
- Include numeric drift comparisons when claiming parity:
  - use `bench/compare_runs.py`
  - include thresholds and pass/fail outcome
- Include generated benchmark tables for reviewers:
  - use `bench/render_report.py`
- Optional pinned metric baselines:
  - see `docs/reproducibility/baselines.md`
  - store small CSV + metadata under `bench/baselines/`

## CI And Release Enforcement

Release gates must include:

1. Full unit tests.
2. Determinism gate.
3. Smoke benchmark execution and artifact capture.
