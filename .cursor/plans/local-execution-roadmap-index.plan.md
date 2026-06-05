---
name: local-execution-roadmap-index
overview: "Route vocabulary and foundation plans; active googlesqlite conformance work lives in googlesqlite-00-index.plan.md"
todos:
  - id: roadmap-googlesqlite-index
    content: "Execute plans in googlesqlite-00-index.plan.md (01→16) for the ported query suite"
    status: pending
isProject: false
---

# Local Execution Roadmap Index

## Goal

Own the **route vocabulary**, foundation prerequisites, and done criteria
for the local multi-strategy execution coordinator behind
`backend/engine/engine.h`.

**Active conformance work** for the ported googlesqlite query suite is
tracked separately — start there:

- [`googlesqlite-00-index.plan.md`](googlesqlite-00-index.plan.md) —
  sequential plans 01→16 derived from the baseline emulator test run
  (`pass=127 fail=568 skip=2` at stamp `20260603T035812Z`).
- [`googlesqlite-subagent-dispatch.plan.md`](googlesqlite-subagent-dispatch.plan.md) —
  how to run those plans **one subagent at a time** (parent cleanup,
  verify gates, index todo updates).

This file remains the terminology anchor referenced by
[`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) and
[`ROADMAP.md`](../../ROADMAP.md).

## Terminology

| Route | Meaning |
|-------|---------|
| `duckdb_native` | Lowers to DuckDB SQL with matching BigQuery semantics. |
| `duckdb_rewrite` | Lowers via a deliberate structural rewrite. |
| `duckdb_udf` | Lowers via a registered DuckDB UDF/macro. |
| `semantic_executor` | Local row/value interpreter; DuckDB is the row source. |
| `control_op` | DDL/metadata through the storage layer. |
| `local_stub` | Deterministic BigQuery-shaped placeholder for specialized families. |
| `unsupported` | Deliberate `UNIMPLEMENTED` with family + plan link. |

## Foundation plans (prerequisites)

Execute before or in parallel with googlesqlite plans when registry/router
gaps block progress:

1. [`execution-disposition-registry.plan.md`](execution-disposition-registry.plan.md)
2. [`engine-router-foundation.plan.md`](engine-router-foundation.plan.md)

Supporting plans (not part of the googlesqlite failure sweep):

- [`conformance-routing-matrix.plan.md`](conformance-routing-matrix.plan.md)
- [`storage-read-write-api-plan.plan.md`](storage-read-write-api-plan.plan.md)
- [`migration-cleanup-docs.plan.md`](migration-cleanup-docs.plan.md)

## Shared rules

- **One route per shape** — no silent runtime fallback between routes.
- **Disposition + emit + conformance land together** — a row flips only
  when implementation and fixtures land in the same change.
- **Compositional promotion at planning time** — mixed-strategy queries
  promote to the highest-priority child route before execution.
- **`unsupported` posture** — see
  [`googlesqlite-15-specialized-stubs.plan.md`](googlesqlite-15-specialized-stubs.plan.md).

## Done criteria (engine-wide)

- Every `SHAPE_TRACKER.md` row has a landed route or an explicit
  `(planned)` row with a googlesqlite plan owner.
- Every `functions.yaml` row has a non-`kFallback` disposition.
- googlesqlite suite: `fail=0` (except intentional skips) per
  [`googlesqlite-00-index.plan.md`](googlesqlite-00-index.plan.md).

## Regenerate googlesqlite plan test lists

```bash
python3 tools/googlesqlite/plan_from_testresults.py \
  --json gateway/e2e/testresults/googlesqlite-emulator-latest.json \
  --emit-plans
```
