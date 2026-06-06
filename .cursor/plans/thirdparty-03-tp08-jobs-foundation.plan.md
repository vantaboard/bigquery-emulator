---
name: Thirdparty 03 — tp08 jobs foundation
overview: Extend jobs.insert wire model and JobInsert branching for LOAD/COPY/EXTRACT job types. Replaces 501 stub with handler skeletons — no format readers yet.
depends_on: [thirdparty-02-gateway-query-metadata]
blocks: [thirdparty-04-tp08-load-jobs, thirdparty-05-tp08-copy-extract-undelete]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 2-3 days
isProject: true
todos:
  - id: job-config-types
    content: Add Load/Copy/Extract sub-structs to gateway/jobs/registry.go JobConfiguration
    status: pending
  - id: job-insert-branch
    content: Branch JobInsert on configuration.load|copy|extract instead of blanket 501
    status: pending
  - id: job-statistics
    content: Add statistics.load|copy|extract counters on completed jobs
    status: pending
  - id: flip-unit-tests
    content: Replace TestJobInsertRejectsLoadConfig with type-dispatch tests; keep 501 only for unknown shapes
    status: pending
  - id: handler-stubs
    content: Wire runSyncLoadInsert/runSyncCopyInsert/runSyncExtractInsert entry points (may return structured errors until plans 04/05)
    status: pending
---

# Thirdparty 03 — tp08 jobs foundation

## Goal

Land the **structural** tp08 surface so plans 04 and 05 can implement behavior without reshaping the registry.

## Current state

[`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) lines 140–144:

```go
if cfg == nil || cfg.Query == nil {
    writeError(w, http.StatusNotImplemented, reasonNotImplemented,
        "jobs.insert: only configuration.query is implemented; "+
            "load / copy / extract paths land in thirdparty-08.")
    return
}
```

[`gateway/jobs/registry.go`](../../gateway/jobs/registry.go) `JobConfiguration` only models `Query`.

[`gateway/handlers/jobs_test.go`](../../gateway/handlers/jobs_test.go) `TestJobInsertRejectsLoadConfig` pins the 501 behavior.

## Implementation

### 1. Wire types

Add to `JobConfiguration` (mirror upstream REST):

```go
type JobConfiguration struct {
    JobType string                    `json:"jobType,omitempty"` // QUERY|LOAD|COPY|EXTRACT
    Query   *JobConfigurationQuery    `json:"query,omitempty"`
    Load    *JobConfigurationLoad     `json:"load,omitempty"`
    Copy    *JobConfigurationCopy     `json:"copy,omitempty"`
    Extract *JobConfigurationExtract  `json:"extract,omitempty"`
    // ...
}
```

Model minimum fields needed by thirdparty samples:
- **Load:** `sourceUris`, `destinationTable`, `sourceFormat`, `writeDisposition`, `schema`, `autodetect`, `schemaUpdateOptions`
- **Copy:** `sourceTables`, `destinationTable`, `writeDisposition`
- **Extract:** `sourceTable`, `destinationUris`, `destinationFormat`, `compression`

Reference upstream shapes in [`docs/bigquery/docs/reference/rest/v2/jobs#JobConfiguration`](../../docs/bigquery/docs/reference/rest/v2/jobs.md).

### 2. JobInsert dispatch

```go
switch {
case cfg.Query != nil:
    runSyncQueryInsert(...)
case cfg.Load != nil:
    runSyncLoadInsert(...)
case cfg.Copy != nil:
    runSyncCopyInsert(...)
case cfg.Extract != nil:
    runSyncExtractInsert(...)
default:
    writeError(..., 501, ...)
}
```

### 3. Job lifecycle

Match existing query-job posture:
- Mint `jobId`, record in `deps.Jobs` as `DONE` synchronously
- Populate `statistics.creationTime/startTime/endTime`
- On failure: set `status.errorResult` (don't 4xx at insert time)

### 4. Statistics extensions

Extend `Statistics` with optional `Load`, `Copy`, `Extract` sub-objects per REST spec (output rows, input files, etc.).

## Verification

```bash
go test ./gateway/handlers/... -count=1 -run JobInsert
go test ./gateway/jobs/... -count=1
```

Expect:
- Query jobs unchanged
- Load/copy/extract bodies accepted (200 + Job resource) even if inner logic returns job-level errors until plans 04/05

## Out of scope

- Reading GCS bytes (plan 04)
- Format parsers CSV/JSON/Avro/Parquet/ORC (plan 04)
- Copy row materialization (plan 05)
- Extract writes to GCS (plan 05)
- `JobInsertUpload` resumable path (plan 04)

## Done when

- [ ] `JobConfiguration` round-trips load/copy/extract JSON from client libraries
- [ ] `JobInsert` dispatches by job type
- [ ] Unit tests cover dispatch; old "rejects load" test replaced
- [ ] `docs/REST_API.md` marks `jobs.insert` load/copy/extract as partial/implemented
