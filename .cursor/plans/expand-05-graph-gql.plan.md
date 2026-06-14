---
name: Expand 05 — Graph / GQL scans
overview: Promote the GoogleSQL graph-query (GQL) surface off unsupported. Implement GRAPH_TABLE + GQL subqueries by lowering the nine ResolvedGraph*Scan classes (and the graph-internal ResolvedCatalogColumnRef) onto the semantic executor over a locally-modeled property graph defined on top of existing tables. This is the largest single-family gap; scope an MVP (node/edge/path matching) and document the long tail.
est_effort: ~5-6 weeks
isProject: true
todos:
  - id: graph-catalog
    content: "Property-graph catalog: model CREATE PROPERTY GRAPH (node/edge tables + labels + properties) in the catalog/storage layer so GRAPH_TABLE can resolve a graph by name to its backing tables. Decide persistence (mirror the view/routine registries)."
    status: pending
  - id: element-scans
    content: "ResolvedGraphElementScan / ResolvedGraphNodeScan / ResolvedGraphEdgeScan: materialize node/edge elements from the backing tables on the semantic executor, with label + property projection."
    status: pending
  - id: pattern-scans
    content: "ResolvedGraphPathScan / ResolvedGraphScan / ResolvedGraphLinearScan: pattern matching (fixed-length paths first; quantified/variable-length paths as a documented follow-up), composing node + edge scans into path bindings."
    status: pending
  - id: table-ref-call
    content: "ResolvedGraphTableScan (the GRAPH_TABLE operator feeding rows back into SQL), ResolvedGraphRefScan, and ResolvedGraphCallScan (graph-scoped calls). Bridge GQL results back into the relational world so GRAPH_TABLE(...) is queryable like a subquery."
    status: pending
  - id: catalog-column-ref
    content: "ResolvedCatalogColumnRef: the graph/catalog-internal column reference GQL scans emit. Coordinate with expand-07 (which owns the catalog-ref disposition row) — land the graph-context handling here, keep the YAML row edit in one place."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: CREATE PROPERTY GRAPH + a GRAPH_TABLE node-match, an edge/path match, and a GRAPH_TABLE feeding an outer SELECT. Flip the ResolvedGraph*Scan rows (+ ResolvedCatalogColumnRef in the graph context) off unsupported in node_dispositions.yaml + SHAPE_TRACKER; update the ENGINE_POLICY Graph row + ROADMAP §Graph / GQL scans. Document the unsupported long tail (variable-length paths, etc.)."
    status: pending
---

# Expand 05 — Graph / GQL scans

## Why

[ROADMAP.md §Graph / GQL scans](../../ROADMAP.md) tracks all nine
`ResolvedGraph*Scan` classes as ⏳ planned; ENGINE_POLICY records the
Spanner-Graph / GQL surface (and `ResolvedCatalogColumnRef`) as
`unsupported`. This is the single largest family of `unsupported` AST
nodes, so the plan scopes an **MVP** — a locally-modeled property graph
over existing tables, with node / edge / fixed-length-path matching —
and documents the long tail rather than promising the full GQL grammar.

## The hard part

GQL is a whole sub-language (`GRAPH_TABLE`, `MATCH`, path patterns,
quantified paths) layered on a property-graph data model BigQuery
doesn't store the way it stores tables. The plan must (a) define how a
property graph is declared + persisted locally, (b) compile the nine
scan classes into semantic-executor operations over the backing tables,
and (c) bridge GQL results back into relational SQL. Variable-length /
quantified path matching is the expensive part — defer it explicitly.

## Key files

- [`backend/catalog/`](../../backend/catalog/) — property-graph catalog (mirror `view_registry` / `procedure_registry`)
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — graph scan evaluation (new graph/ subpackage)
- [`backend/engine/semantic/scan_eval_scan_impl.cc`](../../backend/engine/semantic/) — relational scan eval to bridge into
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — graph-scan dispatch
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) — `ResolvedGraph*Scan` + `ResolvedCatalogColumnRef`
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Graph row

## Steps

1. Property-graph catalog (declare + persist node/edge tables).
2. Element scans (node / edge / element).
3. Pattern scans (path / scan / linear; fixed-length first).
4. `GRAPH_TABLE` / ref / call bridging back to SQL.
5. `ResolvedCatalogColumnRef` in the graph context (coordinate w/ 07).
6. Fixtures + tracker/posture flips + documented long tail.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope (this MVP)

- Variable-length / quantified path patterns (document, keep partial).
- Graph-specific shortest-path / cost functions.
- Cross-graph joins beyond what `GRAPH_TABLE` -> relational supports.
