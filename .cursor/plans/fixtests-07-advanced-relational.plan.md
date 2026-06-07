---
name: FixTests 07 — Advanced relational (pivot, unpivot, recursive CTE)
overview: Close the last three first-party conformance failures - PIVOT, UNPIVOT, and WITH RECURSIVE - all dispositioned to duckdb_rewrite. This is the largest first-party lift and the final gate to conformance 100/100.
depends_on: [fixtests-06-fastpath-scans]
est_effort: 2-3 weeks
isProject: true
todos:
  - id: pivot
    content: Fix pivot - EmitPivotScan FILTER-aggregate lowering (each ResolvedPivotColumn -> <agg> FILTER (WHERE for_expr = value)) with correct output column names and group_by passthrough.
    status: pending
  - id: unpivot
    content: Fix unpivot - EmitUnpivotScan UNION ALL per unpivot_arg_list entry, value/label columns, EXCLUDE NULLS default.
    status: pending
  - id: recursive-cte
    content: Fix recursive_cte - EmitRecursiveRefScan anchor/ref column renaming so the recursive arm composes with surrounding scans.
    status: pending
  - id: verify
    content: All three pass; task conformance:run hits 100/100 and the CI conformance job goes green.
    status: pending
---

# FixTests 07 — Advanced relational (pivot, unpivot, recursive CTE)

## Why

The three advanced-relational fixtures ([`conformance/fixtures/advanced_relational/`](conformance/fixtures/advanced_relational/)) are the largest first-party gap. The engine deliberately disables `REWRITE_PIVOT` / `REWRITE_UNPIVOT` so the analyzer hands the transpiler raw `ResolvedPivotScan` / `ResolvedUnpivotScan` / `ResolvedRecursiveScan` nodes, and the emit code for each exists but is fragile at runtime. Closing these reaches **conformance 100/100**.

> Validate each against the observed diff from [fixtests-01-foundation.plan.md](fixtests-01-foundation.plan.md) (row mismatch vs transpile error).

## Fixtures and work

| Fixture | Shape | Work | Files (per SHAPE_TRACKER) |
|---------|-------|------|---------------------------|
| `pivot` | `ResolvedPivotScan` -> `duckdb_rewrite` | Lower each `ResolvedPivotColumn` (`(pivot_expr_index, pivot_value_index)` + analyzer-chosen name) to `<agg> FILTER (WHERE <for_expr> = <pivot_value>)` over the input scan, with `group_by_list` passthrough. Diagnose column-name / aggregation divergence at runtime. | `EmitPivotScan` |
| `unpivot` | `ResolvedUnpivotScan` -> `duckdb_rewrite` | One `UNION ALL` branch per `unpivot_arg_list` entry: project pass-through cols, rename arg col refs to `value_column_list`, add `label_list[i]` literal under `label_column`; `EXCLUDE NULLS` (BQ default) emits `WHERE NOT (val IS NULL AND ...)`. Fix label/value column mismatch. | `EmitUnpivotScan` |
| `recursive_cte` | `ResolvedRecursiveScan` / `ResolvedRecursiveRefScan` -> `duckdb_rewrite` | `EmitRecursiveRefScan` reads the back of `recursive_cte_stack_` for the active CTE + anchor names and renames anchors back to per-ref column names so surrounding scans compose. Harden the anchor/ref renaming. | `transpiler_emit_with.cc` |

See [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) rows for `ResolvedPivotScan`, `ResolvedUnpivotScan`, `ResolvedRecursiveRefScan`.

## Steps

1. Reproduce each; capture whether it is a transpile failure (raw node not lowered) or a row mismatch (lowering produces wrong values/columns).
2. Fix emit per the table; add intermediate `cc_test` coverage in the transpiler test suite for the lowering where feasible (cheaper than full conformance iteration).
3. Update `SHAPE_TRACKER.md` notes and keep `tools/check_disposition_parity` green.

## Verify

```bash
task emulator:build-engine:bazel
for f in pivot unpivot recursive_cte; do
  task conformance:run-fixture FIXTURE=conformance/fixtures/advanced_relational/$f.yaml
done
task conformance:run             # target 100/100
```

Then confirm CI [`conformance.yml`](.github/workflows/conformance.yml) `conformance` job is green and `googlesql-parity` source leg's full-conformance tier passes (feeds plan 04).

## Out of scope

- bqutils corpus (Track B); third-party suites (plan 14).
