#!/usr/bin/env python3
import argparse
import json
from pathlib import Path


def load_summary(path: Path) -> dict | None:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return None


def collect_summaries(root: Path) -> list[tuple[Path, dict]]:
    out = []
    for p in sorted(root.glob("**/summary.json")):
        data = load_summary(p)
        if data is not None:
            out.append((p, data))
    return out


def render_markdown(summaries: list[tuple[Path, dict]], root: Path) -> str:
    lines = ["# Benchmark Report", "", f"Artifact root: `{root}`", ""]
    if not summaries:
        lines += ["No `summary.json` files found.", ""]
        return "\n".join(lines)

    lines += ["| Benchmark | Success | Runs | Mean ms | Min ms | Max ms | Summary |", "|---|---:|---:|---:|---:|---:|---|"]
    for path, s in summaries:
        name = str(s.get("name", "?"))
        success = "yes" if s.get("success") else "no"
        runs = len(s.get("runs", []))
        mean_ms = s.get("mean_elapsed_ms", "-")
        min_ms = s.get("min_elapsed_ms", "-")
        max_ms = s.get("max_elapsed_ms", "-")
        rel = path.relative_to(root)
        lines.append(f"| {name} | {success} | {runs} | {mean_ms} | {min_ms} | {max_ms} | `{rel}` |")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Render markdown report from benchmark summary artifacts.")
    parser.add_argument("--artifacts-root", required=True, help="Root directory containing benchmark artifacts")
    parser.add_argument("--out-md", required=True, help="Output markdown report path")
    args = parser.parse_args()

    root = Path(args.artifacts_root)
    out_md = Path(args.out_md)
    out_md.parent.mkdir(parents=True, exist_ok=True)

    summaries = collect_summaries(root)
    md = render_markdown(summaries, root)
    out_md.write_text(md + "\n", encoding="utf-8")
    print(f"report_written {out_md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
