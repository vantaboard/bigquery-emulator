---
name: FixTests 10 — bqutils RANGE<> type algebra
overview: Promote the 6 RANGE<> fixtures (cw_range_{date,datetime,timestamp}_{ldiff,rdiff}) by implementing the RANGE type, its constructors, and the range functions (RANGE_OVERLAPS, RANGE_START, RANGE_END) the bodies use. The sibling cw_period_* fixtures already pass and serve as the diff oracle.
depends_on: [fixtests-09-bqutils-bytes]
est_effort: 1-2 weeks
isProject: true
todos:
  - id: repro-diff
    content: Run the 6 cw_range_* fixtures and diff their SQL bodies against the passing cw_period_* fixtures to enumerate exactly which RANGE constructs are missing.
    status: pending
  - id: range-type
    content: Implement the RANGE<DATE|DATETIME|TIMESTAMP> type end to end (analyzer/catalog -> semantic value -> wire) for params and return values.
    status: pending
  - id: range-funcs
    content: Implement RANGE() constructor, RANGE_OVERLAPS, RANGE_START, RANGE_END (and any other range fns the bodies call).
    status: pending
  - id: triage
    content: Re-sync + triage; move the 6 fixtures to passing/; update index count and docs.
    status: pending
---

# FixTests 10 — bqutils RANGE<> type algebra

## Why

Six fixtures need the `RANGE<>` type and its function family — a genuine new type, not just a function. The closely related `cw_period_*` fixtures already pass, so the delta is bounded and discoverable by diffing the bodies.

Fixtures (community): `cw_range_date_ldiff`, `cw_range_date_rdiff`, `cw_range_datetime_ldiff`, `cw_range_datetime_rdiff`, `cw_range_timestamp_ldiff`, `cw_range_timestamp_rdiff`.

## Scope of the RANGE type

- Type plumbing: analyzer recognition + catalog, semantic value representation, REST/gRPC wire encoding for `RANGE<DATE>`, `RANGE<DATETIME>`, `RANGE<TIMESTAMP>` as params and results.
- Functions: `RANGE(...)` constructor, `RANGE_OVERLAPS`, `RANGE_START`, `RANGE_END` (confirm the exact set from the fixture bodies).

## Key files

- Type system / catalog: [`backend/catalog/`](backend/catalog/), semantic value types under [`backend/engine/semantic/`](backend/engine/semantic/).
- Wire encoding: [`gateway/bqtypes/wire.go`](gateway/bqtypes/wire.go).
- Function impls + transpiler emit for the range functions.

## Steps

1. Diff `cw_range_*` bodies vs `cw_period_*` (passing) to list missing constructs.
2. Implement the RANGE type plumbing, then the functions.
3. Re-sync + triage:

```bash
task emulator:build-engine:bazel
task conformance:bqutils-sync
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

4. Update [bqutils-00-index.plan.md](bqutils-00-index.plan.md), [`third_party/README.md`](third_party/README.md), `docs/ENGINE_POLICY.md`, and `SHAPE_TRACKER.md` if a RANGE shape routes through the transpiler.

## Out of scope

- `TIMESTAMP_BUCKET` / interval formatting (plan 12). `typeof`'s RANGE-literal case can be revisited here if plan 08 deferred it.
