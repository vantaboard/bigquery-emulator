#!/usr/bin/env python3
"""Generate benchmark comparison charts from bench JSON results."""

from __future__ import annotations

import argparse
import json
import math
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

EXPECTED_TARGETS = ("emulator", "goccy")
LOG_AXIS_FLOOR_MS = 0.02
SKIPPED_MARKER_MS = 0.15


def _save_chart(fig: plt.Figure, out: Path) -> None:
    fig.savefig(out, format="svg", bbox_inches="tight", pad_inches=0.1)
    plt.close(fig)


def _legend_outside(
    fig: plt.Figure,
    ax: plt.Axes,
    *,
    loc: str = "outside upper right",
    **kwargs,
) -> None:
    handles, labels = ax.get_legend_handles_labels()
    if not handles:
        return
    fig.legend(handles, labels, loc=loc, **kwargs)


def _legend_below(
    fig: plt.Figure,
    ax: plt.Axes,
    *,
    ncol: int = 4,
    fontsize: float = 8,
) -> None:
    """Multi-column legend in a reserved strip below the axes (avoids x-axis overlap)."""
    handles, labels = ax.get_legend_handles_labels()
    if not handles:
        return
    nrows = math.ceil(len(handles) / ncol)
    bottom = min(0.45, 0.14 + 0.042 * nrows)
    fig.subplots_adjust(bottom=bottom)
    fig.legend(
        handles,
        labels,
        loc="center",
        bbox_to_anchor=(0.5, bottom * 0.45),
        ncol=ncol,
        fontsize=fontsize,
    )


def load_results(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def load_baseline(path: Path) -> dict:
    if not path.exists():
        return {"cases": {}}
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def _missing_targets(results: dict) -> list[str]:
    present = {r["target"] for r in results["results"]}
    return [t for t in EXPECTED_TARGETS if t not in present]


def _bq_latency_ms(case_baseline: dict | None) -> float | None:
    if not case_baseline:
        return None
    ms = case_baseline.get("execution_p50_ms")
    if not ms or ms <= 0:
        ms = case_baseline.get("total_p50_ms")
    return ms


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


def _emu_server_ms(row: dict | None) -> float:
    if not row or row.get("outcome") != "ok":
        return math.nan
    engine_p50 = row.get("engine_p50")
    if isinstance(engine_p50, (int, float)) and engine_p50 > 0:
        return float(engine_p50) / 1_000_000.0
    phases = row.get("phases") or {}
    te = phases.get("total_engine") or {}
    p50 = te.get("p50")
    if isinstance(p50, (int, float)) and p50 > 0:
        return float(p50) / 1_000_000.0
    return _p50_ms(row)


def _emu_wall_ms(row: dict | None) -> float:
    """Emulator HTTP round-trip p50 (apples-to-apples vs goccy wall)."""
    if not row or row.get("outcome") != "ok":
        return math.nan
    ms = _p50_ms(row)
    return ms if ms > 0 else math.nan


def _goccy_wall_ms(row: dict | None) -> float:
    if not row or row.get("outcome") != "ok":
        return math.nan
    ms = _p50_ms(row)
    return ms if ms > 0 else math.nan


def comparison_chart(results: dict, baseline: dict, out: Path) -> None:
    cases = sorted({r["case_name"] for r in results["results"]})
    emu = {r["case_name"]: r for r in results["results"] if r["target"] == "emulator"}
    goccy = {r["case_name"]: r for r in results["results"] if r["target"] == "goccy"}
    bq = baseline.get("cases", {})

    labels, emu_wall_ms, emu_engine_ms, goccy_ms, bq_ms = [], [], [], [], []
    goccy_skipped_x: list[int] = []
    for case in cases:
        labels.append(case)
        xi = len(labels) - 1
        emu_wall_ms.append(_emu_wall_ms(emu.get(case)))
        emu_engine_ms.append(_emu_server_ms(emu.get(case)))
        row = goccy.get(case)
        if row and row.get("outcome") == "skipped":
            goccy_ms.append(math.nan)
            goccy_skipped_x.append(xi)
        else:
            goccy_ms.append(_goccy_wall_ms(row))
        bq_val = _bq_latency_ms(bq.get(case))
        bq_ms.append(bq_val if bq_val is not None else math.nan)

    x = range(len(labels))
    # Four grouped bars: vantaboard wall (apples-to-apples vs goccy wall),
    # vantaboard engine-only, goccy wall, and BQ server-side job duration.
    width = 0.2
    offsets = (-1.5 * width, -0.5 * width, 0.5 * width, 1.5 * width)
    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 0.7), 6))
    ax.bar([i + offsets[0] for i in x], emu_wall_ms, width, label="vantaboard (wall)")
    ax.bar([i + offsets[1] for i in x], emu_engine_ms, width, label="vantaboard (total_engine)")
    ax.bar([i + offsets[2] for i in x], goccy_ms, width, label="goccy (wall)")
    ax.bar([i + offsets[3] for i in x], bq_ms, width, label="bigquery (job duration)")
    if goccy_skipped_x:
        ax.scatter(
            [i + offsets[2] for i in goccy_skipped_x],
            [SKIPPED_MARKER_MS] * len(goccy_skipped_x),
            marker="x",
            color=OUTCOMES["skipped"],
            s=50,
            linewidths=1.5,
            label="goccy skipped",
            zorder=5,
            clip_on=False,
        )
    ax.set_yscale("log")
    ax.set_ylim(bottom=LOG_AXIS_FLOOR_MS)
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.set_ylabel("p50 latency (ms, log scale)")
    title = "Benchmark comparison"
    missing = _missing_targets(results)
    if missing:
        title += f" (missing: {', '.join(missing)} — run task bench:run)"
    ax.set_title(title)
    _legend_outside(fig, ax)
    _save_chart(fig, out)


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
    _legend_below(fig, ax, ncol=4, fontsize=8)
    _save_chart(fig, out)


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
                        if target == "emulator":
                            ys.append(_emu_server_ms(r))
                        else:
                            ys.append(_p50_ms(r))
            if xs:
                ax.plot(xs, ys, marker="o", label=f"{target}:{case}")
    ax.set_yscale("log")
    ax.set_xlabel("run index")
    ax.set_ylabel("p50 ms")
    ax.set_title("Latency trend")
    _legend_outside(fig, ax, loc="outside lower center", fontsize=7, ncol=4)
    _save_chart(fig, out)


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
    missing = _missing_targets(results)
    if missing:
        print(f"warning: results missing targets {missing}; run task bench:run for full charts")

    comparison_chart(results, baseline, out_dir / "comparison.svg")
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
