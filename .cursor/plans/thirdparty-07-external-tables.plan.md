---
name: Thirdparty 07 — External tables
overview: Model externalDataConfiguration and enable queries against GCS-backed external tables (Sheets deferred or stubbed).
depends_on: [thirdparty-04-tp08-load-jobs]
blocks: []
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 1-2 weeks
isProject: true
todos:
  - id: external-wire-model
    content: Add externalDataConfiguration to bqtypes.Table; persist on tables.insert/patch/get
    status: pending
  - id: gcs-external-scan
    content: Query engine path for external GCS tables (DuckDB read_csv/read_parquet over fake-gcs URLs)
    status: pending
  - id: permanent-temp-tables
    content: Support permanent vs temporary external table patterns from samples
    status: pending
  - id: sheets-stub
    content: Google Sheets external tables — 501 with clear message or mock if feasible
    status: pending
  - id: external-tests
    content: Green python test_query_external_gcs_* and node external source query tests
    status: pending
---

# Thirdparty 07 — External tables

## Goal

Enable querying data **without loading** into native tables — external `gs://` sources.

## Baseline failures

| Suite | Tests |
|-------|-------|
| Python | `test_query_external_gcs_temporary_table`, `test_query_external_sheets_permanent_table`, `test_query_external_sheets_temporary_table` |
| Node | `should query an external data source with permanent table`, `should query an external data source with temporary table` (items 27–28) |

## Wire model

Add `ExternalDataConfiguration` to [`gateway/bqtypes`](../../gateway/bqtypes):

- `sourceUris[]`
- `sourceFormat` — `CSV`, `NEWLINE_DELIMITED_JSON`, `PARQUET`, etc.
- `autodetect`, `schema`, `csvOptions`, `googleSheetsOptions`

Persist when clients call `tables.insert` with `externalDataConfiguration` set (table `type` may be `EXTERNAL` or `TABLE` with external config).

## Query path

When engine resolves a table reference with external config:

1. Map `gs://` URI → fake-gcs HTTP URL (same as plan 04)
2. Register ephemeral DuckDB external scan or transpile to `read_csv_auto('http://...')`
3. Return query results through normal `jobs.query` path

**Files:**
- [`gateway/handlers/tables.go`](../../gateway/handlers/tables.go) — accept external config on insert
- Engine: [`backend/engine/duckdb/`](../../backend/engine/duckdb/) or coordinator — external table registration in catalog
- May need catalog adapter changes in [`backend/engine/catalog/`](../../backend/engine/catalog/)

## Google Sheets

Upstream samples use Sheets API + `googleSheetsOptions`. Options:

- **Defer:** Return structured 501; add skip in thirdparty only if full parity accepts partial Sheets (conflicts with user goal — prefer minimal mock)
- **Mock:** Static CSV export URL stand-in for sheets fixture

Document decision in PR.

## Dependencies

- fake-gcs seeded (`task testdata:fake-gcs-sync`)
- GCS URI fetch logic from plan 04 can be reused

## Verification

```bash
task thirdparty:fake-gcs-up
task thirdparty:python-bigquery-tests
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_query_external_gcs_temporary_table.py -v' task thirdparty:python-bigquery-tests
task thirdparty:node-bigquery-tests
```

## Out of scope

- BigLake / AWS Glue external catalogs
- Authenticated real Google Sheets API

## Done when

- [ ] GCS external table query tests pass
- [ ] Sheets tests pass or have documented emulator stub with conformance coverage
- [ ] `docs/ENGINE_POLICY.md` updates `LOAD DATA gs://` vs external table distinction
