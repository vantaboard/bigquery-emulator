# Core BigQuery usage conformance fixtures

Hand-authored fixtures that exercise everyday client patterns: views,
backtick-qualified names, DDL lifecycle, default-dataset resolution,
DML read-back, and common SQL shapes (joins, aggregates, windows, etc.).

## Production validation required

**Do not bootstrap `expected:` rows from emulator output.** Every fixture
in this tree must have `expected.rows` (or `expected.error`) derived from
production BigQuery semantics, validated with the `bq` CLI when GCP access
is available (see `.cursor/rules/conformance-bq-validation.mdc`).

All fixtures set `verified_production: true`. The conformance runner and
`task conformance:update-baselines` refuse to rewrite protected fixtures.

## Layout

| Directory | Coverage |
|-----------|----------|
| `views/` | CREATE VIEW, read rows, filters, OR REPLACE, joins/aggregates, nested views, backtick names, DROP |
| `qualified_names/` | `` `dataset.table` ``, `` `project.dataset.table` `` for SELECT/DDL/DML |
| `ddl_lifecycle/` | CREATE OR REPLACE, CTAS read-back, IF NOT EXISTS, ALTER, DROP IF EXISTS |
| `default_dataset/` | Bare table names with/without request `defaultDataset` |
| `dml_readback/` | INSERT/UPDATE/DELETE/MERGE/TRUNCATE with post-state SELECT |
| `everyday_sql/` | Joins, aggregates, windows, QUALIFY, set ops, CTEs, UNNEST, scalars, scripting; `cache_shape_*` dedup-then-downstream-op templates (DISTINCT, view-backed cache reads) |

These fixtures omit `expected.route` so they assert **behavior** (rows/errors),
not internal routing disposition.
