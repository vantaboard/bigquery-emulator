---
name: materialized view schema resolution
overview: "Fix the engine-side catalog gap so `SELECT * FROM <mview>` resolves the materialized view's output schema instead of collapsing to zero columns. Engine work in `backend/catalog/`; unblocks `QueryMaterializedViewIT`."
todos:
  - id: tp13_repro
    content: "Reproduce against a minimal fixture: CREATE TABLE base (a INT64, b STRING, c BOOL); CREATE MATERIALIZED VIEW mv AS SELECT * FROM base; SELECT * FROM mv;"
    status: pending
  - id: tp13_root_cause
    content: "Trace where the engine forgets the materialized view's column list. Likely places: backend/catalog/ materialized-view registration; the analyzer's view-resolution code path."
    status: pending
  - id: tp13_fix
    content: "Wire the materialized view's resolved output schema into the catalog at CREATE time and surface it from the catalog reader the analyzer consults."
    status: pending
  - id: tp13_dml_refresh
    content: "Confirm BQ MV refresh semantics (lazy / scheduled / manual REFRESH MATERIALIZED VIEW). At minimum, lazy: re-resolve on each SELECT until a real cache lands."
    status: pending
  - id: tp13_tests
    content: "Add engine-side fixture under conformance/fixtures/ for the SELECT * regression; rerun `task thirdparty:java-bigquery-tests -- -Dit.test=QueryMaterializedViewIT`."
    status: pending
  - id: tp13_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp13` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 13 — materialized view output-schema resolution

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 3.2 (now deleted).
- Failing log line `.logs/thirdparty-20260602-134739.log:3306`: the
  IT creates a base table with 3 columns, creates a materialized
  view over it
  (`third_party/java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/QueryMaterializedViewIT.java:75-83`),
  then queries `SELECT * FROM <mview>` and the engine cannot resolve
  the view's output schema (`SELECT * would expand to zero columns`).
- Out of scope at the gateway level — this is engine work in
  `backend/catalog/`.

## Prerequisites

None — engine-side and independent of the gateway plans.

## Scope

Reproduce, root-cause, and fix the schema-resolution gap. Strict
no-silent-approximation rule: do not paper over the analyzer error
with a stub schema; the materialized view's output schema must be
the resolved column list of its defining query.

Out of scope for this plan:

- Materialized-view caching / incremental refresh policies.
- Differential refresh on base-table changes.
- View partition pruning.

Those are follow-on plans once the basic schema resolution lands.

## Implementation sketch

The exact fix depends on where the column list is dropped. Likely
shapes:

1. **At CREATE.** `CREATE MATERIALIZED VIEW` registers the view's
   name + defining SQL but not the resolved output schema. Fix:
   resolve the defining SELECT at CREATE time and persist the
   resolved column list alongside the view definition in
   `backend/catalog/`.

2. **At SELECT.** The analyzer attempts to resolve the view's
   schema lazily but fails because the catalog reader does not
   expose materialized-view columns. Fix: surface them in the
   reader so the analyzer's view-resolution path works.

3. **Both.** Resolve at CREATE, persist, and surface at SELECT.

Pick the smallest fix that holds for the minimal reproducer below,
then re-run the IT.

### Minimal reproducer

```sql
CREATE TABLE base (a INT64, b STRING, c BOOL);
INSERT INTO base VALUES (1, 'x', TRUE), (2, 'y', FALSE);
CREATE MATERIALIZED VIEW mv AS SELECT * FROM base;
SELECT * FROM mv;
```

Expected: three rows times three columns. Actual today: error
about zero columns.

## Tests

- Engine-side fixture under `conformance/fixtures/catalog/`
  covering CREATE MATERIALIZED VIEW + SELECT *.
- Unit test for the catalog reader path that surfaces MV columns.
- `task thirdparty:java-bigquery-tests -- -Dit.test=QueryMaterializedViewIT`
  passes.

## Done criteria

- `SELECT * FROM <mview>` returns the defining query's columns.
- The conformance fixture pins the route + schema.
- `QueryMaterializedViewIT` passes.
- `thirdparty-00-completion-index.plan.md` todo `tp13` flipped to
  `completed`.
