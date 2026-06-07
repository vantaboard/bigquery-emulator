---
name: BQUtils 05 — SQL aggregate UDFs (UDAF)
overview: Enable call-time evaluation of SQL aggregate UDFs so bigquery-utils generate_udaf_test cases (scaled_average, scaled_sum, cw_mode_*, etc.) can be generated and pass. Extend the codegen pipeline to stop dropping generate_udaf_test.
depends_on: [bqutils-02-engine-build-triage]
blocks: [bqutils-09-promote-to-gate]
est_effort: 1-2 weeks
isProject: true
todos:
  - id: repro
    content: Hand-write a minimal CREATE AGGREGATE FUNCTION fixture and confirm the current failure mode (registers metadata but call-time aggregate eval unsupported).
    status: pending
  - id: eval-path
    content: Add a SQL UDAF body evaluation path analogous to EvalSqlUdfBody in the aggregate evaluators (scan_eval_aggregate.cc / aggregate_specialized.cc) and wire route classification for aggregate SQL UDFs.
    status: pending
  - id: udaf-shape
    content: Mirror Dataform generate_udaf_test semantics (input_columns with NOT AGGREGATE markers + input_rows) in a UDAF codegen path in the extractor/generator.
    status: pending
  - id: fixtures
    content: Add first-party conformance fixtures for SQL UDAFs, then re-sync + re-triage bigquery-utils UDAF cases into passing/.
    status: pending
  - id: docs
    content: Update docs/ENGINE_POLICY.md / ROADMAP.md / SHAPE_TRACKER.md for SQL UDAF support.
    status: pending
---

# BQUtils 05 — SQL aggregate UDFs (UDAF)

## Why

`udfs/community/test_cases.js` uses `generate_udaf_test(...)` for aggregate UDFs (e.g. `scaled_average`, `scaled_sum`, `cw_mode_*`). Plan 01 drops these (`kind == udaf`) because the engine has no call-time UDAF body evaluation.

## Current engine reality (verified)

- `backend/catalog/create_function_util.cc:33-36` sets `FunctionEnums::AGGREGATE` when `is_aggregate()` — so CREATE can register metadata.
- But `backend/engine/semantic/scan_eval_aggregate.cc` (`EvalAggregateForRows`) only handles builtins; `backend/engine/semantic/aggregate_specialized.cc:431-433` ends with `"aggregate '...' is not implemented"` for unknown names.
- There is **no** SQL UDAF body evaluation analogous to `EvalSqlUdfBody`.

## UDAF test shape (from the harness)

`unit_test_utils.js:35-53` — `generate_udaf_test(udaf_name, { input_columns:[...], input_rows: "<subquery>", expected_output })`. `input_columns` entries may carry a ` NOT AGGREGATE` marker (passed as a literal arg, not aggregated). The codegen must build `SELECT udaf(<agg cols>, <non-agg literals>) FROM (<input_rows>)`.

## Steps

1. **Repro**: minimal `CREATE AGGREGATE FUNCTION` + `SELECT agg(x) FROM UNNEST([...])` fixture; confirm the "not implemented" failure.
2. **Eval path**: implement SQL UDAF body evaluation in the aggregate evaluators; wire route classification so aggregate SQL UDFs route to the semantic executor. Honor `NOT AGGREGATE` params.
3. **Codegen**: add a UDAF branch in `scripts/bigquery_utils/extract_test_cases.js` + the Go generator that mirrors `generate_udaf_test` (input_columns/NOT AGGREGATE/input_rows), emitting fixtures to `known_failing/` initially.
4. **Fixtures + triage**: add first-party UDAF fixtures under `conformance/fixtures/aggregate/`, re-sync + re-triage to migrate bigquery-utils UDAFs to `passing/`.
5. **Docs**.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/fixtures/aggregate/<new_udaf>.yaml
task conformance:bqutils-sync && task conformance:bqutils
```

## Out of scope

- ANY TYPE scalar (plan 03), JS (plan 04), TVF (plan 06).
