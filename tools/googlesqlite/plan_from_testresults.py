#!/usr/bin/env python3
"""Classify googlesqlite emulator test failures and emit .cursor/plans/*.plan.md."""

from __future__ import annotations

import argparse
import json
import re
import textwrap
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_JSON = (
    REPO_ROOT
    / "gateway/e2e/testresults/googlesqlite-emulator-20260603T035812Z.json"
)
PLANS_DIR = REPO_ROOT / ".cursor/plans"

DATE_TIME_MARKERS = (
    "parse_date",
    "parse_timestamp",
    "parse_datetime",
    "parse_time",
    "date_",
    "datetime_",
    "timestamp",
    "time_add",
    "time_sub",
    "time_diff",
    "time_trunc",
    "current_date",
    "current_time",
    "current_timestamp",
    "current_datetime",
    "unix_",
    "make_interval",
    "justify_",
    "last_day",
    "date_from_unix",
    "unix_date",
    "extract_date",
    "extract_from_timestamp",
    "extract_from_interval",
    "minimum_/_maximum_timestamp",
    "interval_operator",
    "interval_from_sub",
    "cast_integer_to_datetime",
    "date_type",
    "date_operator",
)

JSON_MARKERS = (
    "json_",
    "parse_json",
    "to_json",
    "JSON",
)

ARRAY_MARKERS = (
    "generate_array",
    "generate_date",
    "generate_timestamp",
    "array_",
    "UNNEST",
    "unnest",
    "make_array",
    "array_scan",
    "null_array_scan",
)

ADVANCED_RELATIONAL_MARKERS = (
    "GROUPING",
    "ROLLUP",
    "CUBE",
    "PIVOT",
    "UNPIVOT",
    "recursive",
    "TABLE SAMPLE",
    "qualify",
    "group_by_rollup",
    "grouping_sets",
)


@dataclass
class PlanSpec:
    plan_id: str
    slug: str
    title: str
    goal: str
    primary_files: list[str]
    implementation_steps: list[str]
    dependencies: list[str] = field(default_factory=list)
    blocked_by_notes: str = ""
    todo_id: str = ""
    todo_content: str = ""


PLAN_SPECS: dict[str, PlanSpec] = {
    "01": PlanSpec(
        plan_id="01",
        slug="ddl-catalog",
        title="DDL / Catalog / Gateway SQL",
        goal="Fix unqualified DDL table names, dataset bootstrap, and catalog 404s blocking 14 googlesqlite tests.",
        primary_files=[
            "gateway/e2e/emulator_sql.go",
            "backend/engine/control/control_op_executor.cc",
            "backend/catalog/googlesql_catalog.h",
        ],
        implementation_steps=[
            "Teach `emulator_sql.go` to qualify bare `CREATE TABLE t` as `dataset.table` using the test project default dataset.",
            "Relax or branch `control_op_executor` DDL name parsing so single-segment names accepted when a default dataset is in scope.",
            "Auto-create datasets referenced by googlesqlite tests before DDL runs (404 `Not found: Dataset …`).",
            "Return BigQuery-shaped success for `CREATE TABLE` / `CREATE TABLE AS` used in test setup.",
            "Add focused unit tests in `control_op_executor_test.cc` for one-segment vs three-segment names.",
        ],
        todo_id="gsql-01-ddl-catalog",
        todo_content="Qualify DDL names and auto-create datasets for googlesqlite port tests",
    ),
    "02": PlanSpec(
        plan_id="02",
        slug="withscan-cte",
        title="WithScan / CTE Emit",
        goal="Implement DuckDB lowering for `ResolvedWithScan` / `ResolvedWithRefScan` to unblock ~58 googlesqlite tests.",
        primary_files=[
            "backend/engine/duckdb/transpiler/transpiler.cc",
            "backend/engine/duckdb/transpiler/transpiler.h",
            "conformance/fixtures/cte_subquery/",
        ],
        implementation_steps=[
            "Implement `EmitWithScan` to emit `WITH cte AS (subquery) …` preserving column aliases.",
            "Implement `EmitWithRefScan` for CTE references inside the same query.",
            "Wire emit paths through existing join / scan recursion; fail closed only when a child shape is unsupported.",
            "Add conformance fixture asserting `duckdb_native` route for non-recursive CTE.",
            "Re-run plan verify regex; expect `TestJoinVariants` and CTE-heavy `TestQuery` rows to pass.",
        ],
        dependencies=["googlesqlite-01-ddl-catalog.plan.md"],
        todo_id="gsql-02-withscan-cte",
        todo_content="Implement EmitWithScan/EmitWithRefScan in DuckDB transpiler",
    ),
    "03": PlanSpec(
        plan_id="03",
        slug="operator-disposition",
        title="Operator Dispositions",
        goal="Register internal comparison/arithmetic operators (`$equal`, `$less`, `$add`, …) in the disposition table.",
        primary_files=[
            "backend/engine/duckdb/transpiler/functions.yaml",
            "backend/engine/coordinator/route_classifier.cc",
            "conformance/dispositions/",
        ],
        implementation_steps=[
            "Add `functions.yaml` rows for `$equal`, `$less`, `$greater`, `$add`, `$subtract`, `$multiply`, `$divide` with `duckdb_native` or `semantic_executor` as appropriate.",
            "Ensure transpiler maps internal operator names to DuckDB SQL operators.",
            "Regenerate functions table if the repo uses `functions_table_gen.awk`.",
            "Verify `TestQualifyClause`, `TestComputedColumnsInProject`, `TestRecursiveCTE` no longer hit `has no disposition`.",
        ],
        dependencies=["googlesqlite-01-ddl-catalog.plan.md"],
        todo_id="gsql-03-operator-disposition",
        todo_content="Add disposition rows for internal $-prefixed operators",
    ),
    "04": PlanSpec(
        plan_id="04",
        slug="scan-emits",
        title="Scan Emits (ProjectScan, OrderByScan, …)",
        goal="Close missing DuckDB emit coverage for core scan nodes blocking ~56 googlesqlite tests.",
        primary_files=[
            "backend/engine/duckdb/transpiler/transpiler.cc",
            "backend/engine/duckdb/duckdb_executor.cc",
            "backend/engine/duckdb/transpiler/SHAPE_TRACKER.md",
        ],
        implementation_steps=[
            "Implement or complete `EmitProjectScan` for SELECT-list / computed-column projections.",
            "Implement `EmitOrderByScan`, `EmitLimitOffsetScan`, and `EmitSetOperationScan` where still returning empty SQL.",
            "Update 501 error strings to cite `googlesqlite-04-scan-emits.plan.md`.",
            "Add one conformance fixture per newly supported scan kind.",
            "Run plan verify regex against `TestQuery` ProjectScan failures.",
        ],
        dependencies=[
            "googlesqlite-02-withscan-cte.plan.md",
            "googlesqlite-03-operator-disposition.plan.md",
        ],
        todo_id="gsql-04-scan-emits",
        todo_content="Implement missing DuckDB scan emit methods",
    ),
    "05": PlanSpec(
        plan_id="05",
        slug="arrow-marshaling",
        title="Arrow → BigQuery Result Marshaling",
        goal="Fix `arrow_to_bq` failures for anonymous/window column names blocking ~19 googlesqlite tests.",
        primary_files=[
            "backend/engine/duckdb/arrow_to_bq.cc",
            "backend/engine/duckdb/duckdb_executor.cc",
            "gateway/e2e/emulator_sql.go",
        ],
        implementation_steps=[
            "Map DuckDB anonymous columns (`$col1`, window outputs) to stable BigQuery field names in Arrow→JSON conversion.",
            "Support INT64 columns backed by non-standard Arrow physical types used by window functions.",
            "Add regression tests for window frame queries that currently fail in `TestWindowVariousFrames`.",
            "Re-verify GROUPING SETS tests that fail with `product_sum` column marshaling errors.",
        ],
        dependencies=["googlesqlite-04-scan-emits.plan.md"],
        blocked_by_notes="Some GROUPING SETS tests may still need plan 13 SQL lowering before rows match.",
        todo_id="gsql-05-arrow-marshaling",
        todo_content="Fix arrow_to_bq column naming for window and anonymous columns",
    ),
    "06": PlanSpec(
        plan_id="06",
        slug="aggregate-modifiers",
        title="Aggregate Modifiers (ORDER BY / LIMIT / HAVING)",
        goal="Support BigQuery `array_agg` / `string_agg` modifiers blocking ~8 googlesqlite tests.",
        primary_files=[
            "backend/engine/duckdb/transpiler/transpiler.cc",
            "backend/engine/duckdb/transpiler/functions.yaml",
        ],
        implementation_steps=[
            "Extend aggregate emit to translate ORDER BY / LIMIT / DISTINCT modifiers on `array_agg` and `string_agg`.",
            "Route modifier-heavy aggregates to `duckdb_rewrite` when DuckDB syntax differs from BigQuery.",
            "Cover dedicated tests: `TestArrayAggLimitOrderBy`, `TestStringAggOrderByLimit`, `TestAggregateModifiers`.",
            "Mirror modifier behavior for matching `TestQuery/array_agg_*` and `TestQuery/string_agg_*` subtests.",
        ],
        dependencies=["googlesqlite-04-scan-emits.plan.md"],
        todo_id="gsql-06-aggregate-modifiers",
        todo_content="Emit array_agg/string_agg with ORDER BY and LIMIT modifiers",
    ),
    "07": PlanSpec(
        plan_id="07",
        slug="semantic-core-expr",
        title="Semantic Core Expressions / Scan Walk",
        goal="Extend semantic executor scan walk and expression kinds for ~22 googlesqlite tests.",
        primary_files=[
            "backend/engine/semantic/eval_expr.cc",
            "backend/engine/semantic/eval_expr.h",
            "backend/engine/coordinator/local_coordinator_engine.cc",
        ],
        implementation_steps=[
            "Allow semantic SELECT path to handle `OrderByScan`, `WithScan`, and nested scan trees without requiring top-level `ProjectScan`.",
            "Implement `ResolvedSubqueryExpr` evaluation (scalar, EXISTS, IN, ARRAY forms).",
            "Implement `MakeStruct`, `GetStructField`, and correlated `ResolvedArrayScan` where listed in failures.",
            "Add semantic executor unit tests mirroring failing subtests.",
        ],
        dependencies=[
            "googlesqlite-05-arrow-marshaling.plan.md",
            "googlesqlite-06-aggregate-modifiers.plan.md",
        ],
        todo_id="gsql-07-semantic-core-expr",
        todo_content="Semantic scan walk + SubqueryExpr/struct/array expr kinds",
    ),
    "08": PlanSpec(
        plan_id="08",
        slug="semantic-operators",
        title="Semantic Internal Operators",
        goal="Implement `$like`, `$between`, `$in`, bitwise, and IS DISTINCT FROM operators (~30 tests).",
        primary_files=[
            "backend/engine/semantic/eval_expr.cc",
            "backend/engine/semantic/functions/",
            "backend/engine/duckdb/transpiler/functions.yaml",
        ],
        implementation_steps=[
            "Register internal operators in `functions.yaml` with `semantic_executor` disposition.",
            "Implement BigQuery-exact NULL semantics for `$like`, `$between`, `$in`, `$is_true`, `$is_false`.",
            "Implement bitwise and `$is_distinct_from` / `$is_not_distinct_from` with BigQuery truth tables.",
            "Implement `round` with precision argument in semantic path.",
            "Verify all matching `TestQuery/*operator*` subtests in this bucket.",
        ],
        dependencies=["googlesqlite-07-semantic-core-expr.plan.md"],
        todo_id="gsql-08-semantic-operators",
        todo_content="Semantic evaluator for $-prefixed comparison/bitwise operators",
    ),
    "09": PlanSpec(
        plan_id="09",
        slug="date-time",
        title="Semantic Date / Time Functions",
        goal="Implement date/time parse, format, diff, trunc, and current_* functions (~150 tests).",
        primary_files=[
            "backend/engine/semantic/functions/",
            "backend/engine/semantic/eval_expr.cc",
        ],
        implementation_steps=[
            "Phase A: `parse_date`, `parse_datetime`, `parse_timestamp`, `parse_time` with format-string coverage from failures.",
            "Phase B: `format_date`, `format_datetime`, `format_timestamp`, `format_time`, and `format` numeric variants.",
            "Phase C: `date_add/sub/diff/trunc`, `datetime_*`, `timestamp_*`, `time_*`, `current_*`, unix conversions.",
            "Phase D: interval helpers (`make_interval`, `justify_*`) and extract functions.",
            "Add focused C++ tests per phase; run plan verify regex after each phase.",
        ],
        dependencies=["googlesqlite-08-semantic-operators.plan.md"],
        todo_id="gsql-09-date-time",
        todo_content="Semantic date/time function registry (phased)",
    ),
    "10": PlanSpec(
        plan_id="10",
        slug="string-hash-format",
        title="Semantic String / Hash / Format",
        goal="Implement string, regex, hash, encoding, and `format` functions (~94 tests).",
        primary_files=[
            "backend/engine/semantic/functions/",
            "backend/engine/semantic/eval_expr.cc",
        ],
        implementation_steps=[
            "Implement core string funcs: `concat`, `substr`, `length`, `lower`/`upper`, `trim`, `split`, `replace`.",
            "Implement regex family: `regexp_contains`, `regexp_extract`, `regexp_replace`, `regexp_instr`.",
            "Implement hash/encoding: `md5`, `sha1`, `sha256`, `sha512`, `to_base64`, `from_hex`, etc.",
            "Implement `format`, `least`/`greatest`, and remaining single-test string functions from failure list.",
        ],
        dependencies=["googlesqlite-09-date-time.plan.md"],
        todo_id="gsql-10-string-hash-format",
        todo_content="Semantic string, regex, hash, and format functions",
    ),
    "11": PlanSpec(
        plan_id="11",
        slug="json",
        title="Semantic JSON Functions",
        goal="Implement JSON extract/query/value/array functions (~42 tests).",
        primary_files=[
            "backend/engine/semantic/functions/",
            "backend/engine/semantic/eval_expr.cc",
        ],
        implementation_steps=[
            "Implement `json_extract`, `json_query`, `json_value`, `json_extract_scalar` with escape/format args.",
            "Implement array variants: `json_extract_array`, `json_query_array`, `json_value_array`, string array forms.",
            "Implement `parse_json`, `to_json`, `to_json_string`, and JSON type helpers (`json_bool`, `json_int64`, …).",
            "Match BigQuery NULL and empty-array edge cases from `TestQuery/json_*` subtests.",
        ],
        dependencies=["googlesqlite-10-string-hash-format.plan.md"],
        todo_id="gsql-11-json",
        todo_content="Semantic JSON function family",
    ),
    "12": PlanSpec(
        plan_id="12",
        slug="arrays-generators",
        title="Semantic Arrays / Generators",
        goal="Implement array generators, UNNEST paths, and array combinators (~20 tests).",
        primary_files=[
            "backend/engine/semantic/functions/",
            "backend/engine/semantic/eval_expr.cc",
            "backend/engine/coordinator/local_coordinator_engine.cc",
        ],
        implementation_steps=[
            "Implement `generate_array`, `generate_date_array`, `generate_timestamp_array`.",
            "Implement `array_concat`, `array_length`, `array_reverse`, and array scan join helpers.",
            "Fix semantic UNNEST / `WITH OFFSET` paths (`TestUnnestWithOffset`, `TestArrayTransformLambda`).",
            "Verify array literal concat and struct-array tests from failure list.",
        ],
        dependencies=["googlesqlite-11-json.plan.md"],
        todo_id="gsql-12-arrays-generators",
        todo_content="Semantic array generators and UNNEST paths",
    ),
    "13": PlanSpec(
        plan_id="13",
        slug="advanced-relational",
        title="Advanced Relational (GROUPING SETS, PIVOT, …)",
        goal="Land GROUPING SETS / ROLLUP / CUBE / PIVOT / recursive CTE SQL (~3+ dedicated tests; overlaps plan 05).",
        primary_files=[
            "backend/engine/duckdb/transpiler/transpiler.cc",
            "backend/engine/coordinator/local_coordinator_engine.cc",
        ],
        implementation_steps=[
            "Lower GROUPING SETS / ROLLUP / CUBE through DuckDB-compatible rewrite or semantic executor.",
            "Implement PIVOT / UNPIVOT emit (`TestPivotAndUnpivotExtra`, `TestQuery/PIVOT`).",
            "Fix recursive CTE evaluation (`TestRecursiveCTE`, `TestWithRecursiveSelfReferential`).",
            "Re-run tests that failed on arrow marshaling for grouping aggregates after SQL is correct.",
        ],
        dependencies=["googlesqlite-12-arrays-generators.plan.md"],
        blocked_by_notes="Re-verify plan 05 failures after this plan lands.",
        todo_id="gsql-13-advanced-relational",
        todo_content="GROUPING SETS, PIVOT, recursive CTE lowering",
    ),
    "14": PlanSpec(
        plan_id="14",
        slug="dml-system",
        title="DML / System Variables / information_schema",
        goal="Support DML setup statements, `@@` system variables, and information_schema queries.",
        primary_files=[
            "backend/engine/semantic/dml/dml_executor.h",
            "backend/engine/control/control_op_executor.cc",
            "backend/catalog/googlesql_catalog.h",
            "gateway/e2e/emulator_sql.go",
        ],
        implementation_steps=[
            "Support multi-statement scripts or sequential exec in `emulator_sql.go` for CREATE+INSERT patterns.",
            "Implement UPDATE/DELETE/INSERT paths used by dedicated DML tests.",
            "Recognize `@@time_zone` and related system variables (`TestSystemVariableSet`, `TestSystemVariableTimeZone`).",
            "Expose information_schema tables for googlesqlite catalog introspection tests.",
        ],
        dependencies=["googlesqlite-13-advanced-relational.plan.md"],
        todo_id="gsql-14-dml-system",
        todo_content="DML executor, system variables, information_schema",
    ),
    "15": PlanSpec(
        plan_id="15",
        slug="specialized-stubs",
        title="Specialized / NET / UDF Stubs",
        goal="Resolve specialized-feature 501s and NET.* / UDF tests (~24 tests) with local impl or deterministic stubs.",
        primary_files=[
            "backend/engine/semantic/stubs/",
            "backend/engine/coordinator/route_classifier.cc",
            "backend/engine/duckdb/transpiler/functions.yaml",
        ],
        implementation_steps=[
            "Replace legacy specialized-feature plan references in 501 messages with this plan.",
            "For each failing specialized family, choose local_impl vs local_stub vs unsupported per test expectation.",
            "Implement or stub `net.*` functions referenced by `TestQuery/net_*` subtests.",
            "Wire CREATE FUNCTION / temp function tests to udf routing or explicit unsupported errors matching googlesqlite expectations.",
        ],
        dependencies=["googlesqlite-14-dml-system.plan.md"],
        todo_id="gsql-15-specialized-stubs",
        todo_content="Specialized/NET/UDF policy and minimal implementations",
    ),
    "16": PlanSpec(
        plan_id="16",
        slug="result-fixes",
        title="Result / Driver Fixes",
        goal="Fix remaining wrong results, empty row sets, and Go driver scan type mismatches (~16 tests).",
        primary_files=[
            "gateway/e2e/emulator_sql.go",
            "backend/engine/coordinator/local_coordinator_engine.cc",
        ],
        implementation_steps=[
            "Fix `sql.NullInt64` scanning: return NULL as sql.NullInt64 valid=false instead of bare int64.",
            "Fix LIMIT/OFFSET result delivery (`TestLimitOffset`).",
            "Fix EXISTS/IN/subquery tests returning empty result sets when rows expected.",
            "Sweep any tests still failing after plans 01–15; re-baseline with `run_googlesqlite_emulator_tests.sh`.",
        ],
        dependencies=["googlesqlite-15-specialized-stubs.plan.md"],
        blocked_by_notes="Run last; some tests here may clear earlier once upstream plans land.",
        todo_id="gsql-16-result-fixes",
        todo_content="emulator_sql type mapping and empty-result semantic bugs",
    ),
}


def load_results(path: Path) -> tuple[list[str], dict[str, str]]:
    failed: list[str] = []
    outputs: dict[str, list[str]] = defaultdict(list)
    for raw in path.read_text(encoding="utf-8").splitlines():
        raw = raw.strip()
        if not raw:
            continue
        try:
            ev = json.loads(raw)
        except json.JSONDecodeError:
            continue
        test = ev.get("Test") or ""
        action = ev.get("Action")
        if action == "fail" and test:
            failed.append(test)
        if action == "output" and test and ev.get("Output"):
            outputs[test].append(ev["Output"])
    merged = {k: "".join(v) for k, v in outputs.items()}
    return failed, merged


def classify_test(test: str, text: str) -> str:
    if "DDL target name must be" in text or (
        "404" in text and "Dataset" in text
    ):
        return "01"
    if "Expected end of input but got keyword CREATE" in text:
        return "14"
    if "node:WithScan" in text or (
        "WithScan" in text and "501" in text and "semantic" not in text
    ):
        return "02"
    if any(
        marker in text
        for marker in (
            "node:ProjectScan",
            "node:OrderByScan",
            "node:LimitOffsetScan",
            "node:SetOperationScan",
        )
    ) and "transpiler does not yet cover" in text:
        return "04"
    if re.search(r"function '\$[^']+' has no disposition", text):
        if "transpiler does not yet cover" not in text:
            return "03"
        if re.search(
            r"function '\$(equal|less|greater|add|subtract|multiply|divide|not_equal)' has no disposition",
            text,
        ):
            return "03"
    if "arrow_to_bq" in text:
        return "05"
    if "aggregate" in text and "modifier" in text:
        return "06"
    if (
        "semantic: SELECT path expects" in text
        or "ResolvedExpr kind" in text
        or "correlated ResolvedArrayScan" in text
    ):
        return "07"
    if re.search(r"function '\$", text) and "is not yet implemented" in text:
        return "08"
    if any(m in text for m in DATE_TIME_MARKERS):
        return "09"
    if any(m in text for m in JSON_MARKERS):
        return "11"
    if any(m in text for m in ARRAY_MARKERS):
        return "12"
    if any(m in text for m in ADVANCED_RELATIONAL_MARKERS):
        return "13"
    if (
        "UPDATE" in text
        or "INSERT" in text
        or "DELETE" in text
        or "information_schema" in text.lower()
        or "@@time_zone" in text
        or "Unrecognized name: @@" in text
    ):
        return "14"
    if "specialized-feature-policy" in text or "net." in text:
        return "15"
    if "create_function" in text or "create_temp_function" in text:
        return "15"
    if (
        "Scan:" in text
        or "sql: no rows" in text
        or "expected" in text.lower()
        or "want" in text.lower()
    ):
        return "16"
    if "501" in text or "notImplemented" in text:
        return "10"
    return "10"


def extract_root_causes(
    tests: Iterable[str], outputs: dict[str, str], limit: int = 3
) -> list[str]:
    seen: list[str] = []
    for test in tests:
        text = outputs.get(test, "")
        for line in text.splitlines():
            line = line.strip()
            if not line or line.startswith("==="):
                continue
            if "googlesqlite_query_test.go:" in line or "transpiler" in line:
                msg = line
                if len(msg) > 200:
                    msg = msg[:197] + "..."
                if msg not in seen:
                    seen.append(msg)
            if len(seen) >= limit:
                return seen
    return seen


def build_run_regex(tests: list[str]) -> str:
    """Build a go test -run regex from failing test names."""
    if not tests:
        return "^$"

    parts = [re.escape(t) + "$" for t in sorted(tests)]
    if len(parts) == 1:
        return "^" + parts[0]
    return "^(" + "|".join(parts) + ")"


def yaml_quote(s: str) -> str:
    if any(c in s for c in ':"\'\n#'):
        return json.dumps(s)
    return s


def render_plan_markdown(
    spec: PlanSpec,
    tests: list[str],
    outputs: dict[str, str],
    baseline_stamp: str,
) -> str:
    root_causes = extract_root_causes(tests, outputs)
    run_regex = build_run_regex(tests)
    test_count = len(tests)

    lines: list[str] = [
        "---",
        f"name: googlesqlite-{spec.plan_id}-{spec.slug}",
        f"overview: {yaml_quote(spec.goal)}",
        "todos:",
        "  - id: " + spec.todo_id,
        "    content: " + yaml_quote(spec.todo_content),
        "    status: pending",
        "isProject: false",
        "---",
        "",
        f"# googlesqlite {spec.plan_id}: {spec.title}",
        "",
        "## Goal",
        "",
        spec.goal,
        "",
        f"Baseline: `{baseline_stamp}` ({test_count} failing tests in this bucket).",
        "",
    ]

    if spec.dependencies:
        lines += ["## Dependencies", ""]
        for dep in spec.dependencies:
            lines.append(f"- [`{dep}`]({dep})")
        lines.append("")

    if spec.blocked_by_notes:
        lines += ["## Notes", "", spec.blocked_by_notes, ""]

    lines += ["## Root cause", ""]
    if root_causes:
        for rc in root_causes:
            lines.append(f"- `{rc}`")
    else:
        lines.append("- (see failing test output in baseline JSON log)")
    lines.append("")

    lines += ["## Primary files", ""]
    for f in spec.primary_files:
        lines.append(f"- [`{f}`](../../{f})")
    lines.append("")

    lines += ["## Implementation steps", ""]
    for i, step in enumerate(spec.implementation_steps, 1):
        lines.append(f"{i}. {step}")
    lines.append("")

    lines += [
        "## Verify",
        "",
        "```bash",
        "BIGQUERY_EMULATOR_BIN=./bin/emulator_main \\",
        "  go test -tags=integration ./gateway/e2e/ \\",
        f"  -run '{run_regex}' \\",
        "  -count=1 -parallel 1",
        "```",
        "",
        "## Done when",
        "",
        f"- All {test_count} tests listed below pass.",
        f"- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~{test_count}.",
        "",
        "## Failing tests",
        "",
    ]
    for t in sorted(tests):
        lines.append(f"- `{t}`")
    lines.append("")
    return "\n".join(lines)


def render_index(
    buckets: dict[str, list[str]],
    baseline_stamp: str,
    pass_count: int,
    fail_count: int,
    skip_count: int,
) -> str:
    order = [f"{i:02d}" for i in range(1, 17)]
    lines = [
        "---",
        "name: googlesqlite-00-index",
        'overview: "Sequential index for fixing 568 googlesqlite emulator port failures (baseline 20260603T035812Z)"',
        "todos:",
    ]
    for pid in order:
        spec = PLAN_SPECS[pid]
        n = len(buckets.get(pid, []))
        lines += [
            f"  - id: gsql-index-{pid}",
            f"    content: {yaml_quote(f'Execute googlesqlite-{pid}-{spec.slug}.plan.md ({n} tests)')}",
            "    status: pending",
        ]
    lines += ["isProject: false", "---", ""]
    lines += [
        "# googlesqlite Emulator Conformance Index",
        "",
        "## Goal",
        "",
        "Drive the ported googlesqlite query suite to green by executing plans "
        "01→16 in order. Each plan owns a failure theme from the baseline run.",
        "",
        "## Baseline",
        "",
        f"- Stamp: `{baseline_stamp}`",
        f"- pass={pass_count} fail={fail_count} skip={skip_count}",
        "- Summary: [`gateway/e2e/testresults/googlesqlite-emulator-20260603T035812Z-summary.txt`](../../gateway/e2e/testresults/googlesqlite-emulator-20260603T035812Z-summary.txt)",
        "- Re-run: [`gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh`](../../gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh)",
        "",
        "## Execution order",
        "",
    ]
    for pid in order:
        spec = PLAN_SPECS[pid]
        n = len(buckets.get(pid, []))
        fname = f"googlesqlite-{pid}-{spec.slug}.plan.md"
        lines.append(
            f"{int(pid)}. [`{fname}`]({fname}) — {spec.title} (~{n} tests)"
        )
    lines += [
        "",
        "## Global verify",
        "",
        "```bash",
        "./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh",
        "```",
        "",
        "## Regenerate plan test lists",
        "",
        "```bash",
        "python3 tools/googlesqlite/plan_from_testresults.py \\",
        "  --json gateway/e2e/testresults/googlesqlite-emulator-latest.json \\",
        "  --emit-plans",
        "```",
        "",
        "## Done when",
        "",
        "- Summary shows `fail=0` (or only intentional skips: "
        "`TestFilterFieldsProto`, `TestAnonymizedDPAggregate`).",
        "",
    ]
    return "\n".join(lines)


FORCE_BUCKET: dict[str, list[str]] = {
    "03": [
        "TestQualifyClause",
        "TestComputedColumnsInProject",
        "TestRecursiveCTE",
        "TestWithRecursiveSelfReferential",
    ],
}


def apply_force_buckets(
    buckets: dict[str, list[str]], failed: list[str]
) -> dict[str, list[str]]:
    failed_set = set(failed)
    for pid, names in FORCE_BUCKET.items():
        for name in names:
            if name not in failed_set:
                continue
            for other in buckets:
                if name in buckets[other]:
                    buckets[other].remove(name)
            if name not in buckets[pid]:
                buckets[pid].append(name)
        buckets[pid] = sorted(set(buckets[pid]))
    return buckets


def classify_all(
    failed: list[str], outputs: dict[str, str]
) -> dict[str, list[str]]:
    buckets: dict[str, list[str]] = {f"{i:02d}": [] for i in range(1, 17)}
    for test in failed:
        text = outputs.get(test, "")
        bucket = classify_test(test, text)
        buckets[bucket].append(test)
    for k in buckets:
        buckets[k] = sorted(set(buckets[k]))
    return apply_force_buckets(buckets, failed)


def parse_summary_counts(summary_path: Path) -> tuple[int, int, int]:
    line = summary_path.read_text(encoding="utf-8").splitlines()[0]
    m = re.match(r"pass=(\d+) fail=(\d+) skip=(\d+)", line)
    if not m:
        return 0, 568, 2
    return int(m.group(1)), int(m.group(2)), int(m.group(3))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--json",
        type=Path,
        default=DEFAULT_JSON,
        help="Path to go test -json JSONL file",
    )
    parser.add_argument(
        "--stamp",
        default="20260603T035812Z",
        help="Baseline run stamp for plan headers",
    )
    parser.add_argument(
        "--emit-plans",
        action="store_true",
        help="Write .cursor/plans/googlesqlite-*.plan.md files",
    )
    parser.add_argument(
        "--plan",
        help="Emit markdown appendix for a single plan id (01-16)",
    )
    parser.add_argument(
        "--summary",
        action="store_true",
        help="Print bucket counts to stdout",
    )
    args = parser.parse_args()

    failed, outputs = load_results(args.json)
    buckets = classify_all(failed, outputs)

    if args.summary:
        total = 0
        for pid in sorted(buckets):
            n = len(buckets[pid])
            total += n
            print(f"{n:4d}  {pid}  {PLAN_SPECS[pid].slug}")
        print(f"total classified: {total} / failed: {len(failed)}")

    if args.plan:
        pid = args.plan.zfill(2)
        tests = buckets.get(pid, [])
        print(render_plan_markdown(PLAN_SPECS[pid], tests, outputs, args.stamp))

    if args.emit_plans:
        PLANS_DIR.mkdir(parents=True, exist_ok=True)
        summary_path = args.json.parent / args.json.name.replace(
            ".json", "-summary.txt"
        )
        if not summary_path.exists():
            summary_path = (
                REPO_ROOT
                / "gateway/e2e/testresults/googlesqlite-emulator-20260603T035812Z-summary.txt"
            )
        pass_c, fail_c, skip_c = parse_summary_counts(summary_path)

        index_path = PLANS_DIR / "googlesqlite-00-index.plan.md"
        index_path.write_text(
            render_index(buckets, args.stamp, pass_c, fail_c, skip_c),
            encoding="utf-8",
        )
        print(f"wrote {index_path}")

        for pid, spec in PLAN_SPECS.items():
            tests = buckets.get(pid, [])
            out_path = PLANS_DIR / f"googlesqlite-{pid}-{spec.slug}.plan.md"
            out_path.write_text(
                render_plan_markdown(spec, tests, outputs, args.stamp),
                encoding="utf-8",
            )
            print(f"wrote {out_path} ({len(tests)} tests)")


if __name__ == "__main__":
    main()
