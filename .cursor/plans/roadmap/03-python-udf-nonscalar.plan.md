---
name: Non-scalar Python UDFs
overview: Extend the landed scalar Python UDF runtime to aggregate (UDAF) and table-valued shapes, reusing the SQL UDAF/TVF evaluation scaffolding on the semantic executor.
todos:
  - id: verify-bq-surface
    content: Verify with bq dry-run exactly which non-scalar LANGUAGE python forms production BigQuery accepts (aggregate? TVF?) before building either
    status: pending
  - id: python-udaf
    content: Register + evaluate aggregate Python UDFs (batch rows to the subprocess, one result per group) if the surface is real
    status: pending
  - id: python-tvf
    content: Register + evaluate table-valued Python UDFs (rows-out protocol) if the surface is real
    status: pending
  - id: fixtures-docs
    content: Conformance fixtures + ENGINE_POLICY / ROADMAP row updates (or sharpened rejects if bq rejects the forms)
    status: pending
isProject: false
---

# 03 — Table-valued / aggregate Python UDFs (real)

- **Roadmap row:** §Python UDFs — "⏳ Table-valued / aggregate Python UDF
  shapes remain unsupported"
- **Policy row:** `docs/ENGINE_POLICY.md` ~110 (Python UDFs `local_impl`,
  non-scalar unsupported)

## Current state at HEAD (grounded)

- Scalar path is landed end-to-end: `backend/catalog/python_udf_registry.cc`
  registers `CREATE FUNCTION ... LANGUAGE python`, persists DDL in
  `DuckDBStorage` (`__bqemu_routines`), rehydrates on restart
  (`backend/engine/coordinator/routine_rehydrate.cc`), and evaluates at call
  time via a sandboxed `python3` subprocess
  (`backend/engine/semantic/python_udf_runtime.{h,cc}`, dispatched from
  `eval_expr_calls.cc`). Pinned by
  `conformance/fixtures/udf/python_scalar_add.yaml`.
- SQL UDAFs and SQL TVFs already evaluate on the semantic executor (ROADMAP
  §Scripting/UDFs/TVFs), so the group-wise and relation-materialization
  scaffolding this plan needs exists; only the Python bridge is missing.
- The JS lane (`js_udf_registry.cc` + Duktape runtime) is scalar-only too and
  stays that way — this plan is Python-only per the roadmap.

## Step 0 gate — verify the production surface first

Per `.cursor/rules/conformance-bq-validation.mdc` posture: before
implementing, confirm with `bq query --dry_run` which forms production
BigQuery actually accepts. As of the last check, BigQuery's Python UDF GA
surface is **scalar-only**; if `CREATE AGGREGATE FUNCTION ... LANGUAGE python`
and table-valued Python forms are rejected upstream, the correct landing for
this roadmap row is a **sharpened reject** (analyzer-time error naming the
unsupported form + ENGINE_POLICY row), not an implementation — mirroring how
`ResolvedSequence` was closed. Record the bq transcript in the commit.

## Done-criteria (if the surface is real)

1. `CREATE AGGREGATE FUNCTION f(x FLOAT64) RETURNS FLOAT64 LANGUAGE python`
   registers, persists, rehydrates, and evaluates per group (including
   `GROUP BY` and empty-group NULL semantics).
2. Table-valued Python UDF registers and its `ResolvedTVFScan` materializes
   rows from the subprocess.
3. Both shapes survive engine restart (routine persistence proof).
4. `SAFE.` prefix behavior matches the scalar lane.

## Implementation steps

### Step 1 — registry + DDL classification

Extend `python_udf_registry.cc` (and the `CREATE AGGREGATE FUNCTION` /
`CREATE TABLE FUNCTION` control-op paths) to accept `LANGUAGE python`,
storing the aggregate/TVF flavor in the persisted routine row so
`routine_rehydrate.cc` restores the right kind.

### Step 2 — subprocess protocol extension

`python_udf_runtime.cc` today evaluates one scalar call per invocation.
Extend the protocol: aggregate mode sends the full group's argument rows and
expects one value; TVF mode sends bound arguments and expects a row stream
(JSON-lines). Keep the sandbox posture (no network, `BIGQUERY_EMULATOR_PYTHON`
override honored) identical to the scalar lane.

### Step 3 — semantic executor dispatch

Wire the aggregate flavor into the UDAF evaluation path (where SQL UDAFs
dispatch) and the TVF flavor into the TVF materialization path (next to
`ResolvedRelationArgumentScan` handling). Per-group batching for UDAFs should
reuse the grouping machinery in `scan_eval_aggregate.cc` rather than
re-grouping.

## Tests

- Conformance: `conformance/fixtures/udf/python_udaf_weighted_avg.yaml`,
  `python_tvf_generate_rows.yaml` (or `*_rejected.yaml` fixtures pinning the
  sharpened envelope if Step 0 says reject).
- `gateway/e2e/routine_persistence_test.go`: add non-scalar Python rows to
  the restart matrix.
- Unit: protocol round-trip tests next to the existing runtime tests.

## Out of scope

- Non-scalar **JS** UDFs (stay unsupported; roadmap says so explicitly).
- `packages` installs — [`04-python-udf-packages.plan.md`](04-python-udf-packages.plan.md).
- Vectorized/Arrow batching for performance (correctness first; the semantic
  executor's "fast enough for tests" bar applies).

## Touch list

`backend/catalog/python_udf_registry.cc`, `backend/catalog/udf_registry.cc`,
`backend/engine/semantic/python_udf_runtime.{h,cc}`,
`backend/engine/semantic/eval_expr_calls.cc` + UDAF/TVF eval paths,
`backend/engine/coordinator/routine_rehydrate.cc`,
`conformance/fixtures/udf/`, `docs/ENGINE_POLICY.md`, `ROADMAP.md`.
