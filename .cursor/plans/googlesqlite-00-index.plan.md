---
name: googlesqlite-00-index
overview: "Sequential index for fixing googlesqlite emulator port failures (baseline 20260604T220231Z: pass=545 fail=121 skip=2)"
todos:
  - id: gsql-index-01
    content: Execute googlesqlite-01-ddl-catalog.plan.md (12 tests)
    status: pending
  - id: gsql-index-02
    content: Execute googlesqlite-02-withscan-cte.plan.md (51 tests)
    status: pending
  - id: gsql-index-03
    content: Execute googlesqlite-03-operator-disposition.plan.md (4 tests)
    status: completed
  - id: gsql-index-04
    content: Execute googlesqlite-04-scan-emits.plan.md (62 tests)
    status: pending
  - id: gsql-index-05
    content: Execute googlesqlite-05-arrow-marshaling.plan.md (19 tests)
    status: pending
  - id: gsql-index-06
    content: Execute googlesqlite-06-aggregate-modifiers.plan.md (8 tests)
    status: completed
  - id: gsql-index-07
    content: Execute googlesqlite-07-semantic-core-expr.plan.md (27 tests)
    status: pending
  - id: gsql-index-08
    content: Execute googlesqlite-08-semantic-operators.plan.md (30 tests)
    status: pending
  - id: gsql-index-09
    content: Execute googlesqlite-09-date-time.plan.md (158 tests)
    status: pending
  - id: gsql-index-10
    content: Execute googlesqlite-10-string-hash-format.plan.md (83 tests)
    status: pending
  - id: gsql-index-11
    content: Execute googlesqlite-11-json.plan.md (42 tests)
    status: pending
  - id: gsql-index-12
    content: Execute googlesqlite-12-arrays-generators.plan.md (20 tests)
    status: pending
  - id: gsql-index-13
    content: Execute googlesqlite-13-advanced-relational.plan.md (3 tests)
    status: pending
  - id: gsql-index-14
    content: Execute googlesqlite-14-dml-system.plan.md (6 tests)
    status: pending
  - id: gsql-index-15
    content: Execute googlesqlite-15-specialized-stubs.plan.md (24 tests)
    status: pending
  - id: gsql-index-16
    content: Execute googlesqlite-16-result-fixes.plan.md (19 tests)
    status: pending
isProject: false
---

# googlesqlite Emulator Conformance Index

## Goal

Drive the ported googlesqlite query suite to green by executing plans 01→16 in order. Each plan owns a failure theme from the baseline run.

## Baseline

- Stamp: `20260604T220231Z` (post Phase 0: no cascade; real failures only)
- pass=545 fail=121 skip=2
- Summary: [`gateway/e2e/testresults/googlesqlite-emulator-20260604T220231Z-summary.txt`](../../gateway/e2e/testresults/googlesqlite-emulator-20260604T220231Z-summary.txt)
- Re-run: [`gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh`](../../gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh)

## Execution order

1. [`googlesqlite-01-ddl-catalog.plan.md`](googlesqlite-01-ddl-catalog.plan.md) — DDL / Catalog / Gateway SQL (~12 tests)
2. [`googlesqlite-02-withscan-cte.plan.md`](googlesqlite-02-withscan-cte.plan.md) — WithScan / CTE Emit (~51 tests)
3. [`googlesqlite-03-operator-disposition.plan.md`](googlesqlite-03-operator-disposition.plan.md) — Operator Dispositions (~4 tests)
4. [`googlesqlite-04-scan-emits.plan.md`](googlesqlite-04-scan-emits.plan.md) — Scan Emits (ProjectScan, OrderByScan, …) (~62 tests)
5. [`googlesqlite-05-arrow-marshaling.plan.md`](googlesqlite-05-arrow-marshaling.plan.md) — Arrow → BigQuery Result Marshaling (~19 tests)
6. [`googlesqlite-06-aggregate-modifiers.plan.md`](googlesqlite-06-aggregate-modifiers.plan.md) — Aggregate Modifiers (ORDER BY / LIMIT / HAVING) (~8 tests)
7. [`googlesqlite-07-semantic-core-expr.plan.md`](googlesqlite-07-semantic-core-expr.plan.md) — Semantic Core Expressions / Scan Walk (~27 tests)
8. [`googlesqlite-08-semantic-operators.plan.md`](googlesqlite-08-semantic-operators.plan.md) — Semantic Internal Operators (~30 tests)
9. [`googlesqlite-09-date-time.plan.md`](googlesqlite-09-date-time.plan.md) — Semantic Date / Time Functions (~158 tests)
10. [`googlesqlite-10-string-hash-format.plan.md`](googlesqlite-10-string-hash-format.plan.md) — Semantic String / Hash / Format (~83 tests)
11. [`googlesqlite-11-json.plan.md`](googlesqlite-11-json.plan.md) — Semantic JSON Functions (~42 tests)
12. [`googlesqlite-12-arrays-generators.plan.md`](googlesqlite-12-arrays-generators.plan.md) — Semantic Arrays / Generators (~20 tests)
13. [`googlesqlite-13-advanced-relational.plan.md`](googlesqlite-13-advanced-relational.plan.md) — Advanced Relational (GROUPING SETS, PIVOT, …) (~3 tests)
14. [`googlesqlite-14-dml-system.plan.md`](googlesqlite-14-dml-system.plan.md) — DML / System Variables / information_schema (~6 tests)
15. [`googlesqlite-15-specialized-stubs.plan.md`](googlesqlite-15-specialized-stubs.plan.md) — Specialized / NET / UDF Stubs (~24 tests)
16. [`googlesqlite-16-result-fixes.plan.md`](googlesqlite-16-result-fixes.plan.md) — Result / Driver Fixes (~19 tests)

## Global verify

```bash
./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh
```

## Subagent execution

Run plans 01→16 sequentially via background subagents. The parent agent
orchestrates; each subagent owns one themed plan.

**Start here:** [`googlesqlite-subagent-dispatch.plan.md`](googlesqlite-subagent-dispatch.plan.md)

## Regenerate plan test lists

```bash
python3 tools/googlesqlite/plan_from_testresults.py \
  --json gateway/e2e/testresults/googlesqlite-emulator-latest.json \
  --emit-plans
```

## Done when

- Summary shows `fail=0` (or only intentional skips: `TestFilterFieldsProto`, `TestAnonymizedDPAggregate`).
