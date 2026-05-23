---
name: ddl-statements
overview: "Phase 6c: CREATE/DROP/ALTER TABLE via SQL."
todos:
  - id: ddl-create-drop
    content: "CREATE TABLE AS SELECT, CREATE TABLE (schema), DROP TABLE, ALTER TABLE ADD COLUMN via catalog+storage"
    status: pending
isProject: false
---

# Phase 6c: Ddl Statements

## Prerequisites

- [dml-update-delete_f3c4d5e6.plan.md](dml-update-delete_f3c4d5e6.plan.md)

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/queries -d '{"query":"CREATE TABLE ds.new AS SELECT 1 x","useLegacySql":false}'
```

## Done criteria

- CREATE TABLE + DROP TABLE round-trip via SQL

## Next plan(s)

- [dml-ddl-e2e_h5e6f7a8.plan.md](dml-ddl-e2e_h5e6f7a8.plan.md)
