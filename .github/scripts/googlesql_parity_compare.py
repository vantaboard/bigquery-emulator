#!/usr/bin/env python3
"""Compare GoogleSQL source-vs-prebuilt parity-job results (safety-gate parity check).

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
    disagreed (PASS vs FAIL on the same fixture x profile).
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
INFRA_NOTE_MARKERS = (
    "failed before upload",
    "not a parity divergence",
)


def _resolve_report(results_dir: pathlib.Path, name: str) -> pathlib.Path | None:
    """Find a conformance JSON report under results_dir.

    upload-artifact@v6 nests files under workspace paths when an absolute
    path (e.g. /tmp/googlesql-validate.json) is mixed with repo-root globs.
    Prefer a direct child match, then fall back to the shallowest rglob hit.
    """
    direct = results_dir / name
    if direct.is_file():
        return direct
    matches = [p for p in results_dir.rglob(name) if p.is_file()]
    if not matches:
        return None
    matches.sort(key=lambda p: len(p.parts))
    return matches[0]


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


def _job_failed(result: str | None) -> bool:
    return _norm_status(result) == "failure"


def _outcome_emoji(value: str) -> str:
    return {
        "success": "PASS",
        "failure": "FAIL",
        "cancelled": "CANCELLED",
        "skipped": "skipped",
        "missing": "missing",
    }.get(value, value)


def _is_infra_note(note: str) -> bool:
    return any(marker in note for marker in INFRA_NOTE_MARKERS)


def _diff_reports(
    prebuilt: dict | None,
    source: dict | None,
    *,
    label: str,
    prebuilt_leg_failed: bool,
    source_leg_failed: bool,
) -> tuple[list[str], list[str]]:
    """Compare two conformance Reports; return (notes, diffs).

    `notes` lists summary-level mismatches (counts, missing reports).
    `diffs` lists per-fixture status disagreements as
    `fixture(profile): prebuilt=PASS source=FAIL` lines.
    """
    notes: list[str] = []
    diffs: list[str] = []
    if prebuilt is None and source is None:
        if prebuilt_leg_failed and source_leg_failed:
            notes.append(
                f"{label}: both legs failed before upload "
                "(build/infra failure — not a parity divergence)"
            )
        elif prebuilt_leg_failed or source_leg_failed:
            notes.append(
                f"{label}: report missing because a leg failed before upload "
                f"(prebuilt={'failed' if prebuilt_leg_failed else 'ok'}, "
                f"source={'failed' if source_leg_failed else 'ok'})"
            )
        else:
            notes.append(f"{label}: neither leg produced a report")
        return notes, diffs
    if prebuilt is None:
        if prebuilt_leg_failed:
            notes.append(
                f"{label}: prebuilt-leg report missing "
                "(prebuilt job failed before upload)"
            )
        else:
            notes.append(f"{label}: prebuilt-leg report missing")
        return notes, diffs
    if source is None:
        if source_leg_failed:
            notes.append(
                f"{label}: source-leg report missing "
                "(source job failed before upload)"
            )
        else:
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
    lanes_label = args.lanes if args.lanes != "all" else "smoke + conformance"
    lines.append(
        f"## GoogleSQL source-vs-prebuilt parity "
        f"(tier: `{args.tier}`, lanes: `{lanes_label}`)"
    )
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
    parser.add_argument(
        "--lanes",
        default="all",
        choices=("smoke", "conformance", "all"),
        help="Which conformance JSON lanes to compare (default: all).",
    )

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

    prebuilt_failed = _job_failed(args.prebuilt_result)
    source_failed = _job_failed(args.source_result)

    compare_smoke = args.lanes in ("smoke", "all")
    compare_conformance = (
        args.lanes in ("conformance", "all") and args.tier in ("scheduled", "release")
    )

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
    if compare_smoke:
        add_stage("Smoke query set", args.prebuilt_smoke, args.source_smoke)
    if compare_conformance:
        add_stage(
            "Conformance (duckdb)",
            args.prebuilt_conformance_duckdb,
            args.source_conformance_duckdb,
        )

    # Per-fixture diff for each report we have on disk.
    parity_notes: list[str] = []
    parity_diffs: list[tuple[str, list[str]]] = []

    def diff_pair(label: str, prebuilt_name: str, source_name: str) -> None:
        p_path = _resolve_report(args.prebuilt_results_dir, prebuilt_name)
        s_path = _resolve_report(args.source_results_dir, source_name)
        p = _load_report(p_path) if p_path is not None else None
        s = _load_report(s_path) if s_path is not None else None
        notes, diffs = _diff_reports(
            p,
            s,
            label=label,
            prebuilt_leg_failed=prebuilt_failed,
            source_leg_failed=source_failed,
        )
        parity_notes.extend(notes)
        if diffs:
            parity_diffs.append((label, diffs))

    if compare_smoke:
        diff_pair(
            "smoke",
            "conformance-smoke-prebuilt.json",
            "conformance-smoke-source.json",
        )
    if compare_conformance:
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
            "reports exist but diverge; see workflow summary for details.",
            file=sys.stderr,
        )
        return 1

    infra_notes = [n for n in parity_notes if _is_infra_note(n)]
    other_notes = [n for n in parity_notes if n not in infra_notes]

    if infra_notes and not other_notes:
        print(
            "::error::GoogleSQL parity legs failed before upload — "
            "fix build/infra (not prebuilt-vs-source drift); "
            "see workflow summary for details.",
            file=sys.stderr,
        )
        return 1

    if other_notes:
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
