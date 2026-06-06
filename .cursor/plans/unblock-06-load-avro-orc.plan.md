---
name: Unblock 06 — Load jobs AVRO/ORC
overview: Retest load/copy/extract after GCS networking fix; implement AVRO/ORC parsers and improve job error surfacing for remaining Jobs bucket failures.
depends_on: [unblock-01-gcs-networking]
blocks: [unblock-09-test-isolation]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: ~1 week
isProject: true
todos:
  - id: retest-jobs-bucket
    content: "After plan 01: go test ./gateway/load/... ./gateway/handlers/... -run 'Load|JobInsert|Copy|Extract'; measure node Jobs failures"
    status: pending
  - id: avro-parser
    content: Replace AVRO stub in gateway/load/parse_columnar.go with working reader
    status: pending
  - id: orc-parser
    content: Replace ORC stub with working reader (or document DEFER with clear job error)
    status: pending
  - id: job-error-surface
    content: Propagate HTTP reason from failed jobs instead of generic client failure
    status: pending
  - id: schema-update-load
    content: Confirm schemaUpdateOptions add/relax via load job for remaining node tests
    status: pending
  - id: gate-load-suites
    content: Filtered python load_table_uri_* and node GCS load variants
    status: pending
  - id: status-commit
    content: Update orchestration-status.md; lint + commit
    status: pending
---

# Unblock 06 — Load jobs AVRO/ORC

## Goal

Drive node **Jobs** bucket (was 22/45 at baseline) toward zero after plan 01 GCS fix. Complete deferred formats from [thirdparty-04-tp08-load-jobs.plan.md](thirdparty-04-tp08-load-jobs.plan.md) Phase B–D.

## Prerequisite

**Plan 01 must be PASS** — without in-container `STORAGE_EMULATOR_HOST`, retesting load jobs wastes time.

## Retest first (mandatory)

```bash
go test ./gateway/load/... ./gateway/handlers/... -count=1 -run 'Load|JobInsert|Copy|Extract'
task testdata:fake-gcs-sync
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_load_table_uri_csv.py samples/tests/test_load_table_uri_json.py -v' task thirdparty:python-bigquery-tests
```

Record remaining failure count before writing AVRO/ORC code. Many CSV/JSON/Parquet GCS tests may already pass after plan 01.

## Current stubs

[`gateway/load/parse_columnar.go`](../../gateway/load/parse_columnar.go):

```go
// AVRO / ORC return explicit "not yet implemented" errors
```

Parquet path exists in [`parse_parquet.go`](../../gateway/load/parse_parquet.go) — use as template.

## Implementation

### AVRO

- Parse AVRO OCF bytes from `FetchSource` output
- Map schema to `bqtypes.TableSchema`; rows to bulk insert
- Consider `github.com/linkedin/goavro` or stdlib-compatible decoder (evaluate license + go.mod policy)

### ORC

- ORC is harder — evaluate `scritchley/orc` or defer with **clear** job `errorResult` if second attempt shows multi-day scope
- Document DEFER in orchestration-status if deferred

### Job error surfacing

Node shows generic `A failure occurred during this request` when gateway returns opaque 500. Improve in [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) — include `status.errorResult` with parser/fetch message.

### schemaUpdateOptions

Confirm load-job path for add/relax column (landed partially in `f155c4f`) against node `load_table_add_column` / relax variants if still failing.

## Fast gate

```bash
go test ./gateway/load/... -count=1
```

## Slow gate

```bash
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_load_table_uri_avro.py samples/tests/test_load_table_uri_orc.py -v' task thirdparty:python-bigquery-tests
# node: GCS load family (items 34–55 in baseline log)
```

## Out of scope

- Resumable upload regressions (should stay green from plan 04)
- Hive external (plan 07)

## Done criteria

- [ ] Retest delta documented in orchestration-status
- [ ] AVRO load samples pass or honest DEFER with tests skipped only where implemented
- [ ] Local CSV load + browse-rows cascade fixed if still failing
