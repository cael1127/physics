#!/usr/bin/env python3
import argparse
import csv
import itertools
import json
import subprocess
import sys
from pathlib import Path


def parse_values(spec: str) -> list[str]:
  return [v.strip() for v in spec.split(",") if v.strip()]


def main() -> int:
  parser = argparse.ArgumentParser(description="Run Cartesian sweep of headless CLI parameters.")
  parser.add_argument("--exe", required=True, help="Path to fullphysics_headless executable")
  parser.add_argument("--out-csv", required=True, help="Output CSV path for sweep summary")
  parser.add_argument("--base-args", default="", help="JSON array of base args, e.g. [\"--demo\",\"1\"]")
  parser.add_argument(
      "--grid",
      action="append",
      default=[],
      help="Grid axis as key=comma,separated,values. Example: --grid wind-x=0,0.5,1.0")
  args = parser.parse_args()

  exe = Path(args.exe)
  if not exe.exists():
    print(f"Executable not found: {exe}", file=sys.stderr)
    return 2

  base_args = json.loads(args.base_args) if args.base_args else []
  if not isinstance(base_args, list):
    print("--base-args must decode to a JSON array", file=sys.stderr)
    return 2

  keys = []
  values = []
  for axis in args.grid:
    if "=" not in axis:
      print(f"Invalid --grid axis: {axis}", file=sys.stderr)
      return 2
    key, spec = axis.split("=", 1)
    key = key.strip()
    vals = parse_values(spec)
    if not key or not vals:
      print(f"Invalid --grid axis: {axis}", file=sys.stderr)
      return 2
    keys.append(key)
    values.append(vals)

  rows = []
  for combo in itertools.product(*values) if values else [()]:
    run_args = [str(exe)] + [str(x) for x in base_args]
    params = {}
    for k, v in zip(keys, combo):
      run_args.extend([f"--{k}", str(v)])
      params[k] = v
    proc = subprocess.run(run_args, capture_output=True, text=True)
    row = {
        "exit_code": proc.returncode,
        "stdout": (proc.stdout or "").strip().replace("\n", " "),
        "stderr": (proc.stderr or "").strip().replace("\n", " "),
    }
    row.update(params)
    rows.append(row)

  out_csv = Path(args.out_csv)
  out_csv.parent.mkdir(parents=True, exist_ok=True)
  fieldnames = list(keys) + ["exit_code", "stdout", "stderr"]
  with out_csv.open("w", newline="", encoding="utf-8") as f:
    w = csv.DictWriter(f, fieldnames=fieldnames)
    w.writeheader()
    for row in rows:
      w.writerow({k: row.get(k, "") for k in fieldnames})

  fails = sum(1 for r in rows if int(r["exit_code"]) != 0)
  print(f"sweep_complete rows={len(rows)} failures={fails} out={out_csv}")
  return 0 if fails == 0 else 1


if __name__ == "__main__":
  raise SystemExit(main())
