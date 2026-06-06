---
name: Unblock 07 — Hive external tables
overview: hivePartitioningOptions for external table create + fake-gcs hive fixtures; green CreateTableExternalHivePartitionedIT.
depends_on: [unblock-01-gcs-networking, unblock-06-load-avro-orc]
blocks: [unblock-08-storage-grpc]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: ~1 week
isProject: true
todos:
  - id: fake-gcs-hive-fixtures
    content: Ensure testdata:fake-gcs-sync mirrors gs://cloud-samples-data/bigquery/hive-partitioning-samples/customlayout/*
    status: pending
  - id: bqtypes-hive-options
    content: Add hivePartitioningOptions to externalDataConfiguration in gateway/bqtypes
    status: pending
  - id: materialize-hive
    content: Extend gateway/external/materialize.go — expand partition URIs, parse partition keys from path
    status: pending
  - id: tables-insert-hive
    content: Wire hive options in gateway/handlers/tables.go external table insert
    status: pending
  - id: gate-java-hive-it
    content: JAVA_BQ_SAMPLE_PATHS='java-bigquery/samples/snippets' mvn verify — CreateTableExternalHivePartitionedIT green
    status: pending
  - id: status-commit
    content: Update orchestration-status.md; do NOT allowlist CreateTableExternalHivePartitionedIT; lint + commit
    status: pending
---

# Unblock 07 — Hive external tables

## Goal

Pass Java [`CreateTableExternalHivePartitionedIT`](../../third_party/java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/CreateTableExternalHivePartitionedIT.java) **without** adding it to `JAVA_BQ_ALLOW_FAILING_ITS`.

Extends partial [thirdparty-07-external-tables.plan.md](thirdparty-07-external-tables.plan.md) (GCS CSV external only at `05ad37f`).

## Log signature

```
External table was not created... fetch gs://cloud-samples-data/bigquery/hive-partitioning-samples/customlayout/*: ... connection refused
```

After plan 01, fetch should succeed; remaining gap is **hive partition options** parsing.

## Prerequisites

| Plan | Why |
|------|-----|
| **01 GCS networking** | Container reaches `fake-gcs-server:4443` |
| **06 Load retest** | Shared `FetchSource` / parse patterns stable |

## Implementation

### 1. Fixtures

Extend [`scripts/sync_fake_gcs_public_samples.sh`](../../scripts/sync_fake_gcs_public_samples.sh) (or HTTP mirror) to include:

```
gs://cloud-samples-data/bigquery/hive-partitioning-samples/customlayout/**
```

Verify:

```bash
task testdata:fake-gcs-sync
curl -s "http://localhost:4443/storage/v1/b/cloud-samples-data/o?prefix=bigquery/hive-partitioning-samples/"
```

### 2. API types

Add to external table config in [`gateway/bqtypes`](../../gateway/bqtypes):

- `hivePartitioningOptions` (mode, sourceUriPrefix, requirePartitionFilter, fields)

Reference: BigQuery REST `ExternalDataConfiguration` + Java sample [`CreateTableExternalHivePartitioned.java`](../../third_party/java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/CreateTableExternalHivePartitioned.java).

### 3. Materialization

[`gateway/external/materialize.go`](../../gateway/external/materialize.go):

- Expand `sourceUris` glob / prefix to concrete objects via GCS list API (fake-gcs supports list)
- Parse partition columns from path segments per custom layout
- Register table schema = data columns + partition columns
- Insert parsed rows (or engine external scan if query path preferred)

### 4. Handlers

[`gateway/handlers/tables.go`](../../gateway/handlers/tables.go) — pass hive options into `external.Materialize`.

## Fast gate

```bash
go test ./gateway/external/... ./gateway/handlers/... -count=1 -run 'External|Hive'
```

## Slow gate

```bash
JAVA_BQ_SAMPLE_PATHS='java-bigquery/samples/snippets' task thirdparty:java-bigquery-tests
```

Expect `java-bigquery/samples/snippets` module exit 0 with **no** unexpected failing ITs.

## Out of scope

- Google Sheets external (stay 501 per plan 07)
- Full partition pruning optimizer (correctness over pruning for IT)
- Storage gRPC (plan 08)

## Done criteria

- [ ] `CreateTableExternalHivePartitionedIT` passes
- [ ] Node external GCS tests that need hive layout pass if in baseline
- [ ] Sheets samples still return clear 501
