---
name: Unblock 03 — BigFrames snippet gate
overview: Green all 4 python-bigquery-dataframes-snippet-gate tests — sessionInfo on job statistics and OrderByScan transpiler for GROUP BY+ORDER BY aggregates.
depends_on: [unblock-02-public-data-seed]
blocks: [unblock-10-final-aggregator]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: 2-3 days
isProject: true
todos:
  - id: session-store
    content: gateway/session/store.go — mint sessionId on createSession; honor connectionProperties session_id
    status: pending
  - id: session-wire
    content: statistics.sessionInfo on jobs.insert/query responses in handlers + jobs/registry.go
    status: pending
  - id: session-tests
    content: Handler unit tests for createSession + follow-up connectionProperties
    status: pending
  - id: orderby-fix
    content: Fix OrderByScan transpiler for post-aggregate ORDER BY (GROUP BY species + AVG + sort + LIMIT)
    status: pending
  - id: orderby-conformance
    content: Add conformance fastpath fixture; scoped bazel transpiler test
    status: pending
  - id: gate-green
    content: task thirdparty:python-bigquery-dataframes-snippet-gate exits 0 (4 pass / 28 deselected)
    status: pending
  - id: status-commit
    content: Update orchestration-status.md; lint + commit (Go + C++ if engine touched)
    status: pending
---

# Unblock 03 — BigFrames snippet gate

## Goal

Pass all **4** tests in `task thirdparty:python-bigquery-dataframes-snippet-gate`. Current: **2 pass / 2 fail**.

Extends partial work from [thirdparty-11-bigframes-gate.plan.md](thirdparty-11-bigframes-gate.plan.md) (commit `f1f41cc`).

## Remaining failures (from `.logs/thirdparty-20260605-134407.log`)

| Test | Error |
|------|-------|
| `test_bigquery_dataframes_pandas_methods` | `501 ... family: node:OrderByScan, route: duckdb_native` |
| `test_performance_optimizations` | `AssertionError` at `query_job.session_info is not None` during `df.peek(3)` cache path |

Already green: `test_bigquery_dataframes_set_options`, `test_bigquery_dataframes_load_data_from_bigquery`.

## Part A — sessionInfo

### Failure chain

`df.peek(3)` → temp table cache → `ConnectionProperty("session_id", ...)` → [`bigquery_session.py`](../../third_party/python-bigquery-dataframes-tests/bigframes/session/bigquery_session.py) `_get_session_id()` asserts `query_job.session_info is not None`.

Python client reads **`statistics.sessionInfo`** on the Job resource ([`job/base.py`](../../third_party/python-bigquery-tests/google/cloud/bigquery/job/base.py)), not only top-level query response fields.

### Implementation

1. Add `SessionInfo *bqtypes.SessionInfo` to [`gateway/jobs/registry.go`](../../gateway/jobs/registry.go) `Statistics` (`json:"sessionInfo,omitempty"`)
2. New [`gateway/session/store.go`](../../gateway/session/store.go): in-memory `sessionId` registry
3. Parse `configuration.query.createSession` in job config (mirror [`gateway/bqtypes/types.go`](../../gateway/bqtypes/types.go) `QueryRequest.CreateSession`)
4. Thread through [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) `runSyncQueryInsert` / `finalizeDoneJob`
5. Honor `connectionProperties` with `key == "session_id"` on subsequent queries in [`gateway/handlers/queries.go`](../../gateway/handlers/queries.go)
6. `ABORT_SESSION` / TTL can stub (no-op) for snippet gate

### Tests

```bash
go test ./gateway/handlers/... -count=1 -run 'Session|CreateSession'
```

## Part B — OrderByScan transpiler

### Failing query shape

From [`pandas_methods_test.py`](../../third_party/python-bigquery-dataframes-tests/samples/snippets/pandas_methods_test.py):

```python
bq_df["body_mass_g"].groupby(by=bq_df["species"]).mean().sort_values(ascending=False).head(10)
```

Compiles to **GROUP BY + AVG + ORDER BY aggregate output + LIMIT**.

### Code paths

- Emitter: [`backend/engine/duckdb/transpiler/transpiler_emit_setops.cc`](../../backend/engine/duckdb/transpiler/transpiler_emit_setops.cc) `EmitOrderByScan`
- Empty return → [`duckdb_executor_query.cc`](../../backend/engine/duckdb/duckdb_executor_query.cc) UNIMPLEMENTED with `family: node:OrderByScan`
- Fallback exists: [`backend/engine/semantic/scan_eval_scan_impl.cc`](../../backend/engine/semantic/scan_eval_scan_impl.cc)

### Debug approach

1. Run gate with `--tb=long`; capture `statistics.query.emulatorRoute` / compiled SQL from loopback
2. Identify whether `EmitScan(input)`, `EmitColumnRef`, or `collation_name` returns `""`
3. Fix transpiler or route COLLATE shapes to semantic executor
4. Add fixture under [`conformance/fixtures/fastpath/`](../../conformance/fixtures/fastpath/) mirroring GROUP BY + ORDER BY avg DESC + LIMIT

### Bazel gate

```bash
task bazel:test -- //backend/engine/duckdb/transpiler/...
```

Follow [bazel-process-hygiene](../rules/bazel-process-hygiene.mdc): one invocation; parent cleanup after subagent.

## Final gate

```bash
task thirdparty:python-bigquery-dataframes-snippet-gate
# expect: 4 passed, 28 deselected
```

## Out of scope

- Full bigframes system suite (only 4-test gate)
- Storage Read gRPC for bigframes (plan 08 if needed later)

## Docs

Update [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) if OrderByScan disposition changes.

## Done criteria

- [ ] `test_performance_optimizations` passes
- [ ] `test_bigquery_dataframes_pandas_methods` passes
- [ ] Snippet gate exits 0
