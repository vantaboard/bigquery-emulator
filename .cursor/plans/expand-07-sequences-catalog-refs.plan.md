---
name: Expand 07 — Sequences & catalog column refs
overview: Drain the Catalog / sequence helpers rows from ROADMAP §Planned work - ResolvedSequence / NEXT VALUE FOR, ResolvedExpressionColumn, and the non-graph half of ResolvedCatalogColumnRef. Small, mostly-independent shapes that round out the analyzer-visible surface. Owns the ResolvedCatalogColumnRef disposition row that expand-05 (graph) also touches.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: sequences
    content: "ResolvedSequence / NEXT VALUE FOR: BigQuery does not ship general SQL sequences, but the analyzer surfaces the node. Decide between (a) a local sequence object (CREATE/persist a counter in storage, advance on NEXT VALUE FOR) for the cases GoogleSQL can resolve, or (b) keeping it unsupported with a clearer envelope if no real BigQuery SQL reaches it. Land whichever is correct + document."
    status: pending
  - id: expression-column
    content: "ResolvedExpressionColumn: the column reference used for standalone expression evaluation contexts (e.g. expression-only analysis, check constraints, computed contexts). Evaluate on the semantic executor by binding the named expression column from the eval context."
    status: pending
  - id: catalog-column-ref
    content: "ResolvedCatalogColumnRef: catalog-internal column references. Graph/GQL (the other consumer of this node) is out of scope (unsupported, not planned), so this plan owns the node only for the non-graph catalog-ref cases the analyzer can reach. If no non-graph SQL reaches the node, keep it unsupported with a sharper envelope instead of forcing an implementation."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures for whichever shapes land with real SQL coverage (sequence advance if implemented; expression-column eval). Flip the corresponding rows in node_dispositions.yaml + SHAPE_TRACKER; update ROADMAP §Catalog / sequence helpers + the ENGINE_POLICY Sequences row. For any shape that stays unsupported, sharpen the envelope/doc instead of flipping."
    status: pending
  - id: skip-audit
    content: "Third-party + conformance skip audit (run before declaring done, for whatever shapes actually land). Sweep the GoogleSQL `.test` corpus (conformance/googlesql-corpus/) and the bqutils known_failing/ corpus for sequence / expression-column / catalog-ref fixtures that now pass and promote them. Re-run any third-party subtest that touches these shapes; update third_party/README.md only for rows truly unblocked. For shapes that stay unsupported, leave a note rather than unskipping."
    status: pending
---

# Expand 07 — Sequences & catalog column refs

## Why

[ROADMAP.md §Catalog / sequence helpers](../../ROADMAP.md) lists
`ResolvedSequence`, `ResolvedCatalogColumnRef`, and
`ResolvedExpressionColumn` as ⏳ planned. These are small,
analyzer-visible shapes anchored on `unsupported` today to make the gap
explicit. Some may have no reachable BigQuery SQL surface — for those the
right outcome is a sharper envelope + doc, not a forced implementation.
Note: Graph / GQL is out of scope (unsupported, not planned), so the
graph use of `ResolvedCatalogColumnRef` does **not** drive this plan.

## The hard part

Deciding what is actually reachable. `ResolvedSequence` /
`NEXT VALUE FOR` is the ambiguous one: BigQuery doesn't ship general SQL
sequences, so the plan must confirm whether any analyzable query reaches
the node before building a sequence object. `ResolvedCatalogColumnRef`'s
other consumer (graph/GQL) is out of scope, so only land it if a
non-graph catalog-ref shape is actually reachable.

## Key files

- [`backend/engine/semantic/`](../../backend/engine/semantic/) — expression / column-ref evaluation + eval context binding
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — node dispatch
- [`backend/catalog/`](../../backend/catalog/) — sequence object persistence (if implemented)
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — the three rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Sequences row

## Steps

1. Investigate + decide `ResolvedSequence` reachability; implement or sharpen.
2. `ResolvedExpressionColumn` eval-context binding.
3. `ResolvedCatalogColumnRef` (non-graph) if reachable; else sharpen the envelope.
4. Fixtures (for landed shapes) + tracker/posture flips or doc sharpening.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Third-party / conformance to revisit

For whatever shapes actually land, **audit for newly-passing tests** (not
just fresh fixtures). Re-run to prove it; note shapes that stay
unsupported instead of unskipping.

- **GoogleSQL `.test` + bqutils corpus** — sweep
  `conformance/googlesql-corpus/` and
  `conformance/thirdparty-fixtures/bigquery_utils/known_failing/` for
  sequence / expression-column / catalog-ref fixtures to promote.
- **client lanes** — re-run any subtest touching these shapes; update
  `third_party/README.md` for rows truly unblocked.

## Out of scope

- The graph-context use of `ResolvedCatalogColumnRef` — Graph / GQL is
  unsupported and not planned.
- A general SQL sequence DDL surface if no BigQuery SQL reaches it.
