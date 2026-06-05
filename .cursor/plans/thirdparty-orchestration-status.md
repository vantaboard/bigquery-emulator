# Thirdparty orchestration status

Started: 2026-06-05  
Dispatch plan: `.cursor/plans/dispatch_remaining_thirdparty_plans_c28984a6.plan.md`  
Baseline: `.logs/thirdparty-20260605-112926.log`

## Plan results

| Plan | Attempts | Result | Commit | Notes |
|------|----------|--------|--------|-------|
| 01-harness | 1 | PASS | 7eaa9e5 | java exit 0; 4/32 dataframes selected |
| 02-gateway-query-metadata | 1 | PASS | 8c4dbfd | query params, pagination, metadata PATCH |
| 03-tp08-foundation | 1 | PASS | 4027bdc | JobInsert dispatch + registry types |
| 04-tp08-load | 1 | PARTIAL | b1023fe | Phase A: CSV/JSON GCS+local |
| 04-tp08-load-remainder | 1 | PARTIAL | f155c4f | Parquet, uploads, schemaUpdateOptions; AVRO/ORC deferred |
| 05-tp08-copy-extract | 1 | PASS | 06cc8c2 | COPY/EXTRACT + snapshot undelete |
| 06-routines-crud | 1 | PASS | ffda83a | Routine REST CRUD + DDL hook |
| 07-external-tables | 1 | PARTIAL | 05ad37f | GCS external CSV; Sheets 501 |
| 08-advanced-query-params | 1 | PARTIAL | 5e582b1 | TIMESTAMP/STRUCT/positional; schema jobs |
| 09-public-data-seed | 1 | PARTIAL | a99a901 | usa_names/shakespeare/stackoverflow seeded |
| 10-storage-grpc | 1 | PARTIAL | fb36866 | BUFFERED flush engine path; Java ITs still allowlisted |
| 11-bigframes-gate | 1 | PARTIAL | f1f41cc | 2/4 snippet gate pass; OrderByScan + sessionInfo remain |

## Final aggregator

Log: `.logs/thirdparty-20260605-160518.log` (exit 201, `THIRDPARTY_REBUILD=1`, post-unblock lane)

| Suite | Baseline (134407) | Final (160518) | Status |
|-------|-------------------|----------------|--------|
| golang-bigquery-tests | OK | OK | PASS |
| python-bigquery-tests | 34 failed | 22 failed (`test_client_query_total_rows` PASS; truncate/resumable + query params remain) | FAILED (Δ −12) |
| node-bigquery-tests | 45 failing | 18 failing (no `Already Exists`) | FAILED (Δ −27) |
| java-bigquery-tests | snippets IT fail | OK (`CreateTableExternalHivePartitionedIT` green) | PASS |
| dataframes-snippet-gate | 2 pass / 2 fail | 3 pass / 1 fail (`performance_optimizations` needs storage read) | PARTIAL |

**Overall:** DEFERRED partial parity — unblock lane 01–09 landed; storage gRPC (08) and residual python/node failures block exit 0.

## Unblock lane

Dispatch: `.cursor/plans/unblock_subagent_dispatch_896b06e4.plan.md`  
Baseline: `.logs/thirdparty-20260605-134407.log`  
**NEXT:** *(unblock lane complete — second pass on DEFERRED 08 + residual suites)*

| Plan | Attempts | Result | Commit | Notes |
|------|----------|--------|--------|-------|
| unblock-01-gcs-networking | 1 | PASS | add0410 | in-container fake-gcs OK; test_load_table_uri_csv PASSED |
| unblock-02-public-data-seed | 1 | PASS | 73508c8 | 100 TX rows + usa_1910_current; test_client_query_total_rows PASSED |
| unblock-03-bigframes-gate | 1 | PARTIAL | d8adef2 | 3/4 snippet gate; performance_optimizations needs storage read (08) |
| unblock-04-gateway-wire-shapes | 1 | PASS | 78fd622 | expirationTime/labels/writeDisposition/dataset region wire shapes |
| unblock-05-query-params | 1 | PASS | f8f4fb0 | ARRAY/TIMESTAMP/STRUCT handler+engine binding |
| unblock-06-load-avro-orc | 1 | PARTIAL | 4670fec | AVRO+ORC URI load pass; truncate/resumable + node Jobs remain |
| unblock-07-hive-external | 1 | PASS | bb2bbc3 | CreateTableExternalHivePartitionedIT green |
| unblock-08-storage-grpc | 2 | DEFERRED | e094416 | bazel storage tests pass; Java ITs need public gRPC shim + Arrow IPC |
| unblock-09-test-isolation | 1 | PARTIAL | cfcd367 | node 45→18; no Already Exists; cascade delete fixed |
| unblock-10-final-aggregator | 1 | DEFERRED | 86f46a2 | exit 201; log `.logs/thirdparty-20260605-160518.log` |

Index: [unblock-00-index.plan.md](unblock-00-index.plan.md)

## Remaining blockers (honest partial parity)

- **Storage (08):** Public `BigQueryWrite`/`BigQueryRead` gRPC shim + Arrow IPC for `performance_optimizations` and Java storage ITs
- **Python (22):** Truncate/resumable load URIs, query destination/params, copy/extract, Sheets external (501 by design)
- **Node (18):** Query transpiler gaps, wire shapes (labels/collation), browse rows, Firestore backup load
- **LOAD:** Truncate write disposition + resumable upload `Location` header (relative URL → MissingSchema)
