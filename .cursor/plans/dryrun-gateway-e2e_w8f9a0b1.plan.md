---
name: dryrun-gateway-e2e
overview: "Phase 4c: Wire jobs.query dryRun=true end-to-end."
todos:
  - id: go-dryrun-handler
    content: "Wire queries.go QueryRun dryRun=true: call DryRun, return schema + totalBytesProcessed, jobComplete=true"
    status: completed
  - id: dryrun-test
    content: "E2E: dryRun SELECT * FROM ds.t matches table schema; bad syntax returns 400 invalidQuery"
    status: completed
isProject: false
---

# Phase 4c: Dryrun Gateway E2E

## Prerequisites

- [dryrun-cpp-rpc_v7e8f9a0.plan.md](dryrun-cpp-rpc_v7e8f9a0.plan.md)

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/queries -d '{"query":"SELECT 1","dryRun":true,"useLegacySql":false}'
```

## Done criteria

- jobs.query?dryRun=true works E2E

## Next plan(s)

- [ref-impl-adapter_x9a0b1c2.plan.md](ref-impl-adapter_x9a0b1c2.plan.md)
- [transpiler-skeleton_c4f5a6b7.plan.md](transpiler-skeleton_c4f5a6b7.plan.md)
