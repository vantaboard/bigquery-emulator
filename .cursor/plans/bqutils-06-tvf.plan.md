---
name: BQUtils 06 — Table-valued functions (TVF)
overview: Implement CREATE TABLE FUNCTION (TVF) support in the engine so any bigquery-utils table-function patterns can be exercised. Currently UNIMPLEMENTED in the control-op executor.
depends_on: [bqutils-02-engine-build-triage]
blocks: [bqutils-09-promote-to-gate]
est_effort: 1-2 weeks
isProject: true
todos:
  - id: scope-check
    content: Audit bigquery-utils (and broader corpus) for actual CREATE TABLE FUNCTION usage; confirm how many fixtures this unblocks before investing.
    status: pending
  - id: control-op
    content: Replace the UNIMPLEMENTED stub for RESOLVED_CREATE_TABLE_FUNCTION_STMT in control_op_executor.cc with registration; persist via udf_registry analog.
    status: pending
  - id: call-path
    content: Wire TVF resolution + execution at query time (FROM tvf(args)) through the analyzer catalog + executor.
    status: pending
  - id: fixtures
    content: Add first-party conformance fixtures for a simple TVF (CREATE TABLE FUNCTION ... AS SELECT ...; SELECT * FROM tvf(...)); re-sync + re-triage.
    status: pending
  - id: docs
    content: Update docs/ENGINE_POLICY.md / ROADMAP.md / SHAPE_TRACKER.md for TVF support.
    status: pending
---

# BQUtils 06 — Table-valued functions (TVF)

## Why

`CREATE TABLE FUNCTION` is `UNIMPLEMENTED` in `backend/engine/control/control_op_executor.cc:144-147`. bigquery-utils is mostly scalar UDFs, so **scope-check first** (todo) — this plan may unblock few fixtures and could be deprioritized relative to 03/05. Kept here so the out-of-scope surface is fully tracked.

## Current engine reality

- `backend/engine/control/control_op_executor.cc:144-147` — `RESOLVED_CREATE_TABLE_FUNCTION_STMT` returns `UNIMPLEMENTED`.
- No registration/replay analog for TVFs in `backend/catalog/udf_registry.cc`.

## Steps

1. **Scope-check**: grep bigquery-utils for `CREATE TABLE FUNCTION` / TVF patterns; quantify the payoff. If negligible, record the finding and stop (leave as a tracked gap).
2. **Register**: implement registration on `RESOLVED_CREATE_TABLE_FUNCTION_STMT` (mirror the scalar UDF registration in `local_coordinator_engine.cc` + `udf_registry`).
3. **Call path**: resolve + execute `FROM tvf(args)` through the analyzer catalog and executor.
4. **Fixtures**: add a minimal TVF fixture under `conformance/fixtures/`; re-sync + re-triage if codegen emits TVF fixtures.
5. **Docs**.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/fixtures/<new_tvf>.yaml
```

## Out of scope

- ANY TYPE (plan 03), JS (plan 04), UDAF (plan 05).
