---
name: wire-marshal-go
overview: "Phase 5c: BigQuery f/v wire marshaling and unit tests."
todos:
  - id: wire-marshal
    content: "Add gateway/bqtypes/wire.go: ValueToCell per StandardSqlDataType — INT64 string, TIMESTAMP RFC3339Z, STRUCT, NULL"
    status: pending
  - id: wire-marshal-test
    content: "Unit tests: INT64, FLOAT64 NaN, BYTES base64, TIMESTAMP, STRUCT, ARRAY, NULL"
    status: pending
isProject: false
---

# Phase 5c: Wire Marshal Go

## Prerequisites

- [execute-query-stream_y0b1c2d3.plan.md](execute-query-stream_y0b1c2d3.plan.md)

## Verification

```bash
go test ./gateway/bqtypes/... -run Wire
```

## Done criteria

- wire.go tests pass

## Next plan(s)

- [jobs-query-handler_a2d3e4f5.plan.md](jobs-query-handler_a2d3e4f5.plan.md)
