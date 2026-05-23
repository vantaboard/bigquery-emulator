---
name: tabledata-e2e
overview: "Phase 3j: insertAll + tabledata.list E2E on memory storage."
todos:
  - id: insert-all-handler
    content: "Implement tabledata.insertAll: decode body, convert rows to proto, AppendRows via catalog/engine"
    status: pending
  - id: tabledata-list-handler
    content: "Implement tabledata.list: paginated GET with startIndex/maxResults/pageToken"
    status: pending
  - id: e2e-test-go
    content: "Add gateway/e2e/catalog_test.go: create dataset/table, insertAll 2 rows, list returns 2 rows"
    status: pending
isProject: false
---

# Phase 3j: Tabledata E2E

## Prerequisites

- [projects-stubs_s4b5c6d7.plan.md](projects-stubs_s4b5c6d7.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/...
```

## Done criteria

- insertAll + list round-trip 2 rows on --storage=memory

## Next plan(s)

- [googlesql-vendor-catalog_u6d7e8f9.plan.md](googlesql-vendor-catalog_u6d7e8f9.plan.md)
