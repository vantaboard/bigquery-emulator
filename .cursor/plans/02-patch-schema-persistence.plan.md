# 02 — PATCH /tables schema persistence reflected on GET

- **UI gap:** #3 (priority **P1**)
- **UI feature blocked:** Edit Schema modal — change mode (REQUIRED→NULLABLE),
  edit descriptions/defaults, add new fields. The modal trusts the PATCH 200
  body and updates cache, but a real refresh (`GET`) reverts the change.
- **Verified state at HEAD (`d390572`):** Broken for mode-relax / descriptions /
  `defaultValueExpression`; field-add works.

## Symptom / repro (UI brief)

1. Create table with `id INT64 REQUIRED`.
2. `PATCH /bigquery/v2/projects/{p}/datasets/{d}/tables/{t}` with
   `{ "schema": { "fields": [{ "name":"id","type":"INT64","mode":"NULLABLE", ... }] } }`.
3. PATCH returns **200** with the updated schema in the body (UI refreshes from it).
4. **Bug:** a subsequent `GET .../tables/{t}` still shows `mode: "REQUIRED"`
   (mode reverts). Description and `defaultValueExpression` edits are likewise
   not reflected.

## Current state at HEAD (grounded)

| PATCH change | PATCH 200 body | GET after PATCH | Why |
|--------------|----------------|-----------------|-----|
| mode REQUIRED→NULLABLE | shows NULLABLE (echo of request) | reverts to engine mode | `syncPatchedTableSchema` only passes `ALLOW_FIELD_ADDITION`, never `ALLOW_FIELD_RELAXATION`; overlay strips mode |
| field description | shows in body | unchanged | `mergeSchemas` doesn't update descriptions; overlay strips description |
| `defaultValueExpression` | dropped on decode | absent | not a field on `bqtypes.TableFieldSchema` |
| add new field | shows in body | **persists** if `ApplySchemaUpdate` succeeds | `syncPatchedTableSchema` + `load.ApplySchemaUpdate` |
| policyTags / collation | round-trips | works | `MergeSchemaPolicyTags` overlay |

### Root cause (two compounding issues)

1. **PATCH response is request-echo, not engine-backed.** `TablePatch`
   (`gateway/handlers/tables.go` ~431–449) merges the overlay into the request
   body and returns it without re-reading the catalog — so the 200 body looks
   correct even when nothing persisted.
2. **PATCH→engine sync is addition-only.** `syncPatchedTableSchema`
   (`tables.go` ~451–480) calls `load.MergeSchemasForAppend` /
   `load.ApplySchemaUpdate` with `[]string{"ALLOW_FIELD_ADDITION"}` only. The
   relaxation constant `ALLOW_FIELD_RELAXATION` already exists
   (`gateway/load/schema.go` ~15–16) but is never passed. `mergeSchemas`
   (`load/schema.go` ~162–204) only adds new columns or relaxes REQUIRED when
   `allowRelax` is true, and never updates descriptions.
3. **Overlay strips schema metadata.** `stripEngineOwnedTableFields`
   (`gateway/handlers/metadata_store.go` ~174–191) stores schema only via
   `ExtractSchemaPolicyOverlay` (policyTags + collation). Mode, description, and
   `defaultValueExpression` are dropped, so on GET the engine schema wins.

## Goal / done-criteria (UI-observable)

For the repro above and the broader Edit Schema modal:

1. `PATCH` with `mode: "NULLABLE"` on a REQUIRED field → subsequent `GET` shows
   `mode: "NULLABLE"`.
2. `PATCH` editing a field `description` → `GET` returns the new description.
3. `PATCH` setting `defaultValueExpression` → round-trips through GET.
4. Adding new fields continues to persist (no regression).
5. The PATCH 200 body equals what the next GET returns (consistency), not a blind
   echo of the request.
6. Invalid narrowing (NULLABLE→REQUIRED, type changes) is rejected with a
   BigQuery-consistent 400, matching real semantics.

## Implementation steps

### Step 1 — Add `defaultValueExpression` to the wire type

`gateway/bqtypes/types.go` `TableFieldSchema` (~467–481): add
`DefaultValueExpression string json:"defaultValueExpression,omitempty"`. Wire it
through `fieldToProto` / `fieldFromProto` (`gateway/handlers/tables_schema.go`)
and into the engine `FieldSchema` proto if the engine should own it; otherwise
persist via overlay (Step 3).

### Step 2 — Pass relaxation + description merge on PATCH

In `syncPatchedTableSchema` (`tables.go` ~454–480):
- Pass `[]string{"ALLOW_FIELD_ADDITION", "ALLOW_FIELD_RELAXATION"}` so
  REQUIRED→NULLABLE reaches the engine.
- Extend `load.mergeSchemas` / `load.ApplySchemaUpdate` (`load/schema.go`) to:
  - relax REQUIRED→NULLABLE on existing fields (constant already exists),
  - update `description` on existing fields,
  - reject invalid narrowing (NULLABLE→REQUIRED) and incompatible type changes
    with a clear error surfaced as 400.
- Decide where `defaultValueExpression` lives: engine schema (preferred, so GET
  derives it) or overlay (Step 3) if the engine cannot store it yet.

### Step 3 — Persist schema metadata the engine doesn't own

If `defaultValueExpression` (or description, if not engine-owned) must survive
GET, extend the overlay:
- `stripEngineOwnedTableFields` / `ExtractSchemaPolicyOverlay`
  (`metadata_store.go`, `gateway/bqtypes/schema_policy.go`) to also capture
  per-field `description` / `defaultValueExpression`.
- The GET merge (`applyTableMetadataOverlay` → `MergeSchemaPolicyTags`) to apply
  them back, matched by field name/path (mind nested/REPEATED STRUCT fields).

Prefer engine-owned persistence for mode + description so GET is engine-authoritative;
use the overlay only for fields the engine schema genuinely cannot represent.

### Step 4 — Make the PATCH response engine-consistent

After applying the schema update and column governance, re-read the table
(`Catalog.DescribeTable` + overlay merge — the same path `TableGet` uses) and
return **that** as the PATCH 200 body, instead of echoing the request. This
guarantees PATCH-body == next-GET and surfaces rejected changes immediately.

### Step 5 — Consider `tables.update` (PUT) parity

`TableUpdate` (`tables.go` ~411–420) only writes overlay metadata and never
touches the engine schema. If the UI ever issues PUT with a full schema, give it
the same sync path. Lower priority than PATCH.

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
