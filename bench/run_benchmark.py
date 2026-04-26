#!/usr/bin/env python3
import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path


def _read_profile(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def _platform_key() -> str:
    p = sys.platform.lower()
    if p.startswith("win"):
        return "windows"
    if p.startswith("linux"):
        return "linux"
    if p.startswith("darwin"):
        return "macos"
    return "default"


def _resolve_command(profile: dict) -> list[str]:
    cmd = profile.get("command")
    if not cmd:
        raise ValueError("Profile must define 'command'")
    if isinstance(cmd, list):
        return [str(x) for x in cmd]
    if isinstance(cmd, dict):
        key = _platform_key()
        selected = cmd.get(key) or cmd.get("default")
        if not selected:
            raise ValueError(f"Profile command missing platform key '{key}' and 'default'")
        return [str(x) for x in selected]
    raise ValueError("'command' must be list or object")


def _env_metadata() -> dict:
    git_sha = None
    try:
        out = subprocess.check_output(["git", "rev-parse", "HEAD"], stderr=subprocess.STDOUT, text=True)
        git_sha = out.strip()
    except Exception:
        git_sha = None
    return {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "platform": platform.platform(),
        "python_version": sys.version,
        "machine": platform.machine(),
        "processor": platform.processor(),
        "git_sha": git_sha,
    }


def _write_json(path: Path, payload: dict) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)


def run(profile_path: Path, out_root: Path) -> int:
    profile = _read_profile(profile_path)
    runs = int(profile.get("runs", 1))
    if runs <= 0:
        raise ValueError("'runs' must be >= 1")

    cmd = _resolve_command(profile)
    name = str(profile.get("name", profile_path.stem))

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    out_dir = out_root / f"{name}-{stamp}"
    out_dir.mkdir(parents=True, exist_ok=True)

    _write_json(out_dir / "profile.json", profile)
    _write_json(out_dir / "environment.json", _env_metadata())

    run_results = []
    for i in range(runs):
        start = time.perf_counter()
        proc = subprocess.run(cmd, capture_output=True, text=True)
        elapsed_ms = int((time.perf_counter() - start) * 1000.0)

        run_result = {
            "run_index": i,
            "command": cmd,
            "exit_code": proc.returncode,
            "elapsed_ms": elapsed_ms,
            "stdout_file": f"run_{i:03d}.stdout.log",
            "stderr_file": f"run_{i:03d}.stderr.log",
        }
        run_results.append(run_result)

        (out_dir / run_result["stdout_file"]).write_text(proc.stdout or "", encoding="utf-8")
        (out_dir / run_result["stderr_file"]).write_text(proc.stderr or "", encoding="utf-8")
        _write_json(out_dir / f"run_{i:03d}.json", run_result)

    success = all(r["exit_code"] == 0 for r in run_results)
    summary = {
        "name": name,
        "profile": str(profile_path),
        "runs": run_results,
        "success": success,
        "min_elapsed_ms": min(r["elapsed_ms"] for r in run_results),
        "max_elapsed_ms": max(r["elapsed_ms"] for r in run_results),
        "mean_elapsed_ms": int(sum(r["elapsed_ms"] for r in run_results) / len(run_results)),
    }
    _write_json(out_dir / "summary.json", summary)
    print(f"[bench] wrote artifacts to {out_dir}")
    print(f"[bench] success={success}")
    return 0 if success else 1


def main() -> int:
    parser = argparse.ArgumentParser(description="Run profile-driven benchmark and write reproducible artifacts.")
    parser.add_argument("--profile", required=True, help="Path to benchmark profile JSON")
    parser.add_argument("--out", default="artifacts/bench", help="Output root directory for artifacts")
    args = parser.parse_args()

    profile_path = Path(args.profile)
    if not profile_path.exists():
        print(f"[bench] profile not found: {profile_path}", file=sys.stderr)
        return 2

    out_root = Path(args.out)
    out_root.mkdir(parents=True, exist_ok=True)

    return run(profile_path, out_root)


if __name__ == "__main__":
    raise SystemExit(main())
