---
name: bigquery-emulator-roadmap-index
overview: "Umbrella index for bigquery-emulator. Sequences 46 small sub-plans (Phases 0–9) for one subagent session each. All plans are size S."
todos:
  - id: run-00-bootstrap-docker
    content: "Run bootstrap-docker_a0b1c2d3.plan.md"
    status: completed
  - id: run-01-bootstrap-ci
    content: "Run bootstrap-ci_e0f1a2b3.plan.md"
    status: completed
  - id: run-02-gateway-auth-discovery
    content: "Run gateway-auth-discovery_f1a2b3c4.plan.md"
    status: completed
  - id: run-03-gateway-legacy-sql
    content: "Run gateway-legacy-sql_g2b3c4d5.plan.md"
    status: completed
  - id: run-04-grpc-proto-codegen
    content: "Run grpc-proto-codegen_h3c4d5e6.plan.md"
    status: completed
  - id: run-05-grpc-cpp-server
    content: "Run grpc-cpp-server_i4d5e6f7.plan.md"
    status: completed
  - id: run-06-grpc-gateway-client
    content: "Run grpc-gateway-client_j5e6f7a8.plan.md"
    status: completed
  - id: run-07-cpp-interfaces
    content: "Run cpp-interfaces_k6f7a8b9.plan.md"
    status: completed
  - id: run-08-memory-storage
    content: "Run memory-storage_l7a8b9c0.plan.md"
    status: completed
  - id: run-09-engine-cli-scaffold
    content: "Run engine-cli-scaffold_m8b9c0d1.plan.md"
    status: completed
  - id: run-10-vendor-duckdb
    content: "Run vendor-duckdb_n9c0d1e2.plan.md"
    status: completed
  - id: run-11-duckdb-storage-core
    content: "Run duckdb-storage-core_o0d1e2f3.plan.md"
    status: completed
  - id: run-12-duckdb-storage-ddl
    content: "Run duckdb-storage-ddl_p1e2f3a4.plan.md"
    status: completed
  - id: run-13-catalog-grpc-cpp
    content: "Run catalog-grpc-cpp_q2f3a4b5.plan.md"
    status: completed
  - id: run-14-catalog-rest-go
    content: "Run catalog-rest-go_r3a4b5c6.plan.md"
    status: completed
  - id: run-15-projects-stubs
    content: "Run projects-stubs_s4b5c6d7.plan.md"
    status: completed
  - id: run-16-tabledata-e2e
    content: "Run tabledata-e2e_t5c6d7e8.plan.md"
    status: completed
  - id: run-17-googlesql-vendor-catalog
    content: "Run googlesql-vendor-catalog_u6d7e8f9.plan.md"
    status: completed
  - id: run-18-dryrun-cpp-rpc
    content: "Run dryrun-cpp-rpc_v7e8f9a0.plan.md"
    status: completed
  - id: run-19-dryrun-gateway-e2e
    content: "Run dryrun-gateway-e2e_w8f9a0b1.plan.md"
    status: completed
  - id: run-20-ref-impl-adapter
    content: "Run ref-impl-adapter_x9a0b1c2.plan.md"
    status: completed
  - id: run-21-execute-query-stream
    content: "Run execute-query-stream_y0b1c2d3.plan.md"
    status: completed
  - id: run-22-wire-marshal-go
    content: "Run wire-marshal-go_z1c2d3e4.plan.md"
    status: completed
  - id: run-23-jobs-query-handler
    content: "Run jobs-query-handler_a2d3e4f5.plan.md"
    status: completed
  - id: run-24-query-select-e2e
    content: "Run query-select-e2e_b3e4f5a6.plan.md"
    status: completed
  - id: run-25-transpiler-skeleton
    content: "Run transpiler-skeleton_c4f5a6b7.plan.md"
    status: completed
  - id: run-26-transpiler-emit-scans
    content: "Run transpiler-emit-scans_d5a6b7c8.plan.md"
    status: completed
  - id: run-27-transpiler-emit-join-agg
    content: "Run transpiler-emit-join-agg_e6b7c8d9.plan.md"
    status: completed
  - id: run-28-duckdb-engine-exec
    content: "Run duckdb-engine-exec_f7c8d9e0.plan.md"
    status: completed
  - id: run-29-transpiler-struct-unnest
    content: "Run transpiler-struct-unnest_a8d9e0f1.plan.md"
    status: completed
  - id: run-30-transpiler-functions-window
    content: "Run transpiler-functions-window_b9e0f1a2.plan.md"
    status: completed
  - id: run-31-duckdb-arrow-pipeline
    content: "Run duckdb-arrow-pipeline_c0f1a2b3.plan.md"
    status: completed
  - id: run-32-duckdb-parity-e2e
    content: "Run duckdb-parity-e2e_d1a2b3c4.plan.md"
    status: completed
  - id: run-33-dml-insert
    content: "Run dml-insert-e2e_e2b3c4d5.plan.md"
    status: completed
  - id: run-34-dml-update-delete
    content: "Run dml-update-delete_f3c4d5e6.plan.md"
    status: completed
  - id: run-35-ddl-statements
    content: "Run ddl-statements_g4d5e6f7.plan.md"
    status: completed
  - id: run-36-dml-ddl-e2e
    content: "Run dml-ddl-e2e_h5e6f7a8.plan.md"
    status: completed
  - id: run-37-storage-read-proto
    content: "Run storage-read-proto_i6f7a8b9.plan.md"
    status: completed
  - id: run-38-storage-read-rows
    content: "Run storage-read-rows_j7a8b9c0.plan.md"
    status: completed
  - id: run-39-storage-read-gateway-e2e
    content: "Run storage-read-gateway_e2e_k8b9c0d1.plan.md"
    status: completed
  - id: run-40-conformance-fixtures-runner
    content: "Run conformance-fixtures-runner_l9c0d1e2.plan.md"
    status: completed
  - id: run-41-conformance-diff-ci
    content: "Run conformance-diff-ci_m0d1e2f3.plan.md"
    status: completed
  - id: run-42-conformance-seed-docs
    content: "Run conformance-seed-docs_n1e2f3a4.plan.md"
    status: completed
  - id: run-43-docker-compose-smoke
    content: "Run docker-compose-smoke_o2f3a4b5.plan.md"
    status: completed
  - id: run-44-goreleaser-release
    content: "Run goreleaser-release_p3a4b5c6.plan.md"
    status: pending
  - id: run-45-profile-docs-version
    content: "Run profile-docs-version_q4b5c6d7.plan.md"
    status: pending
isProject: false
---

# BigQuery emulator — roadmap plan index

46 **small** sub-plans under `.cursor/plans/`. Each plan has ≤3 todos and fits one subagent session.

## Already landed

- Phase 0 scaffold + Phase 1 route wiring (501 stubs)
- [docs/REST_API.md](../../docs/REST_API.md)

## Plan catalog (all S)

| # | Plan | Phase |
|---|------|-------|
| 00 | [bootstrap-docker_a0b1c2d3](bootstrap-docker_a0b1c2d3.plan.md) | Phase 0a |
| 01 | [bootstrap-ci_e0f1a2b3](bootstrap-ci_e0f1a2b3.plan.md) | Phase 0b |
| 02 | [gateway-auth-discovery_f1a2b3c4](gateway-auth-discovery_f1a2b3c4.plan.md) | Phase 1a |
| 03 | [gateway-legacy-sql_g2b3c4d5](gateway-legacy-sql_g2b3c4d5.plan.md) | Phase 1b |
| 04 | [grpc-proto-codegen_h3c4d5e6](grpc-proto-codegen_h3c4d5e6.plan.md) | Phase 2a |
| 05 | [grpc-cpp-server_i4d5e6f7](grpc-cpp-server_i4d5e6f7.plan.md) | Phase 2b |
| 06 | [grpc-gateway-client_j5e6f7a8](grpc-gateway-client_j5e6f7a8.plan.md) | Phase 2c |
| 07 | [cpp-interfaces_k6f7a8b9](cpp-interfaces_k6f7a8b9.plan.md) | Phase 3a |
| 08 | [memory-storage_l7a8b9c0](memory-storage_l7a8b9c0.plan.md) | Phase 3b |
| 09 | [engine-cli-scaffold_m8b9c0d1](engine-cli-scaffold_m8b9c0d1.plan.md) | Phase 3c |
| 10 | [vendor-duckdb_n9c0d1e2](vendor-duckdb_n9c0d1e2.plan.md) | Phase 3d |
| 11 | [duckdb-storage-core_o0d1e2f3](duckdb-storage-core_o0d1e2f3.plan.md) | Phase 3e |
| 12 | [duckdb-storage-ddl_p1e2f3a4](duckdb-storage-ddl_p1e2f3a4.plan.md) | Phase 3f |
| 13 | [catalog-grpc-cpp_q2f3a4b5](catalog-grpc-cpp_q2f3a4b5.plan.md) | Phase 3g |
| 14 | [catalog-rest-go_r3a4b5c6](catalog-rest-go_r3a4b5c6.plan.md) | Phase 3h |
| 15 | [projects-stubs_s4b5c6d7](projects-stubs_s4b5c6d7.plan.md) | Phase 3i |
| 16 | [tabledata-e2e_t5c6d7e8](tabledata-e2e_t5c6d7e8.plan.md) | Phase 3j |
| 17 | [googlesql-vendor-catalog_u6d7e8f9](googlesql-vendor-catalog_u6d7e8f9.plan.md) | Phase 4a |
| 18 | [dryrun-cpp-rpc_v7e8f9a0](dryrun-cpp-rpc_v7e8f9a0.plan.md) | Phase 4b |
| 19 | [dryrun-gateway-e2e_w8f9a0b1](dryrun-gateway-e2e_w8f9a0b1.plan.md) | Phase 4c |
| 20 | [ref-impl-adapter_x9a0b1c2](ref-impl-adapter_x9a0b1c2.plan.md) | Phase 5a |
| 21 | [execute-query-stream_y0b1c2d3](execute-query-stream_y0b1c2d3.plan.md) | Phase 5b |
| 22 | [wire-marshal-go_z1c2d3e4](wire-marshal-go_z1c2d3e4.plan.md) | Phase 5c |
| 23 | [jobs-query-handler_a2d3e4f5](jobs-query-handler_a2d3e4f5.plan.md) | Phase 5d |
| 24 | [query-select-e2e_b3e4f5a6](query-select-e2e_b3e4f5a6.plan.md) | Phase 5e |
| 25 | [transpiler-skeleton_c4f5a6b7](transpiler-skeleton_c4f5a6b7.plan.md) | Phase 5f |
| 26 | [transpiler-emit-scans_d5a6b7c8](transpiler-emit-scans_d5a6b7c8.plan.md) | Phase 5g |
| 27 | [transpiler-emit-join-agg_e6b7c8d9](transpiler-emit-join-agg_e6b7c8d9.plan.md) | Phase 5h |
| 28 | [duckdb-engine-exec_f7c8d9e0](duckdb-engine-exec_f7c8d9e0.plan.md) | Phase 5i |
| 29 | [transpiler-struct-unnest_a8d9e0f1](transpiler-struct-unnest_a8d9e0f1.plan.md) | Phase 5j |
| 30 | [transpiler-functions-window_b9e0f1a2](transpiler-functions-window_b9e0f1a2.plan.md) | Phase 5k |
| 31 | [duckdb-arrow-pipeline_c0f1a2b3](duckdb-arrow-pipeline_c0f1a2b3.plan.md) | Phase 5l |
| 32 | [duckdb-parity-e2e_d1a2b3c4](duckdb-parity-e2e_d1a2b3c4.plan.md) | Phase 5m |
| 33 | [dml-insert-e2e_e2b3c4d5](dml-insert-e2e_e2b3c4d5.plan.md) | Phase 6a |
| 34 | [dml-update-delete_f3c4d5e6](dml-update-delete_f3c4d5e6.plan.md) | Phase 6b |
| 35 | [ddl-statements_g4d5e6f7](ddl-statements_g4d5e6f7.plan.md) | Phase 6c |
| 36 | [dml-ddl-e2e_h5e6f7a8](dml-ddl-e2e_h5e6f7a8.plan.md) | Phase 6d |
| 37 | [storage-read-proto_i6f7a8b9](storage-read-proto_i6f7a8b9.plan.md) | Phase 7a |
| 38 | [storage-read-rows_j7a8b9c0](storage-read-rows_j7a8b9c0.plan.md) | Phase 7b |
| 39 | [storage-read-gateway_e2e_k8b9c0d1](storage-read-gateway_e2e_k8b9c0d1.plan.md) | Phase 7c |
| 40 | [conformance-fixtures-runner_l9c0d1e2](conformance-fixtures-runner_l9c0d1e2.plan.md) | Phase 8a |
| 41 | [conformance-diff-ci_m0d1e2f3](conformance-diff-ci_m0d1e2f3.plan.md) | Phase 8b |
| 42 | [conformance-seed-docs_n1e2f3a4](conformance-seed-docs_n1e2f3a4.plan.md) | Phase 8c |
| 43 | [docker-compose-smoke_o2f3a4b5](docker-compose-smoke_o2f3a4b5.plan.md) | Phase 9a |
| 44 | [goreleaser-release_p3a4b5c6](goreleaser-release_p3a4b5c6.plan.md) | Phase 9b |
| 45 | [profile-docs-version_q4b5c6d7](profile-docs-version_q4b5c6d7.plan.md) | Phase 9c |

## Dependency graph (simplified)

```
00→01→02  (bootstrap)
03→04     (gateway polish)
05→06→07  (grpc)
08→09→10  (cpp interfaces + memory + cli)
11→12→13  (duckdb storage)
14→15→16→17 (catalog + tabledata)
18→19→20  (dryrun)
21→22→23→24→25 (reference impl exec)
26→27→28→29→30→31→32→33 (duckdb transpiler)
34→35→36→37 (dml/ddl)
38→39→40  (storage read)
41→42→43  (conformance)
44→45→46  (distribution)
```

**Parallel lanes after plan 20:** 21–25 (ref impl) ∥ 26–33 (duckdb engine)

## Subagent template

```
Read and execute .cursor/plans/<plan-file>.plan.md
Mark todos completed. Commit per CLAUDE.md. Run verification before done.
```
