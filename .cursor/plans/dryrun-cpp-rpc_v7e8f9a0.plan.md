---
name: dryrun-cpp-rpc
overview: "Phase 4b: Query.DryRun gRPC and googlesql→BigQuery type reflection."
todos:
  - id: dryrun-rpc
    content: "Implement query.cc DryRun: parse, analyze, return DryRunResponse schema + estimated_bytes_processed"
    status: pending
  - id: type-reflection
    content: "Add googlesql_to_bq.{h,cc}: googlesql::Type → proto FieldSchema"
    status: pending
  - id: analyze-errors
    content: "Map analysis errors to InvalidArgument with line/col; gateway maps to HTTP 400 invalidQuery"
    status: pending
isProject: false
---

# Phase 4b: Dryrun Cpp Rpc

## Prerequisites

- [googlesql-vendor-catalog_u6d7e8f9.plan.md](googlesql-vendor-catalog_u6d7e8f9.plan.md)

## Verification

```bash
grpcurl -d '{"sql":"SELECT 1"}' localhost:9060 emulator.Query/DryRun
```

## Done criteria

- DryRun returns schema for SELECT 1
- Syntax error returns InvalidArgument

## Next plan(s)

- [dryrun-gateway-e2e_w8f9a0b1.plan.md](dryrun-gateway-e2e_w8f9a0b1.plan.md)
