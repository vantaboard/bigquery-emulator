---
name: dml-update-delete
overview: "Phase 6b: UPDATE, DELETE, and basic MERGE on both engines."
todos:
  - id: update-delete
    content: "Reference impl UPDATE/DELETE scan+rewrite; DuckDB engine emits UPDATE/DELETE SQL"
    status: pending
  - id: merge-basic
    content: "MERGE INTO WHEN MATCHED/NOT MATCHED single-table target; document BQ limitations"
    status: pending
isProject: false
---

# Phase 6b: Dml Update Delete

## Prerequisites

- [dml-insert-e2e_e2b3c4d5.plan.md](dml-insert-e2e_e2b3c4d5.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run DML
```

## Done criteria

- UPDATE/DELETE/MERGE return correct dmlStats

## Next plan(s)

- [ddl-statements_g4d5e6f7.plan.md](ddl-statements_g4d5e6f7.plan.md)
