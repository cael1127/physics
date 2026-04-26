# Contributing To FullPhysicsC

Thanks for contributing. This project is research- and education-first, so reproducibility and clarity are required.

## Development Principles

1. Keep APIs small, composable, and documented.
2. Preserve deterministic behavior unless explicitly changing the contract.
3. Prefer measurable improvements (tests, benchmarks, docs) over large speculative rewrites.

## Required For Every PR

- Build passes in CI.
- `fullphysics_tests` passes.
- `fullphysics_determinism` passes.
- If physics behavior changed, update or add benchmark profile evidence.
- If user-facing API changed, update docs in `docs/`.

## Physics Changes Checklist

- Add/extend regression tests in `tests/`.
- Explain numerical tradeoffs in PR description.
- Include reproducibility impact (none/low/medium/high).
- If adding solver features, include a small scenario proving behavior.

## Docs Checklist

- Add/update Learn/Use/Contribute docs paths:
  - `docs/learn/`
  - `docs/use/`
  - `docs/contribute/`
- Keep examples minimal and runnable.

## Benchmark And Artifact Rules

- Benchmark profiles live under `profiles/`.
- Benchmark runner artifacts must be reproducible:
  - profile
  - environment metadata
  - stdout/stderr
  - summary metrics
- Do not alter historical benchmark result interpretation without updating governance docs.
- Optional pinned baselines for strict regression:
  - follow `docs/reproducibility/baselines.md`
