---
name: BQUtils 08 — Views corpus
overview: Add conformance coverage for the self-contained bigquery-utils views (notably views/migration/teradata/sys_calendar.sql). Audit-log views require Cloud Logging tables and stay out of scope.
depends_on: [bqutils-01-codegen-pipeline, bqutils-02-engine-build-triage]
blocks: []
est_effort: 2-3 days
isProject: true
todos:
  - id: inventory
    content: Classify views/ — self-contained (views/migration/teradata/sys_calendar.sql, uses GENERATE_DATE_ARRAY, no external tables) vs external-dep (views/audit/* need Cloud Logging audit tables).
    status: pending
  - id: fixtures
    content: Author conformance fixtures for self-contained views (CREATE VIEW in setup, SELECT from it with a deterministic ORDER BY / sampled rows, assert known output).
    status: pending
  - id: engine-check
    content: Confirm engine CREATE VIEW + SELECT round-trip and the functions the view uses (GENERATE_DATE_ARRAY, date math); record gaps.
    status: pending
  - id: docs
    content: Document view coverage + provenance in third_party/README.md.
    status: pending
---

# BQUtils 08 — Views corpus

## Why

`views/` holds plain `.sql` view definitions. Most (`views/audit/*`) require pre-populated Cloud Logging -> BigQuery audit tables and are **out of scope**. The exception is `views/migration/teradata/sys_calendar.sql`, a self-contained calendar view built from `GENERATE_DATE_ARRAY` with no external tables — a good conformance candidate.

## Steps

1. **Inventory/classify**: confirm which views are self-contained. Expect only `sys_calendar` (and possibly small derivatives).
2. **Engine check**: confirm `CREATE VIEW` + `SELECT` round-trip (there is already `simple_bigquery_view` coverage in the third_party lane) and that the view's functions (`GENERATE_DATE_ARRAY`, date extraction) are supported.
3. **Fixtures**: author fixture(s) that `setup.sql` the `CREATE VIEW`, then `query` a deterministic slice (e.g. `SELECT ... FROM sys_calendar WHERE calendar_date = DATE '2024-01-01'`) and assert known column values. Use `match: unordered` or a tight `WHERE`+`ORDER BY` to keep expectations stable.
4. Triage into `passing/` vs `known_failing/`; **docs**.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/thirdparty-fixtures/bigquery_utils/.../sys_calendar.yaml
```

## Out of scope

- `views/audit/*` (need Cloud Logging audit tables).
- Stored procedures (plan 07).
