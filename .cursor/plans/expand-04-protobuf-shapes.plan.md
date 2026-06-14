---
name: Expand 04 — Protobuf field access
overview: Promote the GoogleSQL proto type surface off unsupported. Implement the resolved-AST proto shapes - ResolvedMakeProto, ResolvedGetProtoField, ResolvedGetProtoOneof, ResolvedReplaceField, ResolvedGetRowField, ResolvedFilterField, ResolvedFilterFieldArg - on the semantic executor so PROTO-typed columns/expressions construct, read, and mutate fields locally with BigQuery semantics.
est_effort: ~3-4 weeks
isProject: true
todos:
  - id: proto-type-plumbing
    content: "PROTO type plumbing: decide the storage + REST wire encoding for PROTO values (BigQuery surfaces proto fields as STRUCT-shaped; the type carries a descriptor). Map PROTO <-> a storable representation in DuckDBStorage and define how the analyzer's descriptor pool is sourced (GoogleSQL resolves against a descriptor). Reconcile with the existing STRUCT lowering."
    status: pending
  - id: construct-read
    content: "ResolvedMakeProto (construct a proto value from field args) and ResolvedGetProtoField / ResolvedGetProtoOneof (read a field / determine the set oneof) on the semantic executor, with default-value + presence (has_) semantics matching proto2/proto3 rules."
    status: pending
  - id: mutate
    content: "ResolvedReplaceField (REPLACE_FIELDS) and ResolvedFilterField / ResolvedFilterFieldArg (FILTER_FIELDS include/exclude) — field-path mutation/projection on proto (and struct) values, reusing the deep-STRUCT mutation primitives from the DML executor where possible."
    status: pending
  - id: get-row-field
    content: "ResolvedGetRowField — row/value field access used in the proto + value-table context; align with the existing GetStructField path."
    status: pending
  - id: routing
    content: "Route classifier: these are expression nodes; confirm a query containing any of them promotes to the semantic executor (per the priority order) and that descriptor-less / unsupported proto shapes still surface a clear UNIMPLEMENTED rather than crashing."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: proto construct + field read, oneof, REPLACE_FIELDS, FILTER_FIELDS, GetRowField. Flip the proto rows in node_dispositions.yaml (unsupported -> semantic_executor) + SHAPE_TRACKER + the ENGINE_POLICY Protobuf row; update ROADMAP §Protobuf field access."
    status: pending
  - id: skip-audit
    content: "Third-party + conformance skip audit (run before declaring done). Check whether any currently-skipped or known_failing proto cases now pass and promote them: widen the GoogleSQL `.test` corpus lane (conformance/googlesql-corpus/) with proto cases the implementation covers; sweep the bqutils corpus known_failing/ for proto-shaped fixtures (conformance/thirdparty-fixtures/bigquery_utils/known_failing/); re-run any third-party subtests that touch proto columns. Update third_party/README.md only for rows actually unblocked + note anything still failing."
    status: pending
---

# Expand 04 — Protobuf field access

## Why

[ROADMAP.md §Protobuf field access](../../ROADMAP.md) tracks the proto
AST family as ⏳ planned; ENGINE_POLICY records "the emulator does not
model the GoogleSQL proto type surface end-to-end." The
`node_dispositions.yaml` rows `ResolvedMakeProto`, `ResolvedGetProtoField`,
`ResolvedGetProtoOneof`, `ResolvedReplaceField`, `ResolvedGetRowField`,
`ResolvedFilterField`, `ResolvedFilterFieldArg` are all `unsupported`, so
any PROTO-typed query surfaces `UNIMPLEMENTED`.

## The hard part

PROTO is a first-class GoogleSQL type backed by a descriptor pool the
analyzer resolves against; DuckDB has no native proto type. The plan must
(a) decide where descriptors come from and (b) map PROTO onto a storable
+ wire-encodable representation (BigQuery exposes proto fields with
STRUCT-like access and proto2/proto3 presence + default rules). Field
mutation (`REPLACE_FIELDS` / `FILTER_FIELDS`) can reuse the deep-STRUCT
mutation primitives already in the semantic DML executor.

## Key files

- [`backend/engine/semantic/`](../../backend/engine/semantic/) — expression evaluation (new proto_funcs / eval unit)
- [`backend/engine/semantic/eval_expr_update_constructor.cc`](../../backend/engine/semantic/) — deep field mutation to reuse
- [`backend/engine/duckdb/transpiler/types.cc`](../../backend/engine/duckdb/transpiler/types.cc) — PROTO type lowering
- [`backend/catalog/googlesql_catalog.{h,cc}`](../../backend/catalog/) — descriptor pool sourcing
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — promotion to semantic executor
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — proto rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Protobuf row

## Steps

1. PROTO type plumbing + descriptor sourcing + storage/wire encoding.
2. Construct (`MakeProto`) + read (`GetProtoField` / `GetProtoOneof`).
3. Mutate / project (`ReplaceField`, `FilterField`/`FilterFieldArg`).
4. `GetRowField`.
5. Routing + descriptor-less `UNIMPLEMENTED` guard.
6. Fixtures + tracker/posture flips.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Third-party / conformance to revisit

When the proto surface lands, **audit for newly-passing tests** — don't
just add fresh fixtures. Re-run to prove it; note anything still failing.

- **GoogleSQL `.test` corpus** — widen `conformance/googlesql-corpus/`
  with proto cases the implementation now covers.
- **bqutils corpus** — sweep
  `conformance/thirdparty-fixtures/bigquery_utils/known_failing/` for
  proto-shaped fixtures that can move to `passing/`.
- **client lanes** — re-run any subtest touching proto columns; update
  `third_party/README.md` for rows actually unblocked.

## Out of scope

- User-supplied `.proto` upload/registration UX beyond what GoogleSQL's
  descriptor pool already exposes to the analyzer.
- Proto extensions / `Any` packing if they require descriptor machinery
  not available locally — leave `unsupported` + documented.
