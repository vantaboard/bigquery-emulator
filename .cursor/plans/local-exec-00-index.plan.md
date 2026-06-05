---
name: local-exec-00-index
overview: "Sequential index for fixing query port emulator failures (baseline 20260604T220231Z: pass=545 fail=121 skip=2)"
todos:
  - id: gsql-index-01
    content: Execute local-exec-01-ddl-catalog.plan.md (12 tests)
    status: pending
  - id: gsql-index-02
    content: Execute local-exec-02-withscan-cte.plan.md (51 tests)
    status: pending
  - id: gsql-index-03
    content: Execute local-exec-03-operator-disposition.plan.md (4 tests)
    status: completed
  - id: gsql-index-04
    content: Execute local-exec-04-scan-emits.plan.md (62 tests)
    status: pending
  - id: gsql-index-05
    content: Execute local-exec-05-arrow-marshaling.plan.md (19 tests)
    status: pending
  - id: gsql-index-06
    content: Execute local-exec-06-aggregate-modifiers.plan.md (8 tests)
    status: completed
  - id: gsql-index-07
    content: Execute local-exec-07-semantic-core-expr.plan.md (27 tests)
    status: pending
  - id: gsql-index-08
    content: Execute local-exec-08-semantic-operators.plan.md (30 tests)
    status: pending
  - id: gsql-index-09
    content: Execute local-exec-09-date-time.plan.md (158 tests)
    status: pending
  - id: gsql-index-10
    content: Execute local-exec-10-string-hash-format.plan.md (83 tests)
    status: pending
  - id: gsql-index-11
    content: Execute local-exec-11-json.plan.md (42 tests)
    status: pending
  - id: gsql-index-12
    content: Execute local-exec-12-arrays-generators.plan.md (20 tests)
    status: pending
  - id: gsql-index-13
    content: Execute local-exec-13-advanced-relational.plan.md (3 tests)
    status: pending
  - id: gsql-index-14
    content: Execute local-exec-14-dml-system.plan.md (6 tests)
    status: pending
  - id: gsql-index-15
    content: Execute local-exec-15-specialized-stubs.plan.md (24 tests)
    status: pending
  - id: gsql-index-16
    content: Execute local-exec-16-result-fixes.plan.md (19 tests)
    status: pending
isProject: false
---

# query port Emulator Conformance Index

## Goal

Drive the ported query port suite to green by executing plans 01→16 in order. Each plan owns a failure theme from the baseline run.

## Baseline

- Stamp: `20260604T220231Z` (post Phase 0: no cascade; real failures only)
- pass=545 fail=121 skip=2
- Summary: [`gateway/e2e/testresults/query-port-20260604T220231Z-summary.txt`](../../gateway/e2e/testresults/query-port-20260604T220231Z-summary.txt)
- Re-run: [`gateway/e2e/testresults/run_query_port_tests.sh`](../../gateway/e2e/testresults/run_query_port_tests.sh)

## Execution order

1. [`local-exec-01-ddl-catalog.plan.md`](local-exec-01-ddl-catalog.plan.md) — DDL / Catalog / Gateway SQL (~12 tests)
2. [`local-exec-02-withscan-cte.plan.md`](local-exec-02-withscan-cte.plan.md) — WithScan / CTE Emit (~51 tests)
3. [`local-exec-03-operator-disposition.plan.md`](local-exec-03-operator-disposition.plan.md) — Operator Dispositions (~4 tests)
4. [`local-exec-04-scan-emits.plan.md`](local-exec-04-scan-emits.plan.md) — Scan Emits (ProjectScan, OrderByScan, …) (~62 tests)
5. [`local-exec-05-arrow-marshaling.plan.md`](local-exec-05-arrow-marshaling.plan.md) — Arrow → BigQuery Result Marshaling (~19 tests)
6. [`local-exec-06-aggregate-modifiers.plan.md`](local-exec-06-aggregate-modifiers.plan.md) — Aggregate Modifiers (ORDER BY / LIMIT / HAVING) (~8 tests)
7. [`local-exec-07-semantic-core-expr.plan.md`](local-exec-07-semantic-core-expr.plan.md) — Semantic Core Expressions / Scan Walk (~27 tests)
8. [`local-exec-08-semantic-operators.plan.md`](local-exec-08-semantic-operators.plan.md) — Semantic Internal Operators (~30 tests)
9. [`local-exec-09-date-time.plan.md`](local-exec-09-date-time.plan.md) — Semantic Date / Time Functions (~158 tests)
10. [`local-exec-10-string-hash-format.plan.md`](local-exec-10-string-hash-format.plan.md) — Semantic String / Hash / Format (~83 tests)
11. [`local-exec-11-json.plan.md`](local-exec-11-json.plan.md) — Semantic JSON Functions (~42 tests)
12. [`local-exec-12-arrays-generators.plan.md`](local-exec-12-arrays-generators.plan.md) — Semantic Arrays / Generators (~20 tests)
13. [`local-exec-13-advanced-relational.plan.md`](local-exec-13-advanced-relational.plan.md) — Advanced Relational (GROUPING SETS, PIVOT, …) (~3 tests)
14. [`local-exec-14-dml-system.plan.md`](local-exec-14-dml-system.plan.md) — DML / System Variables / information_schema (~6 tests)
15. [`local-exec-15-specialized-stubs.plan.md`](local-exec-15-specialized-stubs.plan.md) — Specialized / NET / UDF Stubs (~24 tests)
16. [`local-exec-16-result-fixes.plan.md`](local-exec-16-result-fixes.plan.md) — Result / Driver Fixes (~19 tests)

## Global verify

```bash
./gateway/e2e/testresults/run_query_port_tests.sh
```

## Subagent execution

Run plans 01→16 sequentially via background subagents. The parent agent
orchestrates; each subagent owns one themed plan.

**Start here:** [`local-exec-subagent-dispatch.plan.md`](local-exec-subagent-dispatch.plan.md)

## Regenerate plan test lists

```bash
python3 tools/local_exec/plan_from_testresults.py \
  --json gateway/e2e/testresults/query-port-latest.json \
  --emit-plans
```

## Done when

- Summary shows `fail=0` (or only intentional skips: `TestFilterFieldsProto`, `TestAnonymizedDPAggregate`).
