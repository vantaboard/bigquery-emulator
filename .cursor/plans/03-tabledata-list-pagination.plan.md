---
name: tabledata.list pagination
overview: Native GET /tables/{id}/data pagination works for Preview; harden edge cases and BQ parity fields.
todos:
  - id: e2e-multi-page
    content: gateway/e2e — insert 5 rows, paginate with maxResults=2 and pageToken
    status: pending
  - id: selected-fields-format
    content: Optional — selectedFields, formatOptions parity if UI needs them
    status: pending
  - id: view-preview-path
    content: Document or implement view preview via tabledata.list vs query fallback
    status: pending
isProject: false
---

# 03 — tabledata.list (table data preview) pagination & parity

- **UI gap:** #4 (priority **P3**)
- **Verified state at HEAD (`60d19b3e`):** **Pass** — pagination verified with 5-row table.

## Current state at HEAD (grounded)

Endpoint `GET .../tables/{t}/data` is registered (`gateway/server.go` ~116;
dual-mounted ~65–68), discovery-documented
(`gateway/handlers/discovery_methods.go` ~146–153), and marked `done` in
`docs/REST_API.md` (~85).

Handler chain: `TableDataList` → `tableDataListPaging` → `buildTableDataList`
(`gateway/handlers/tabledata.go` ~286–372) → `Catalog.DescribeTable` +
`Catalog.ListRows` → `frontend/handlers/catalog.cc` `ListRows` →
`DuckDBStorage::ScanRows` (Parquet on disk).

| Capability | State | Notes |
|------------|-------|-------|
| `maxResults` | ✅ | default **10000** (`tableDataListDefaultMaxResults`) |
| `pageToken` | ✅ | plain decimal next-row-index; overrides `startIndex` |
| `startIndex` | ✅ | forwarded to `ListRowsRequest.StartIndex` |
| `rows` f/v cells | ✅ | `CellsToRowForSchema` / `ValueToCell` (`gateway/bqtypes/wire.go`) |
| `totalRows` | ✅ | decimal string |
| `kind` | ✅ | `bigquery#tableDataList` |
| `etag` | ❌ | field exists on struct, never set |
| `selectedFields` | ❌ | parsed-but-ignored comment; no code |
| `formatOptions.useInt64Timestamp` | ❌ | TIMESTAMP currently always decimal micros (`wire.go` ~125–136) |
| views preview | ❌ | `ScanRows` returns empty for views (no Parquet) |
| external tables | ❌ | reads Parquet only |
| large-table perf | ⚠️ | each page does a full Parquet scan to compute `total_rows` (O(n)/page) |

## Goal / done-criteria (UI-observable)

1. Multi-page round-trip: with N > pageSize rows, chaining `pageToken` returns all
   rows exactly once with stable `totalRows`. (Primary missing test.)
2. A UI-friendly default page size (or the UI always sends `maxResults`); document
   `maxResults=0` semantics and add an upper bound cap.
3. `selectedFields` projects the returned cells.
4. `formatOptions.useInt64Timestamp` honored for TIMESTAMP encoding.
5. `etag` populated (stable per table version) so the UI can cache.
6. Views: either preview by executing `view.query` with paging, or return a clear,
   documented response the UI can fall back from (today it falls back to a LIMIT
   query — acceptable, but document it).

## Implementation steps

### Step 1 — Multi-page e2e test (do this first)

Add to `gateway/e2e` (mirror `catalog_test.go` ~140–162): seed > `maxResults`
rows, page with `maxResults=K`, follow `pageToken` until empty, assert union ==
all rows and `totalRows` constant. This locks current behavior before changes.

### Step 2 — Default page size + caps

In `tableDataListPaging` (`tabledata.go` ~322–336): keep `maxResults` honored;
add a documented maximum cap; clarify/align `maxResults=0` with
`jobs.getQueryResults` semantics. Coordinate the default with the UI (the UI can
also just always send `maxResults`).

### Step 3 — `selectedFields`

Parse the `selectedFields` query param (comma-separated, dotted for nested) in
the handler and project columns. Implement either in the gateway after
`ListRows`, or push down to the engine (`ListRowsRequest`) / Storage Read
(`StorageRead.CreateReadSession` supports `selected_fields`). Gateway-side
projection is the simplest first cut.

### Step 4 — `formatOptions.useInt64Timestamp`

Thread `DataFormatOptions` from the request into `encodeCellForField`
(`gateway/bqtypes/wire.go`) so TIMESTAMP encoding follows the flag instead of
always emitting decimal micros.

### Step 5 — `etag`

Populate the `etag` field on the response from a cheap table version signal
(row count + schema hash, or table metadata mtime). Wire type already has the
field (`gateway/bqtypes/tabledata_wire_types.go` ~62).

### Step 6 — Views (optional, coordinate with plan 01)

Once views are listable (plan 01), make preview work by executing `view.query`
through the query path and paginating the result, or explicitly document that
the UI should use its LIMIT-query fallback for views.

### Step 7 — Large-table performance (optional)

Avoid a full scan for `total_rows` on every page: add an engine `CountRows` (or
parquet row-group offset metadata) and offset-limited reads, or route large
reads through the Storage Read path.

## Tests

- e2e multi-page pagination (Step 1).
- unit (`tabledata_test.go`): default `maxResults`, `maxResults=0`, malformed
  `pageToken` → 400 (only `startIndex` is covered today), `selectedFields`
  projection, `formatOptions.useInt64Timestamp`, `etag` present.

## Out of scope

- Full Storage Read REST proxy.
- External-table data reads.

## Touch list

`gateway/handlers/tabledata.go`, `gateway/bqtypes/wire.go`,
`gateway/bqtypes/tabledata_wire_types.go`, `gateway/e2e/`,
optionally `frontend/handlers/catalog.cc` (engine `CountRows` / offset pushdown).
