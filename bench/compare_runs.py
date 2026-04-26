#!/usr/bin/env python3
import argparse
import csv
import json
import math
import sys
from pathlib import Path


def load_csv(path: Path) -> list[dict]:
    with path.open("r", encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def to_float(v: str) -> float:
    try:
        return float(v)
    except Exception:
        return float("nan")


def compare_columns(rows_a: list[dict], rows_b: list[dict], columns: list[str]) -> dict:
    n = min(len(rows_a), len(rows_b))
    out = {"sample_count": n, "columns": {}}
    for col in columns:
        diffs = []
        for i in range(n):
            a = to_float(rows_a[i].get(col, "nan"))
            b = to_float(rows_b[i].get(col, "nan"))
            if math.isnan(a) or math.isnan(b):
                continue
            diffs.append(a - b)
        if not diffs:
            out["columns"][col] = {"sample_count": 0}
            continue
        abs_diffs = [abs(d) for d in diffs]
        mae = sum(abs_diffs) / len(abs_diffs)
        rmse = math.sqrt(sum(d * d for d in diffs) / len(diffs))
        max_abs = max(abs_diffs)
        out["columns"][col] = {
            "sample_count": len(diffs),
            "mae": mae,
            "rmse": rmse,
            "max_abs": max_abs,
        }
    return out


def evaluate_thresholds(stats: dict, thresholds: dict) -> tuple[bool, list[str]]:
    failed = []
    ok = True
    for col, limits in thresholds.items():
        col_stats = stats["columns"].get(col)
        if not col_stats or col_stats.get("sample_count", 0) == 0:
            ok = False
            failed.append(f"{col}: no comparable samples")
            continue
        for metric in ("mae", "rmse", "max_abs"):
            if metric in limits:
                val = float(col_stats.get(metric, float("inf")))
                lim = float(limits[metric])
                if val > lim:
                    ok = False
                    failed.append(f"{col}.{metric}={val:.9g} > {lim:.9g}")
    return ok, failed


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare two headless metrics CSV files.")
    parser.add_argument("--a", required=True, help="Path to baseline CSV")
    parser.add_argument("--b", required=True, help="Path to candidate CSV")
    parser.add_argument(
        "--columns",
        default="kinetic_energy,com_y,contact_count",
        help="Comma-separated numeric columns to compare")
    parser.add_argument(
        "--thresholds-json",
        default="",
        help="Optional JSON thresholds map, e.g. {\"com_y\":{\"rmse\":0.01}}")
    parser.add_argument("--out-json", default="", help="Optional output path for comparison JSON")
    args = parser.parse_args()

    path_a = Path(args.a)
    path_b = Path(args.b)
    if not path_a.exists() or not path_b.exists():
        print("Input CSV path missing", file=sys.stderr)
        return 2

    rows_a = load_csv(path_a)
    rows_b = load_csv(path_b)
    columns = [c.strip() for c in args.columns.split(",") if c.strip()]
    stats = compare_columns(rows_a, rows_b, columns)
    stats["a"] = str(path_a)
    stats["b"] = str(path_b)

    pass_thresholds = True
    failures = []
    if args.thresholds_json:
        thresholds = json.loads(args.thresholds_json)
        pass_thresholds, failures = evaluate_thresholds(stats, thresholds)
        stats["thresholds"] = thresholds
        stats["threshold_pass"] = pass_thresholds
        stats["threshold_failures"] = failures

    payload = json.dumps(stats, indent=2)
    if args.out_json:
        out = Path(args.out_json)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(payload + "\n", encoding="utf-8")
    print(payload)

    if args.thresholds_json and not pass_thresholds:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
