---
name: Full 03 — Time travel, decorators, snapshots
overview: Add BigQuery's historical-data surface - `FOR SYSTEM_TIME AS OF`, table decorators (@<time>/@<offset>), table snapshots (CREATE SNAPSHOT TABLE), and UNDROP/restore - on top of a lightweight per-table version log in DuckDBStorage. Where true history cannot be reconstructed, return the current snapshot for in-window timestamps and a BigQuery-shaped error for out-of-window ones rather than silently lying.
est_effort: ~2 weeks
isProject: true
todos:
  - id: version-log
    content: "Design a best-effort version/change log in backend/storage/duckdb (append-only mutation timestamps per table, bounded to the 7-day BigQuery window) sufficient to answer FOR SYSTEM_TIME AS OF for recent timestamps; document the durability caveat (per ROADMAP persistence non-goal)."
    status: pending
  - id: system-time-scan
    content: "FOR SYSTEM_TIME AS OF <expr> on ResolvedTableScan: thread the as-of timestamp from the analyzer (for_system_time_expr) through the scan so the engine reads the table state at/just-before that time; out-of-window timestamps return BigQuery's `Invalid snapshot time` error."
    status: pending
  - id: table-decorators
    content: "Table decorators `table@<millis>` / `table@<offset>` parsed at the gateway/analyzer boundary and lowered to the same as-of scan path; reject decorator + FOR SYSTEM_TIME combinations the way BigQuery does."
    status: pending
  - id: snapshots-clones
    content: "CREATE SNAPSHOT TABLE (and CREATE TABLE ... CLONE) routed to control_op: materialize a point-in-time copy through Storage; record snapshot base table + timestamp metadata for jobs/REST."
    status: pending
  - id: undrop-restore
    content: "UNDROP TABLE / dataset undelete-of-table-state: keep dropped table rows recoverable within the time-travel window; wire the DDL through ControlOpExecutor + storage soft-delete."
    status: pending
  - id: fixtures-trackers
    content: "conformance/fixtures/time_travel/ fixtures (as-of select, decorator, snapshot round-trip, out-of-window error); add node_dispositions rows for any new statement shapes; update SHAPE_TRACKER + ROADMAP + ENGINE_POLICY; cross-check docs/bigquery/docs/time-travel.md + table-decorators.md."
    status: pending
---

# Full 03 — Time travel, decorators, snapshots

## Why

`FOR SYSTEM_TIME AS OF`, `table@<time>` decorators, table snapshots, and
`UNDROP` are first-class BigQuery features that appear in migration
workloads and recovery runbooks. They currently surface `UNIMPLEMENTED`.
Reference docs already vendored: `docs/bigquery/docs/time-travel.md`,
`access-historical-data.md`, `table-decorators.md`,
`table-snapshots-create.md`, `restore-deleted-tables.md`.

## Approach

The emulator's persistence is best-effort (ROADMAP non-goal), so we do
**not** promise full 7-day history. The honest design is:

- Keep a bounded per-table mutation log so recent as-of timestamps can
  be answered (at minimum: the current state for any in-window
  timestamp once a version exists).
- Return BigQuery's documented out-of-window error for timestamps the
  log can't satisfy — never a silently-wrong historical answer.

## Key files

- [`backend/storage/duckdb/`](../../backend/storage/duckdb/) — version log + as-of read
- [`backend/engine/control/control_op_executor.{h,cc}`](../../backend/engine/control/) — SNAPSHOT/CLONE/UNDROP DDL
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — route the as-of scan / new statements
- gateway query/analyzer boundary — decorator parsing
- `docs/bigquery/docs/time-travel.md`, `table-decorators.md`, `table-snapshots-create.md`

## Steps

1. Land the version log + as-of read primitive in storage first.
2. Wire `FOR SYSTEM_TIME AS OF` (analyzer exposes the as-of expr on the
   table scan) → as-of read.
3. Add decorator parsing that lowers to the same path.
4. Add `CREATE SNAPSHOT TABLE` / `CLONE` / `UNDROP` as control ops.
5. Fixtures (including the out-of-window error) + tracker/doc updates.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Cross-region / cross-project snapshot semantics.
- Guaranteed 7-day retention with byte-accurate accounting (best-effort
  window only).
