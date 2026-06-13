---
name: Full 04 — Wildcard tables & _TABLE_SUFFIX
overview: Finish wildcard-table querying (`FROM \`dataset.events_*\``) on top of the existing WildcardTable VirtualCatalogTable scaffold - resolve the matching set of tables, union their rows under a common schema, expose the _TABLE_SUFFIX pseudo-column, and honor _TABLE_SUFFIX predicate pruning.
est_effort: ~1 week
isProject: true
todos:
  - id: assess-scaffold
    content: "Read backend/catalog/wildcard_table.{h,cc} and determine what's wired vs missing: table-set resolution from a prefix pattern, schema unification across matched tables, MaterializeInDuckDB union, and _TABLE_SUFFIX synthesis."
    status: pending
  - id: resolve-match-set
    content: "Resolve the wildcard pattern (`prefix*`) to the matching tables in the dataset at analyze time; register the WildcardTable as a googlesql::Table whose schema is the unified column set (BigQuery uses the most-recent matching table's schema as the basis)."
    status: pending
  - id: union-materialize
    content: "MaterializeInDuckDB unions the matched tables' rows, padding columns absent from individual tables with NULL, and adds the _TABLE_SUFFIX column carrying each row's source-table suffix."
    status: pending
  - id: suffix-pruning
    content: "Honor `WHERE _TABLE_SUFFIX BETWEEN/=/IN ...` by pruning the matched-table set before materializing (correctness-preserving optimization; required for the BigQuery idiom to be usable on large date-sharded sets)."
    status: pending
  - id: empty-and-errors
    content: "Match BigQuery edge behavior: zero matches -> error referencing the pattern; schema-incompatible matches -> documented behavior; pseudo-column only present for wildcard scans."
    status: pending
  - id: fixtures-trackers
    content: "conformance/fixtures/wildcard/ fixtures (multi-table union, _TABLE_SUFFIX filter, single-match); update SHAPE_TRACKER (TableScan note / new virtual-table row), ROADMAP, ENGINE_POLICY; drop dbt `wildcard` skip-matrix entry and re-run."
    status: pending
---

# Full 04 — Wildcard tables & _TABLE_SUFFIX

## Why

Wildcard tables (`FROM \`project.dataset.events_*\``) with
`_TABLE_SUFFIX` filtering are the standard pattern for querying
date-sharded / log datasets. A `WildcardTable` scaffold already exists
(`backend/catalog/wildcard_table.{h,cc}`, subclassing
`VirtualCatalogTable`), so this is finishing work rather than greenfield.
The `dbt-bigquery-tests` skip matrix lists `wildcard`.

## Key files

- [`backend/catalog/wildcard_table.{h,cc}`](../../backend/catalog/) — the scaffold to finish
- [`backend/catalog/virtual_table.h`](../../backend/catalog/virtual_table.h) — `MaterializeInDuckDB` union hook
- [`backend/catalog/googlesql_catalog.cc`](../../backend/catalog/googlesql_catalog.cc) — pattern resolution at analyze time
- [`backend/storage/storage.h`](../../backend/storage/storage.h) — table enumeration for the match set
- `docs/bigquery/docs/querying-wildcard-tables.md` — semantics reference

## Steps

1. Inventory the scaffold; list the missing pieces.
2. Pattern → match-set resolution + unified schema registration.
3. Union materialization with NULL-padding + `_TABLE_SUFFIX` synthesis.
4. `_TABLE_SUFFIX` predicate pruning (prune the set, then materialize).
5. Edge behavior (zero match error, pseudo-column scoping).
6. Fixtures + tracker/doc updates + dbt skip-row removal.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/fixtures/wildcard/<new>.yaml
task conformance:run
task lint:dispositions
task thirdparty:dbt-bigquery-tests
task bazel:shutdown && task bazel:status
```

## Out of scope

- Cross-dataset / cross-project wildcards.
- Partition pruning interplay with full-03 partitioning metadata
  (coordinate if both land, but suffix pruning is the deliverable here).
