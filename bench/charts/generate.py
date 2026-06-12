#!/usr/bin/env python3
"""Generate benchmark comparison charts from bench JSON results."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import matplotlib.pyplot as plt
import seaborn as sns


OUTCOMES = {
    "ok": "#2ca02c",
    "error": "#d62728",
    "wrong_result": "#ff7f0e",
    "timeout": "#9467bd",
    "skipped": "#7f7f7f",
}


def load_results(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def load_baseline(path: Path) -> dict:
    if not path.exists():
        return {"cases": {}}
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def comparison_chart(results: dict, baseline: dict, out: Path) -> None:
    cases = sorted({r["case_name"] for r in results["results"]})
    emu = {r["case_name"]: r for r in results["results"] if r["target"] == "emulator"}
    goccy = {r["case_name"]: r for r in results["results"] if r["target"] == "goccy"}
    bq = baseline.get("cases", {})

    labels, emu_ms, goccy_ms, bq_ms = [], [], [], []
    for case in cases:
        labels.append(case)
        emu_ms.append(_p50_ms(emu.get(case)))
        goccy_ms.append(_p50_ms(goccy.get(case)))
        bq_ms.append(bq.get(case, {}).get("total_p50_ms"))

    x = range(len(labels))
    width = 0.25
    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 0.6), 6))
    ax.bar([i - width for i in x], [v or 0 for v in emu_ms], width, label="vantaboard")
    ax.bar(x, [v or 0 for v in goccy_ms], width, label="goccy")
    ax.bar([i + width for i in x], [v or 0 for v in bq_ms], width, label="bigquery baseline")
    ax.set_yscale("log")
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.set_ylabel("p50 latency (ms)")
    ax.set_title("Benchmark comparison")
    ax.legend()
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def ratio_chart(results: dict, baseline: dict, out: Path) -> None:
    cases = sorted({r["case_name"] for r in results["results"] if r["target"] == "emulator"})
    emu = {r["case_name"]: r for r in results["results"] if r["target"] == "emulator"}
    goccy = {r["case_name"]: r for r in results["results"] if r["target"] == "goccy"}
    bq = baseline.get("cases", {})

    labels, emu_ratio, goccy_ratio = [], [], []
    for case in cases:
        bq_ms = bq.get(case, {}).get("total_p50_ms") or 0
        if bq_ms <= 0:
            continue
        labels.append(case)
        emu_ratio.append(_p50_ms(emu.get(case)) / bq_ms if emu.get(case) else None)
        goccy_ratio.append(_p50_ms(goccy.get(case)) / bq_ms if goccy.get(case) else None)

    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 0.6), 5))
    x = range(len(labels))
    ax.plot(x, emu_ratio, marker="o", label="vantaboard / BQ")
    ax.plot(x, goccy_ratio, marker="s", label="goccy / BQ")
    ax.axhline(1.5, color="red", linestyle="--", label="default threshold (1.5x)")
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.set_ylabel("ratio vs BigQuery p50")
    ax.set_title("Latency ratio vs BigQuery baseline")
    ax.legend()
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def support_matrix(results: dict, out: Path) -> None:
    cases = sorted({r["case_name"] for r in results["results"]})
    targets = sorted({r["target"] for r in results["results"]})
    lookup = {(r["case_name"], r["target"]): r.get("outcome", "error") for r in results["results"]}

    data = []
    for case in cases:
        row = [lookup.get((case, t), "error") for t in targets]
        data.append(row)

    fig, ax = plt.subplots(figsize=(max(8, len(targets) * 2), max(6, len(cases) * 0.35)))
    colors = [[OUTCOMES.get(cell, "#cccccc") for cell in row] for row in data]
    ax.imshow([[0] * len(targets)] * len(cases), aspect="auto")
    for i, case in enumerate(cases):
        for j, target in enumerate(targets):
            outcome = lookup.get((case, target), "error")
            ax.add_patch(plt.Rectangle((j - 0.5, i - 0.5), 1, 1, color=OUTCOMES.get(outcome, "#ccc")))
            ax.text(j, i, outcome[:3], ha="center", va="center", fontsize=8, color="white")
    ax.set_xticks(range(len(targets)))
    ax.set_xticklabels(targets)
    ax.set_yticks(range(len(cases)))
    ax.set_yticklabels(cases)
    ax.set_title("Support / correctness matrix")
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def phases_chart(results: dict, out: Path) -> None:
    emu_rows = [r for r in results["results"] if r["target"] == "emulator" and r.get("phases")]
    if not emu_rows:
        return
    cases = [r["case_name"] for r in emu_rows]
    phase_names = sorted({p for r in emu_rows for p in r.get("phases", {})})
    fig, ax = plt.subplots(figsize=(max(10, len(cases) * 0.5), 6))
    bottom = [0.0] * len(cases)
    for phase in phase_names:
        heights = []
        for r in emu_rows:
            stats = r.get("phases", {}).get(phase, {})
            heights.append((stats.get("p50", 0) or 0) / 1_000_000)
        ax.barh(cases, heights, left=bottom, label=phase)
        bottom = [b + h for b, h in zip(bottom, heights)]
    ax.set_xlabel("seconds (p50)")
    ax.set_title("Engine phase breakdown (vantaboard)")
    ax.legend(loc="lower right", fontsize=8)
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def trend_chart(history: list[dict], out: Path) -> None:
    if not history:
        return
    fig, ax = plt.subplots(figsize=(10, 6))
    for target in ("emulator", "goccy"):
        for case in sorted({r["case_name"] for run in history for r in run.get("results", [])}):
            xs, ys = [], []
            for i, run in enumerate(history):
                for r in run.get("results", []):
                    if r["target"] == target and r["case_name"] == case and r.get("outcome") == "ok":
                        xs.append(i)
                        ys.append(_p50_ms(r))
            if xs:
                ax.plot(xs, ys, marker="o", label=f"{target}:{case}")
    ax.set_yscale("log")
    ax.set_xlabel("run index")
    ax.set_ylabel("p50 ms")
    ax.set_title("Latency trend")
    ax.legend(fontsize=7, ncol=2)
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def _p50_ms(row: dict | None) -> float:
    if not row:
        return 0.0
    lat = row.get("latency", {})
    p50 = lat.get("p50")
    if isinstance(p50, (int, float)):
        # Go encoding/json emits time.Duration as nanoseconds.
        return float(p50) / 1_000_000.0
    if isinstance(p50, str) and p50.endswith("ms"):
        return float(p50[:-2])
    if isinstance(p50, str) and p50.endswith("s"):
        return float(p50[:-1]) * 1000
    return 0.0


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--results", required=True)
    parser.add_argument("--baseline", default="bench/baselines/bigquery.json")
    parser.add_argument("--history", default="")
    parser.add_argument("--out", default="bench/charts/out")
    args = parser.parse_args()

    sns.set_theme(style="whitegrid")
    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    results = load_results(Path(args.results))
    baseline = load_baseline(Path(args.baseline))

    comparison_chart(results, baseline, out_dir / "comparison.svg")
    ratio_chart(results, baseline, out_dir / "ratio.svg")
    support_matrix(results, out_dir / "support_matrix.svg")
    phases_chart(results, out_dir / "phases.svg")

    if args.history:
        history_path = Path(args.history)
        if history_path.exists():
            with history_path.open(encoding="utf-8") as f:
                history = json.load(f)
            trend_chart(history, out_dir / "trend.svg")

    print(f"wrote charts to {out_dir}")


if __name__ == "__main__":
    main()
