---
name: tabledata.list pagination
overview: "Finish `TableDataList` in `gateway/handlers/tabledata.go` against `DuckDBStorage::ScanRows` with a `pageToken`-based paginator. The handler returns 501 today when `deps.Catalog == nil` and is otherwise a stub even though `ROADMAP.md` lists this as done; this plan reconciles the mismatch."
todos:
  - id: tp06_audit
    content: "Re-read gateway/handlers/tabledata.go and the engine ScanRows surface; confirm exactly where the stub returns and what schema/error shapes BigQuery expects."
    status: pending
  - id: tp06_paginator
    content: "Wire a pageToken paginator over DuckDBStorage::ScanRows. PageToken is an opaque, stable cursor (row offset + table snapshot id) survivable across paginated calls."
    status: pending
  - id: tp06_handler
    content: "Replace the NotImplemented branch with the paginator-driven response; respect maxResults, startIndex (mutually exclusive with pageToken), and selectedFields."
    status: pending
  - id: tp06_tests
    content: "Unit-test the paginator + handler under gateway/handlers/tabledata_test.go (multi-page, pageToken roundtrip, startIndex past end, selectedFields projection)."
    status: pending
  - id: tp06_node_it
    content: "Re-run `task thirdparty:node-bigquery-tests` — `Tables > should browse table rows` goes green."
    status: pending
  - id: tp06_roadmap
    content: "Update ROADMAP.md's `tabledata.list` row so the marker matches reality (was ✅, kept ✅ but with a real implementation now)."
    status: pending
  - id: tp06_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp06` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 06 — `tabledata.list` paginator

## Source

- `not-implemented-routes-catalog.plan.md` Group 7 (now deleted).
- Catalog observation: the ROADMAP marks `tabledata.list` ✅ around
  line 313, but the handler still returns `NotImplemented` when
  `deps.Catalog == nil` and is otherwise a stub. Node test
  `Tables > should browse table rows` fails as a result.

## Prerequisites

None.

## Scope

This is a handler-side change against an engine surface that already
exists (`DuckDBStorage::ScanRows`). The plan covers:

- The `pageToken` cursor design (opaque, stable across calls).
- `maxResults` honoring.
- `startIndex` (legacy alternative to `pageToken`; mutually exclusive
  with it).
- `selectedFields` projection (project only the requested columns at
  scan time).
- Round-trip of arbitrary `Row.f.v` shapes — preserve the existing
  schema mapping the rest of the handler set uses.

Out of scope: parallel scans (those are owned by the Storage Read API
plan family, plan 17).

## Implementation

1. **Audit the existing stub.** Read
   [`gateway/handlers/tabledata.go`](../../gateway/handlers/tabledata.go)
   and the engine's `ScanRows` signature. Document the exact request
   parameters BigQuery sends and the response shape it expects (rows,
   pageToken, totalRows, schema-optional).

2. **Paginator design.** PageToken format suggestion (kept private to
   the handler): base64-encoded protobuf or JSON of
   `{tableSnapshotID, rowOffset, scanCursor}`. The token must survive
   across paginated calls without re-reading the table; either rely on
   the storage layer's existing snapshot semantics or take a snapshot
   handle at first call and re-resolve it on subsequent calls.

3. **Handler implementation.** Replace the `NotImplemented` branch:
   - Reject `startIndex` + `pageToken` combined with 400.
   - Default `maxResults` if absent (BigQuery's default is large; pick
     a value documented in code).
   - Issue `ScanRows(table, projection, cursor, maxResults)` and emit
     the resulting rows plus a fresh pageToken if more rows remain.
   - Honor `selectedFields` by passing the projection list through to
     `ScanRows`; emit the column list in the response schema when the
     caller asked for `selectedFields` (BigQuery does).

4. **Tests.** Add unit tests under
   `gateway/handlers/tabledata_test.go` for:
   - Single-page response (table smaller than maxResults).
   - Multi-page round-trip (issue token, follow it, terminate).
   - `startIndex` skipping rows; `startIndex >= totalRows` returns
     empty rows + no pageToken.
   - `selectedFields` projecting a subset.
   - Snapshot drift between paginated calls — if a write happens
     between pages, surface either a stable view or a documented
     error (do not silently skip rows).

## Tests

- Unit tests above.
- `task thirdparty:node-bigquery-tests` — `Tables > should browse
  table rows` goes green.
- `task thirdparty:python-bigquery-tests` — any `list_rows` samples
  exercising the table-scan path continue to pass (they may already
  use the synchronous-query path, but confirm).

## Done criteria

- `TableDataList` no longer returns `NotImplemented`; rows return for
  any extant table.
- pageToken round-trip works.
- ROADMAP.md's `tabledata.list` row reflects the real implementation
  state (replace any stale "✅ but stub" hint, if present).
- `thirdparty-00-completion-index.plan.md` todo `tp06` flipped to
  `completed`.
