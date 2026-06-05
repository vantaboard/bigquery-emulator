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

Log: `.logs/thirdparty-20260605-134407.log` (exit 201, `THIRDPARTY_REBUILD=1`)

| Suite | Baseline | Final | Status |
|-------|----------|-------|--------|
| golang-bigquery-tests | OK | OK | PASS |
| python-bigquery-tests | 34 failed | 1 failed (`test_client_query_total_rows`: seed has 3 TX rows, expects 100) | FAILED |
| node-bigquery-tests | 59 failed | 45 failing | FAILED (Δ −14) |
| java-bigquery-tests | panic | `java-bigquery/samples/snippets` failed (`CreateTableExternalHivePartitionedIT` not allowlisted; fake-gcs down for hive path) | FAILED |
| dataframes-snippet-gate | 33 errors | 2 pass / 2 fail | PARTIAL |

**Overall:** DEFERRED partial parity — plans 04–11 landed; full `task thirdparty` did not reach zero failures.

## Unblock lane

Dispatch: `.cursor/plans/unblock_subagent_dispatch_896b06e4.plan.md`  
Baseline: `.logs/thirdparty-20260605-134407.log`  
**NEXT:** `unblock-07-hive-external`

| Plan | Attempts | Result | Commit | Notes |
|------|----------|--------|--------|-------|
| unblock-01-gcs-networking | 1 | PASS | add0410 | in-container fake-gcs OK; test_load_table_uri_csv PASSED |
| unblock-02-public-data-seed | 1 | PASS | 73508c8 | 100 TX rows + usa_1910_current; test_client_query_total_rows PASSED |
| unblock-03-bigframes-gate | 1 | PARTIAL | d8adef2 | 3/4 snippet gate; performance_optimizations needs storage read (08) |
| unblock-04-gateway-wire-shapes | 1 | PASS | 78fd622 | expirationTime/labels/writeDisposition/dataset region wire shapes |
| unblock-05-query-params | 1 | PASS | f8f4fb0 | ARRAY/TIMESTAMP/STRUCT handler+engine binding |
| unblock-06-load-avro-orc | 1 | PARTIAL | 4670fec | AVRO+ORC URI load pass; truncate/resumable + node Jobs remain |
| unblock-07-hive-external | — | pending | — | — |
| unblock-08-storage-grpc | — | pending | — | — |
| unblock-09-test-isolation | — | pending | — | — |
| unblock-10-final-aggregator | — | pending | — | — |

Index: [unblock-00-index.plan.md](unblock-00-index.plan.md)

## Remaining blockers (honest partial parity)

- **Seed:** Expand `usa_names.usa_1910_2013` TX rows (≥100) for `test_client_query_total_rows`; add `usa_1910_current` view for node Views suite
- **LOAD:** AVRO/ORC formats; full thirdparty load suite after docker rebuild
- **External:** Hive-partitioned external table IT (`CreateTableExternalHivePartitionedIT`); Google Sheets samples (501 by design)
- **Engine:** DuckDB `OrderByScan` for BigFrames pandas_methods; `sessionInfo` on query/job responses
- **Storage:** Public `BigQueryWrite`/`BigQueryRead` gRPC shim for Java ITs (other modules allowlisted)
- **Node:** 45 remaining failures (views, routines, external, copy/extract families — see log §node)
- **Conformance:** TIMESTAMP param fixture (plan 08 optional)
