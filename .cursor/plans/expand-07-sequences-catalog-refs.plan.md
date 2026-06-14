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
    content: "ResolvedCatalogColumnRef (non-graph): catalog-internal column references. Coordinate with expand-05 — this plan owns the node_dispositions.yaml row edit; if 05 lands first in the graph context, reconcile so the row reflects both. Implement the non-graph binding on the semantic executor."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures for whichever shapes land with real SQL coverage (sequence advance if implemented; expression-column eval). Flip the corresponding rows in node_dispositions.yaml + SHAPE_TRACKER; update ROADMAP §Catalog / sequence helpers + the ENGINE_POLICY Sequences row. For any shape that stays unsupported, sharpen the envelope/doc instead of flipping."
    status: pending
---

# Expand 07 — Sequences & catalog column refs

## Why

[ROADMAP.md §Catalog / sequence helpers](../../ROADMAP.md) lists
`ResolvedSequence`, `ResolvedCatalogColumnRef`, and
`ResolvedExpressionColumn` as ⏳ planned. These are small,
analyzer-visible shapes anchored on `unsupported` today to make the gap
explicit. Some may have no reachable BigQuery SQL surface (sequences in
particular) — for those the right outcome is a sharper envelope + doc,
not a forced implementation.

## The hard part

Deciding what is actually reachable. `ResolvedSequence` /
`NEXT VALUE FOR` is the ambiguous one: BigQuery doesn't ship general SQL
sequences, so the plan must confirm whether any analyzable query reaches
the node before building a sequence object. `ResolvedCatalogColumnRef`
is shared with the graph plan (05); this plan owns the YAML row so the
two don't double-edit it.

## Key files

- [`backend/engine/semantic/`](../../backend/engine/semantic/) — expression / column-ref evaluation + eval context binding
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — node dispatch
- [`backend/catalog/`](../../backend/catalog/) — sequence object persistence (if implemented)
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — the three rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Sequences row

## Steps

1. Investigate + decide `ResolvedSequence` reachability; implement or sharpen.
2. `ResolvedExpressionColumn` eval-context binding.
3. `ResolvedCatalogColumnRef` (non-graph); reconcile with plan 05.
4. Fixtures (for landed shapes) + tracker/posture flips or doc sharpening.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- The graph-context use of `ResolvedCatalogColumnRef` (lives in plan 05;
  this plan only owns the shared disposition row).
- A general SQL sequence DDL surface if no BigQuery SQL reaches it.
