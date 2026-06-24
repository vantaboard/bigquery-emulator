---
name: Seed STRUCT JSON regression
overview: Regression guard ensuring seed loader REPEATED STRUCT+JSON columns insert correctly on docker compose startup.
todos:
  - id: yaml-e2e-seed
    content: Add end-to-end seed YAML test with REPEATED STRUCT JSON fixture
    status: completed
  - id: keep-unit-tests
    content: Maintain TestCatalogApplier_InsertRows_RepeatedStruct coverage
    status: completed
isProject: false
---

# 08 — Seed loader REPEATED STRUCT + JSON (regression guard)

- **Scope:** local dev ergonomics (not UI inventory #1–17)
- **Verified state at HEAD (`60d19b3e`):** **Fixed** — regression guard only.

## Original symptom (UI brief, v0.5.0)

Seed YAML:
```yaml
structarr:
  - key: profile
    value: '{"age": 10}'
```
where `structarr` is `REPEATED STRUCT<key STRING, value JSON>`. Startup failed:
```
InsertRows ... Conversion Error: Malformed JSON ... Input: "profile"
```
The generated struct swapped key/value:
`{'key': '{"age": 10}', 'value': 'profile'}`.

## Root cause (already fixed)

`gateway/seed/applier.go` `cellFromJSONForField` previously checked
`isStructFieldType(STRUCT)` **before** the REPEATED mode, so a REPEATED STRUCT
array (`[]any`) failed the `map[string]any` cast and fell through to
`ValueToCell`. `ValueToCell`'s `map[string]any` branch iterates **values only**
(Go map iteration order is non-deterministic) and assigns them to positional
struct slots — swapping `key`/`value` and feeding `"profile"` to a JSON column.

Fix (present at HEAD): handle the REPEATED branch first, then map STRUCT element
maps **by field name** (`f.GetFields()` → `m[sub.GetName()]`). The parallel REST
path `gateway/handlers/tabledata.go` `jsonCellForField` was fixed in the same
commit. Covered by `TestCatalogApplier_InsertRows_RepeatedStruct`
(`gateway/seed/applier_test.go`) and `TestTableDataInsertAllRepeatedStruct`
(`gateway/handlers/tabledata_test.go`).

## Residual risk

`ValueToCell` / `jsonToCell` remain **schema-blind** for `map[string]any` (the
code comment at `gateway/seed/applier.go` ~252–254 documents this). Any future
caller that hits a STRUCT value without going through the name-mapped path
(nil schema, wrong Go type, a new code path missing the REPEATED branch) can
reintroduce the swap.

## Goal / done-criteria

1. The original repro is locked by a test at the **`seedfile` YAML layer**
   (today only the `seed`/`handlers` applier layers are covered — there is no
   end-to-end YAML→Apply→InsertRows wire-shape assertion).
2. A short note (code comment or `docs/SEEDING.md`) warns that STRUCT values must
   flow through the name-mapped path, not `ValueToCell`.

## Implementation steps

### Step 1 — Verify current behavior

```bash
go test ./gateway/seed/... ./gateway/handlers/... -run 'RepeatedStruct'
```
Both tests should pass at HEAD. If on a pre-`684aad2` branch, port the fix:
add the REPEATED-mode branch before the STRUCT branch in `cellFromJSONForField`
and a `repeatedElementSchema` helper that clears mode but keeps type + nested
fields (mirror `jsonCellForField` in `tabledata.go`).

### Step 2 — Add the missing end-to-end YAML test

In `gateway/seedfile/seedfile_test.go`, add `TestApply_RepeatedStructJSONSubfield`:
decode YAML with a `structarr` REPEATED STRUCT<key STRING, value JSON> row,
run through `seedfile.Apply` with a capturing fake `CatalogApplier`, and assert
the proto cells:
- `array[0].struct.fields[0]` == `"profile"`
- `array[0].struct.fields[1]` == `{"age": 10}`

Optionally add `TestDecode_RepeatedStructRow` confirming the YAML decodes to
`[]any{map[string]any{...}}` before apply.

### Step 3 — Harden the schema-blind helper (optional)

Make `ValueToCell` / `jsonToCell` log or reject STRUCT `map[string]any` values
when no schema is available (fail loud instead of silently swapping), so future
regressions surface immediately rather than as a confusing JSON error.

## Out of scope

- Broader seed-format changes; this is strictly the REPEATED STRUCT + JSON path.

## Touch list

`gateway/seedfile/seedfile_test.go` (new test), optionally
`gateway/seed/applier.go` (comment/guard) and `docs/SEEDING.md`.
