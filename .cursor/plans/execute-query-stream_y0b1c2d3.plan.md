---
name: execute-query-stream
overview: "Phase 5b: ExecuteQuery server-streaming gRPC handler."
todos:
  - id: execute-query-rpc
    content: "Implement query.cc ExecuteQuery: stream schema first then QueryResultRow batches"
    status: pending
isProject: false
---

# Phase 5b: Execute Query Stream

## Prerequisites

- [ref-impl-adapter_x9a0b1c2.plan.md](ref-impl-adapter_x9a0b1c2.plan.md)

## Verification

```bash
grpcurl -d '{"sql":"SELECT 1"}' localhost:9060 emulator.Query/ExecuteQuery
```

## Done criteria

- ExecuteQuery streams rows over gRPC

## Next plan(s)

- [wire-marshal-go_z1c2d3e4.plan.md](wire-marshal-go_z1c2d3e4.plan.md)
