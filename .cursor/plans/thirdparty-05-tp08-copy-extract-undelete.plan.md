---
name: Thirdparty 05 — tp08 COPY, EXTRACT, undelete
overview: Implement jobs.insert COPY and EXTRACT plus table undelete — completing tp08 beyond LOAD.
depends_on: [thirdparty-04-tp08-load-jobs]
blocks: [thirdparty-11-bigframes-gate]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 3-5 days
isProject: true
todos:
  - id: copy-job
    content: configuration.copy — metadata + row copy between tables
    status: pending
  - id: extract-job
    content: configuration.extract — write table data to GCS URIs (CSV/JSON/compressed)
    status: pending
  - id: table-undelete
    content: tables.undelete or equivalent for python test_undelete_table and node undelete sample
    status: pending
  - id: copy-extract-tests
    content: Gateway tests + python/node copy/extract/undelete thirdparty tests green
    status: pending
---

# Thirdparty 05 — tp08 COPY, EXTRACT, undelete

## Goal

Clear remaining **tp08 non-LOAD** failures from baseline log.

## Failures addressed

| Suite | Tests |
|-------|-------|
| Python | `test_copy_table_multiple_source`, `test_undelete_table` |
| Node | `should copy a table`, `copy multiple source tables`, `should extract a table to GCS CSV/JSON/compressed`, `should undelete a table` (items 36–38, 56–57, 59) |

## COPY job

### Wire shape

`configuration.copy`:
- `sourceTables[]` — `projectId.datasetId.tableId`
- `destinationTable`
- `writeDisposition` — `WRITE_EMPTY`, `WRITE_TRUNCATE`

### Implementation options

1. **Catalog + storage copy:** Clone table metadata; copy rows in storage layer
2. **SQL delegation:** `CREATE TABLE dest AS SELECT * FROM src` via engine `ExecuteQuery`

Prefer option 2 initially (reuses engine); handle multi-source as `UNION ALL` CTAS or sequential inserts.

**Files:**
- [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) — `runSyncCopyInsert`
- [`gateway/jobs/registry.go`](../../gateway/jobs/registry.go) — `JobConfigurationCopy`

### Edge case (log)

Node `copy multiple source tables` — `Already Exists: destinationTable`. Ensure `WRITE_EMPTY` vs `WRITE_TRUNCATE` honored; fresh emulator volume per suite (`THIRDPARTY_FRESH_VOLUME=1`) should prevent bleed — verify idempotent copy behavior.

## EXTRACT job

### Wire shape

`configuration.extract`:
- `sourceTable`
- `destinationUris[]` — `gs://...`
- `destinationFormat` — `CSV`, `NEWLINE_DELIMITED_JSON`
- `compression` — `GZIP` optional

### Implementation

1. Read all rows from source table (engine query or storage scan)
2. Serialize to format
3. `PUT` objects to fake-gcs via HTTP API

**Depends on:** fake-gcs running (`task thirdparty:fake-gcs-up`).

## Table undelete

Python `test_undelete_table` and node `should undelete a table` expect snapshot/time-travel undelete or copy-from-snapshot semantics.

**Investigate upstream sample** to determine exact API:
- `tables.undelete` custom method?
- Copy job from snapshot decorator?

Wire whatever the sample calls in [`third_party/python-bigquery-tests/samples/`](../../third_party/python-bigquery-tests/samples/) and [`third_party/node-bigquery-tests/`](../../third_party/node-bigquery-tests/).

May require soft-delete metadata on `tables.delete` (store tombstone + snapshot id).

## Verification

```bash
go test ./gateway/handlers/... -count=1 -run 'Copy|Extract|Undelete'

task thirdparty:python-bigquery-tests
task thirdparty:node-bigquery-tests
```

## Out of scope

- LOAD jobs (plan 04)
- Cross-project copy with different billing (node may skip)

## Done when

- [ ] Copy single + multi-source tests pass
- [ ] Extract CSV/JSON/compressed tests pass against fake-gcs
- [ ] Undelete tests pass
- [ ] `docs/REST_API.md` updated for copy/extract/undelete routes
