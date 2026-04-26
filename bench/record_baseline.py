#!/usr/bin/env python3
"""Record a baseline headless run: copy CSV, capture metadata, optional SHA-256."""
from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


def _git_sha() -> str | None:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"], stderr=subprocess.STDOUT, text=True
        ).strip()
    except Exception:
        return None


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run fullphysics_headless and write baseline artifacts (CSV + metadata JSON)."
    )
    parser.add_argument("--exe", required=True, help="Path to fullphysics_headless executable")
    parser.add_argument(
        "--headless-args",
        required=True,
        help='JSON array of args after the executable, e.g. ["--demo","1","--steps","120"]',
    )
    parser.add_argument("--out-csv", required=True, help="Where to write baseline.csv")
    parser.add_argument("--out-meta", required=True, help="Where to write baseline metadata JSON")
    args = parser.parse_args()

    exe = Path(args.exe)
    if not exe.exists():
        print(f"Executable not found: {exe}", file=sys.stderr)
        return 2

    try:
        extra = json.loads(args.headless_args)
    except json.JSONDecodeError as e:
        print(f"Invalid --headless-args JSON: {e}", file=sys.stderr)
        return 2
    if not isinstance(extra, list) or not all(isinstance(x, (str, int, float)) for x in extra):
        print("--headless-args must be a JSON array of strings/numbers", file=sys.stderr)
        return 2

    out_csv = Path(args.out_csv)
    out_meta = Path(args.out_meta)
    out_csv.parent.mkdir(parents=True, exist_ok=True)
    out_meta.parent.mkdir(parents=True, exist_ok=True)

    cmd = [str(exe)] + [str(x) for x in extra] + ["--csv", str(out_csv)]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stderr or proc.stdout, file=sys.stderr)
        return proc.returncode or 1

    sha = _sha256_file(out_csv) if out_csv.exists() else None
    meta = {
        "recorded_utc": datetime.now(timezone.utc).isoformat(),
        "git_sha": _git_sha(),
        "command": cmd,
        "output_csv": str(out_csv),
        "sha256_csv": sha,
        "headless_exit_code": proc.returncode,
    }
    out_meta.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")
    print(f"baseline_recorded {out_csv}")
    print(f"metadata_written {out_meta}")
    if sha:
        print(f"sha256_csv {sha}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
