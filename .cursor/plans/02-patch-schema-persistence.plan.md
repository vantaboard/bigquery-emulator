---
name: PATCH schema persistence
overview: PATCH /tables schema changes (mode relax, descriptions, defaults) persist on GET so the Edit Schema modal refresh matches BigQuery.
todos:
  - id: verify-narrowing-400
    content: Confirm NULLABLE→REQUIRED 400 matches production BQ; document in docs/REST_API.md
    status: completed
  - id: patch-equals-get
    content: Optional — ensure PATCH 200 body always matches subsequent GET (engine-backed, not blind echo)
    status: cancelled
  - id: e2e-schema-regression
    content: gateway/e2e — REQUIRED→NULLABLE, description, defaultValueExpression round-trip
    status: completed
isProject: false
---

# 02 — PATCH /tables schema persistence reflected on GET

- **UI gap:** #3 (priority **P1** → **verify-only** at HEAD)
- **UI feature blocked:** Edit Schema modal refresh after PATCH.
- **Verified state at HEAD (`60d19b3e`):** **Pass** for the original repro; optional polish below.

## Current state at HEAD (grounded)

Verified 2026-06-23 on `:9050`.

| PATCH change | PATCH 200 body | GET after PATCH | Notes |
|--------------|----------------|-----------------|-------|
| mode REQUIRED→NULLABLE | ✅ NULLABLE | ✅ persists | |
| field description | ✅ | ✅ persists | |
| `defaultValueExpression` | ✅ | ✅ round-trips | |
| add new field | ✅ | ✅ persists | |
| NULLABLE→REQUIRED | — | — | **400** (BigQuery disallows narrowing) |
| policyTags / collation | ✅ | ✅ | overlay merge |

Original root causes (addition-only sync, overlay stripping) are **fixed** at HEAD.
Remaining work is regression tests and confirming narrowing semantics vs production BQ.

## Goal / done-criteria (UI-observable)

1. ✅ REQUIRED→NULLABLE persists on GET (original UI repro).
2. ✅ Description and `defaultValueExpression` round-trip.
3. ✅ NULLABLE→REQUIRED rejected with 400 (verify vs real BQ — expected correct).
4. Add e2e regression so the fix does not regress.

## Implementation steps

### Step 1 — Production BQ confirmation (verify-only)

Run the NULLABLE→REQUIRED PATCH against real BigQuery (or document from API
reference). If BQ allows it in some cases, align; otherwise keep 400.

### Step 2 — PATCH body consistency (optional polish)

If PATCH 200 ever diverges from GET again, re-read catalog after
`syncPatchedTableSchema` before returning (Step 4 from original plan).

### Step 3 — Regression tests

`gateway/e2e/schema_patch_test.go` covering the UI repro + narrowing rejection.

## Tests

- `gateway/handlers` (fake catalog) — `tables_schema*_test.go`:
  - REQUIRED→NULLABLE: PATCH then GET shows NULLABLE.
  - description edit: PATCH then GET shows new description.
  - `defaultValueExpression`: round-trips through GET.
  - add field: still persists.
  - invalid narrowing NULLABLE→REQUIRED: 400.
  - PATCH-body equals subsequent GET body.
- `gateway/e2e` (real engine): create `id INT64 REQUIRED` → PATCH NULLABLE → GET
  NULLABLE, with a real `ApplySchemaUpdate`.

## Out of scope

- Type coercion beyond BigQuery's allowed widenings.
- Reordering/removing columns (BigQuery disallows removal; keep current behavior).

## Touch list

`gateway/handlers/tables.go` (`TablePatch`, `syncPatchedTableSchema`),
`gateway/load/schema.go` (`mergeSchemas`, `ApplySchemaUpdate`),
`gateway/handlers/metadata_store.go` + `gateway/bqtypes/schema_policy.go`
(if overlay persistence needed), `gateway/bqtypes/types.go`,
`gateway/handlers/tables_schema.go`.
