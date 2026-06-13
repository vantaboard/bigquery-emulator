#!/usr/bin/env python3
"""Generate benchmark comparison charts from bench JSON results."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import seaborn as sns


OUTCOMES = {
    "ok": "#2ca02c",
    "error": "#d62728",
    "wrong_result": "#ff7f0e",
    "timeout": "#9467bd",
    "skipped": "#7f7f7f",
}

EXPECTED_TARGETS = ("emulator", "goccy")
DEFAULT_RATIO_THRESHOLD = 1.5
RATIO_Y_CAP = 10.0
LOG_FLOOR_MS = 0.05


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


def _goccy_wall_ms(row: dict | None) -> float:
    if not row or row.get("outcome") != "ok":
        return math.nan
    ms = _p50_ms(row)
    return ms if ms > 0 else math.nan


def _ratio_vs_bq(numerator_ms: float, bq_ms: float | None) -> float:
    if bq_ms is None or bq_ms <= 0:
        return math.nan
    if numerator_ms != numerator_ms or numerator_ms <= 0:
        return math.nan
    return numerator_ms / max(bq_ms, 1)


def _emulator_cases(results: dict) -> list[str]:
    return sorted({r["case_name"] for r in results["results"] if r["target"] == "emulator"})


def _plot_capped_ratio_scatter(
    ax: plt.Axes,
    x_positions: list[int],
    ratios: list[float],
    *,
    cap: float,
    color: str,
    marker: str,
    label: str,
    zorder: int = 3,
) -> None:
    xs, ys = [], []
    for xi, ratio in zip(x_positions, ratios):
        if ratio != ratio:
            continue
        xs.append(xi)
        ys.append(min(ratio, cap))
        if ratio > cap:
            ax.annotate(
                f"{ratio:.0f}×",
                (xi, cap),
                textcoords="offset points",
                xytext=(0, 6),
                ha="center",
                fontsize=7,
                color=color,
            )
    if xs:
        ax.scatter(xs, ys, color=color, marker=marker, s=36, label=label, zorder=zorder)


def _plot_skipped_markers(
    ax: plt.Axes,
    x_positions: list[int],
    *,
    y: float,
    label: str,
) -> None:
    if not x_positions:
        return
    ax.scatter(
        x_positions,
        [y] * len(x_positions),
        marker="x",
        color=OUTCOMES["skipped"],
        s=40,
        linewidths=1.5,
        label=label,
        zorder=2,
    )


def _configure_ratio_axes(ax: plt.Axes, labels: list[str], *, title: str, ylabel: str) -> None:
    ax.set_yscale("log")
    ax.set_ylim(bottom=max(LOG_FLOOR_MS / 1000, 0.01))
    ax.axhline(DEFAULT_RATIO_THRESHOLD, color="red", linestyle="--", label=f"threshold ({DEFAULT_RATIO_THRESHOLD}×)")
    ax.set_xticks(range(len(labels)))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.set_ylabel(ylabel)
    ax.set_title(title)


def ratio_vantaboard_chart(results: dict, baseline: dict, out: Path) -> None:
    """Gate-relevant vantaboard server time vs BigQuery job duration."""
    cases = _emulator_cases(results)
    emu = {r["case_name"]: r for r in results["results"] if r["target"] == "emulator"}
    bq = baseline.get("cases", {})

    labels, ratios = [], []
    for case in cases:
        bq_ms = _bq_latency_ms(bq.get(case))
        if bq_ms is None or bq_ms <= 0:
            continue
        labels.append(case)
        ratios.append(_ratio_vs_bq(_emu_server_ms(emu.get(case)), bq_ms))

    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 0.6), 5))
    x = list(range(len(labels)))
    _plot_capped_ratio_scatter(
        ax,
        x,
        ratios,
        cap=RATIO_Y_CAP,
        color="#1f77b4",
        marker="o",
        label="vantaboard / BQ (server)",
    )
    _configure_ratio_axes(
        ax,
        labels,
        title=f"Vantaboard latency ratio vs BigQuery (capped at {RATIO_Y_CAP:.0f}×, log scale)",
        ylabel="ratio vs BigQuery execution p50",
    )
    _legend_outside(fig, ax)
    _save_chart(fig, out)


def ratio_goccy_chart(results: dict, baseline: dict, out: Path) -> None:
    """Competitive context: goccy wall-clock vs BigQuery server duration."""
    cases = _emulator_cases(results)
    goccy = {r["case_name"]: r for r in results["results"] if r["target"] == "goccy"}
    bq = baseline.get("cases", {})

    labels, ratios, skipped_x = [], [], []
    for case in cases:
        bq_ms = _bq_latency_ms(bq.get(case))
        if bq_ms is None or bq_ms <= 0:
            continue
        labels.append(case)
        xi = len(labels) - 1
        row = goccy.get(case)
        if row and row.get("outcome") == "ok":
            ratios.append(_ratio_vs_bq(_goccy_wall_ms(row), bq_ms))
        else:
            ratios.append(math.nan)
            skipped_x.append(xi)

    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 0.6), 5))
    x = list(range(len(labels)))
    _plot_capped_ratio_scatter(
        ax,
        x,
        ratios,
        cap=RATIO_Y_CAP,
        color="#ff7f0e",
        marker="s",
        label="goccy / BQ (wall vs server)",
    )
    _plot_skipped_markers(
        ax,
        skipped_x,
        y=0.05,
        label="goccy skipped",
    )
    _configure_ratio_axes(
        ax,
        labels,
        title=f"Goccy latency ratio vs BigQuery (capped at {RATIO_Y_CAP:.0f}×, log scale)",
        ylabel="ratio vs BigQuery execution p50",
    )
    _legend_outside(fig, ax)
    _save_chart(fig, out)


def comparison_chart(results: dict, baseline: dict, out: Path) -> None:
    cases = sorted({r["case_name"] for r in results["results"]})
    emu = {r["case_name"]: r for r in results["results"] if r["target"] == "emulator"}
    goccy = {r["case_name"]: r for r in results["results"] if r["target"] == "goccy"}
    bq = baseline.get("cases", {})

    labels, emu_ms, goccy_ms, bq_ms = [], [], [], []
    goccy_skipped_x: list[int] = []
    for case in cases:
        labels.append(case)
        xi = len(labels) - 1
        emu_ms.append(_emu_server_ms(emu.get(case)))
        row = goccy.get(case)
        if row and row.get("outcome") == "skipped":
            goccy_ms.append(math.nan)
            goccy_skipped_x.append(xi)
        else:
            goccy_ms.append(_goccy_wall_ms(row))
        bq_val = _bq_latency_ms(bq.get(case))
        bq_ms.append(bq_val if bq_val is not None else math.nan)

    x = range(len(labels))
    width = 0.25
    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 0.6), 6))
    ax.bar([i - width for i in x], emu_ms, width, label="vantaboard (total_engine)")
    ax.bar(x, goccy_ms, width, label="goccy (wall)")
    ax.bar([i + width for i in x], bq_ms, width, label="bigquery (job duration)")
    if goccy_skipped_x:
        ax.scatter(
            goccy_skipped_x,
            [LOG_FLOOR_MS] * len(goccy_skipped_x),
            marker="x",
            color=OUTCOMES["skipped"],
            s=50,
            linewidths=1.5,
            label="goccy skipped",
            zorder=5,
        )
    ax.set_yscale("log")
    ax.set_ylim(bottom=LOG_FLOOR_MS)
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.set_ylabel("server p50 latency (ms)")
    title = "Benchmark comparison"
    missing = _missing_targets(results)
    if missing:
        title += f" (missing: {', '.join(missing)} — run task bench:run)"
    ax.set_title(title)
    _legend_outside(fig, ax)
    _save_chart(fig, out)


def support_matrix(results: dict, out: Path) -> None:
    cases = sorted({r["case_name"] for r in results["results"]})
    targets = sorted({r["target"] for r in results["results"]})
    lookup = {(r["case_name"], r["target"]): r.get("outcome", "error") for r in results["results"]}

    fig, ax = plt.subplots(figsize=(max(8, len(targets) * 2), max(6, len(cases) * 0.35)))
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
    legend_handles = [Patch(facecolor=color, label=label) for label, color in OUTCOMES.items()]
    ax.legend(handles=legend_handles, loc="upper left", bbox_to_anchor=(1.02, 1), borderaxespad=0)
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
    ratio_vantaboard_chart(results, baseline, out_dir / "ratio.svg")
    ratio_goccy_chart(results, baseline, out_dir / "ratio_goccy.svg")
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
