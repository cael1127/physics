#!/usr/bin/env python3
"""Run headless, compare to a pinned baseline CSV with tolerances (compare_runs)."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run fullphysics_headless and compare to a baseline CSV with thresholds."
    )
    parser.add_argument("--exe", required=True, help="Path to fullphysics_headless executable")
    parser.add_argument(
        "--headless-args",
        required=True,
        help='JSON array of args, e.g. ["--demo","1","--steps","120","--dt","0.016666667"]',
    )
    parser.add_argument("--baseline-csv", required=True, help="Pinned baseline CSV path")
    parser.add_argument(
        "--thresholds-json",
        required=True,
        help='JSON thresholds for compare_runs, e.g. {"com_y":{"rmse":0.02}}',
    )
    parser.add_argument("--out-csv", default="artifacts/verify/current.csv", help="Current run CSV")
    parser.add_argument(
        "--out-compare",
        default="artifacts/verify/compare.json",
        help="Output path for compare_runs JSON",
    )
    args = parser.parse_args()

    exe = Path(args.exe)
    if not exe.exists():
        print(f"Executable not found: {exe}", file=sys.stderr)
        return 2
    base = Path(args.baseline_csv)
    if not base.exists():
        print(f"Baseline not found: {base}", file=sys.stderr)
        return 2

    try:
        extra = json.loads(args.headless_args)
    except json.JSONDecodeError as e:
        print(f"Invalid --headless-args: {e}", file=sys.stderr)
        return 2
    if not isinstance(extra, list):
        print("--headless-args must be a JSON array", file=sys.stderr)
        return 2

    out_csv = Path(args.out_csv)
    out_csv.parent.mkdir(parents=True, exist_ok=True)
    out_compare = Path(args.out_compare)
    out_compare.parent.mkdir(parents=True, exist_ok=True)

    run_cmd = [str(exe)] + [str(x) for x in extra] + ["--csv", str(out_csv)]
    proc = subprocess.run(run_cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stderr or proc.stdout, file=sys.stderr)
        return proc.returncode or 1

    comp_cmd = [
        sys.executable,
        str(Path(__file__).resolve().parent / "compare_runs.py"),
        "--a",
        str(base),
        "--b",
        str(out_csv),
        "--columns",
        "kinetic_energy,com_y,contact_count",
        "--thresholds-json",
        args.thresholds_json,
        "--out-json",
        str(out_compare),
    ]
    return subprocess.call(comp_cmd)


if __name__ == "__main__":
    raise SystemExit(main())
