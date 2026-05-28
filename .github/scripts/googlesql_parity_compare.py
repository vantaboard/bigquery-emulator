#!/usr/bin/env python3
"""Compare GoogleSQL source-vs-prebuilt parity-job results (Phase 5).

The two parity legs (`build-prebuilt` and `build-source` in
`.github/workflows/googlesql-parity.yml`) each emit:

  * A step-level `outcome` for the engine startup smoke
    (`emulator_main --version`).
  * A step-level `outcome` for the smoke query subset
    (every tier).
  * A step-level `outcome` for the duckdb conformance lane
    (scheduled / release tiers only).
  * JSON conformance reports uploaded as workflow artifacts (downloaded
    by the comparator job into `--prebuilt-results-dir` /
    `--source-results-dir`).

This script aggregates those into:

  * A workflow summary block (markdown) showing the per-mode identity
    + per-stage status side-by-side.
  * A per-fixture diff for any conformance lane where the two modes
    disagreed (PASS vs FAIL on the same fixture × profile).
  * A workflow-level exit code: non-zero on any divergence, missing
    leg, or unexpected schema_version mismatch in the reports.

The script also enforces the "no silent prebuilt->source fallback"
contract by treating a `build-prebuilt` leg in the `result == 'failure'`
state as a parity-fail rather than swallowing it: if the prebuilt artifact
wouldn't validate or link, that's exactly the regression the parity job
exists to surface.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import sys
from collections.abc import Iterable


SCHEMA_VERSION = 1


def _load_report(path: pathlib.Path) -> dict | None:
    if not path.is_file():
        return None
    try:
        return json.loads(path.read_text())
    except json.JSONDecodeError as exc:
        print(
            f"compare: failed to parse {path}: {exc}",
            file=sys.stderr,
        )
        return None


def _norm_status(value: str | None) -> str:
    """GitHub Actions step outcomes: success / failure / cancelled / skipped."""
    if not value:
        return "missing"
    return value.lower()


def _outcome_emoji(value: str) -> str:
    return {
        "success": "PASS",
        "failure": "FAIL",
        "cancelled": "CANCELLED",
        "skipped": "skipped",
        "missing": "missing",
    }.get(value, value)


def _diff_reports(
    prebuilt: dict | None,
    source: dict | None,
    *,
    label: str,
) -> tuple[list[str], list[str]]:
    """Compare two conformance Reports; return (notes, diffs).

    `notes` lists summary-level mismatches (counts, missing reports).
    `diffs` lists per-fixture status disagreements as
    `fixture(profile): prebuilt=PASS source=FAIL` lines.
    """
    notes: list[str] = []
    diffs: list[str] = []
    if prebuilt is None and source is None:
        notes.append(f"{label}: neither leg produced a report")
        return notes, diffs
    if prebuilt is None:
        notes.append(f"{label}: prebuilt-leg report missing")
        return notes, diffs
    if source is None:
        notes.append(f"{label}: source-leg report missing")
        return notes, diffs

    p_schema = prebuilt.get("schema_version")
    s_schema = source.get("schema_version")
    if p_schema != SCHEMA_VERSION or s_schema != SCHEMA_VERSION:
        notes.append(
            f"{label}: schema_version mismatch with comparator "
            f"(expected {SCHEMA_VERSION}; prebuilt={p_schema} source={s_schema})"
        )

    p_summary = prebuilt.get("summary", {})
    s_summary = source.get("summary", {})
    if p_summary != s_summary:
        notes.append(
            f"{label}: summary diverged. prebuilt={p_summary} source={s_summary}"
        )

    # Index per (fixture, profile) so cross-leg comparisons survive
    # different fixture-ordering.
    def index(results: Iterable[dict]) -> dict[tuple[str, str], str]:
        out: dict[tuple[str, str], str] = {}
        for r in results:
            key = (r.get("fixture", "?"), r.get("profile", "?"))
            out[key] = r.get("status", "?")
        return out

    p_idx = index(prebuilt.get("results", []))
    s_idx = index(source.get("results", []))
    keys = sorted(set(p_idx.keys()) | set(s_idx.keys()))
    for k in keys:
        fixture, profile = k
        ps = p_idx.get(k, "MISSING")
        ss = s_idx.get(k, "MISSING")
        if ps != ss:
            diffs.append(
                f"{fixture} ({profile}): prebuilt={ps} source={ss}"
            )
    return notes, diffs


def _render_summary(
    args: argparse.Namespace,
    stage_table: list[tuple[str, str, str, str]],
    parity_notes: list[str],
    parity_diffs: list[tuple[str, list[str]]],
) -> str:
    """Markdown summary written to GITHUB_STEP_SUMMARY."""
    lines: list[str] = []
    lines.append(f"## GoogleSQL source-vs-prebuilt parity (tier: `{args.tier}`)")
    lines.append("")
    lines.append("### Artifact identity")
    lines.append("")
    lines.append("| Mode | googlesql commit | tag | artifact_version | schema | compiler |")
    lines.append("|------|------------------|-----|------------------|--------|----------|")
    lines.append(
        "| prebuilt | `"
        + (args.prebuilt_googlesql_commit or "?")
        + "` | `"
        + (args.prebuilt_googlesql_tag or "?")
        + "` | `"
        + (args.prebuilt_artifact_version or "?")
        + "` | `"
        + (args.prebuilt_schema_version or "?")
        + "` | `"
        + (args.prebuilt_compiler or "?")
        + "` |"
    )
    lines.append(
        "| source   | `"
        + (args.source_googlesql_commit or "?")
        + "` | `"
        + (args.source_googlesql_tag or "?")
        + "` | (n/a) | (n/a) | (host clang) |"
    )
    lines.append("")
    lines.append("### Per-stage outcomes")
    lines.append("")
    lines.append("| Stage | Prebuilt | Source | Status |")
    lines.append("|-------|----------|--------|--------|")
    for stage, prebuilt, source, status in stage_table:
        lines.append(f"| {stage} | {prebuilt} | {source} | {status} |")
    lines.append("")
    if parity_notes:
        lines.append("### Notes")
        lines.append("")
        for note in parity_notes:
            lines.append(f"- {note}")
        lines.append("")
    if parity_diffs:
        lines.append("### Per-fixture divergences")
        lines.append("")
        for label, diffs in parity_diffs:
            lines.append(f"#### {label}")
            lines.append("")
            for d in diffs:
                lines.append(f"- {d}")
            lines.append("")
    return "\n".join(lines) + "\n"


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tier", required=True, choices=("pr", "scheduled", "release"))

    # Top-level job outcomes (`needs.*.result` from the workflow):
    # success / failure / cancelled / skipped.
    parser.add_argument("--prebuilt-result", required=True)
    parser.add_argument("--source-result", required=True)

    # Per-step outcomes harvested from each leg's outputs.
    parser.add_argument("--prebuilt-smoke", default="")
    parser.add_argument("--source-smoke", default="")
    parser.add_argument("--prebuilt-conformance-duckdb", default="")
    parser.add_argument("--source-conformance-duckdb", default="")

    # Artifact identity surfaces.
    parser.add_argument("--prebuilt-googlesql-commit", default="")
    parser.add_argument("--source-googlesql-commit", default="")
    parser.add_argument("--prebuilt-artifact-version", default="")
    parser.add_argument("--prebuilt-googlesql-tag", default="")
    parser.add_argument("--source-googlesql-tag", default="")
    parser.add_argument("--prebuilt-schema-version", default="")
    parser.add_argument("--prebuilt-compiler", default="")

    # Conformance report directories (downloaded artifacts).
    parser.add_argument(
        "--prebuilt-results-dir",
        type=pathlib.Path,
        required=True,
    )
    parser.add_argument(
        "--source-results-dir",
        type=pathlib.Path,
        required=True,
    )

    # Where to render the markdown summary (typically $GITHUB_STEP_SUMMARY).
    parser.add_argument(
        "--github-step-summary",
        type=pathlib.Path,
        default=None,
    )

    args = parser.parse_args(argv)

    # Collect per-stage outcomes side-by-side.
    stage_rows: list[tuple[str, str, str, str]] = []

    def add_stage(name: str, prebuilt: str, source: str) -> None:
        p = _outcome_emoji(_norm_status(prebuilt))
        s = _outcome_emoji(_norm_status(source))
        if p == s:
            status = "matched"
        elif p in ("missing", "skipped") or s in ("missing", "skipped"):
            status = "n/a (one leg absent)"
        else:
            status = "DIVERGED"
        stage_rows.append((name, p, s, status))

    add_stage("Build + link + startup", args.prebuilt_result, args.source_result)
    add_stage("Smoke query set", args.prebuilt_smoke, args.source_smoke)
    if args.tier in ("scheduled", "release"):
        add_stage(
            "Conformance (duckdb)",
            args.prebuilt_conformance_duckdb,
            args.source_conformance_duckdb,
        )

    # Per-fixture diff for each report we have on disk.
    parity_notes: list[str] = []
    parity_diffs: list[tuple[str, list[str]]] = []

    def diff_pair(label: str, prebuilt_name: str, source_name: str) -> None:
        p = _load_report(args.prebuilt_results_dir / prebuilt_name)
        s = _load_report(args.source_results_dir / source_name)
        notes, diffs = _diff_reports(p, s, label=label)
        parity_notes.extend(notes)
        if diffs:
            parity_diffs.append((label, diffs))

    diff_pair(
        "smoke", "conformance-smoke-prebuilt.json", "conformance-smoke-source.json"
    )
    if args.tier in ("scheduled", "release"):
        diff_pair(
            "conformance (duckdb)",
            "conformance-duckdb-prebuilt.json",
            "conformance-duckdb-source.json",
        )

    summary_md = _render_summary(args, stage_rows, parity_notes, parity_diffs)
    print(summary_md)
    if args.github_step_summary is not None:
        try:
            with args.github_step_summary.open("a", encoding="utf-8") as fh:
                fh.write(summary_md)
        except OSError as exc:
            print(
                f"compare: failed to append to {args.github_step_summary}: {exc}",
                file=sys.stderr,
            )

    # Fail the comparator job on any divergence, missing leg, or
    # comparator-detected error.
    diverged = any(row[3] == "DIVERGED" for row in stage_rows)
    if diverged or parity_diffs:
        print(
            "::error::GoogleSQL source-vs-prebuilt parity FAILED — "
            "see workflow summary for details.",
            file=sys.stderr,
        )
        return 1
    if parity_notes:
        # Notes alone (e.g. summary-count drift, missing reports) are
        # still a parity warning that should fail the job — silent
        # drift is exactly the failure mode this lane exists to catch.
        print(
            "::error::GoogleSQL parity comparator surfaced notes — see "
            "workflow summary for details.",
            file=sys.stderr,
        )
        return 1
    print("::notice::GoogleSQL source-vs-prebuilt parity OK")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
