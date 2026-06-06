---
name: Unblock 02 — Public-data seed expand
overview: Expand usa_names fixture to 100+ TX rows and add usa_1910_current table for python total_rows and node Views samples.
depends_on: []
blocks: [unblock-03-bigframes-gate]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: ~0.5 day
isProject: true
todos:
  - id: expand-tx-rows
    content: Add ≥97 synthetic TX rows to usa_1910_2013 in bigquery-public-data.yaml
    status: pending
  - id: usa-1910-current
    content: Add usa_1910_current table (duplicate schema+rows; no view machinery)
    status: pending
  - id: registry-docs
    content: Update SeededPublicTables, publicdata_test.go, testdata/public-data/README.md
    status: pending
  - id: gate-python
    content: "Gate: docs/snippets.py::test_client_query_total_rows passes after THIRDPARTY_FRESH_VOLUME=1 rebuild"
    status: pending
  - id: status-commit
    content: Update orchestration-status.md row; lint + commit
    status: pending
---

# Unblock 02 — Public-data seed expand

## Goal

Green **python** `test_client_query_total_rows` and unblock **node Views** tests that reference `bigquery-public-data.usa_names.usa_1910_current`.

## Log signature

```
AssertionError: assert 'Got 100 rows.' in 'Got 3 rows.\n'
```

```
Table not found: `bigquery-public-data.usa_names.usa_1910_current`
```

## Current state

[`testdata/public-data/bigquery-public-data.yaml`](../../testdata/public-data/bigquery-public-data.yaml):

- `usa_1910_2013`: 5 rows, **3 with `state: TX`**
- No `usa_1910_current`

[`gateway/seedfile/publicdata.go`](../../gateway/seedfile/publicdata.go) `SeededPublicTables` lists only `usa_1910_2013`.

## Implementation

### 1. Expand TX rows

Under `usa_names.tables[usa_1910_2013].rows`, add ≥97 rows with `state: TX` (vary `name`, `year`, `gender`, `number`). Synthetic data is fine — samples only assert row **count** for TX + LIMIT 100.

### 2. Add `usa_1910_current`

Duplicate table entry (same schema + same or superset rows). [`gateway/seedfile/apply.go`](../../gateway/seedfile/apply.go) only supports physical tables today — no YAML view type needed.

### 3. Registry updates

- [`gateway/seedfile/publicdata.go`](../../gateway/seedfile/publicdata.go): add `bigquery-public-data.usa_names.usa_1910_current`
- [`gateway/seedfile/publicdata_test.go`](../../gateway/seedfile/publicdata_test.go): `wantTables["usa_names"]` → 2
- [`testdata/public-data/README.md`](../../testdata/public-data/README.md): document both tables

### 4. Fresh volume / image

[`gateway/seedfile/apply.go`](../../gateway/seedfile/apply.go) skips row insert when table already exists. Require:

```bash
THIRDPARTY_FRESH_VOLUME=1 THIRDPARTY_REBUILD=1 task thirdparty:emulator-up
```

Or rebuild Docker image so new YAML is copied to `/opt/bigquery-emulator/testdata/public-data/`.

## Fast gate

```bash
go test ./gateway/seedfile/... -count=1
PYTHON_SAMPLES_PYTEST_ARGS='docs/snippets.py::test_client_query_total_rows -v' task thirdparty:python-bigquery-tests
```

## Expected delta

| Suite | Change |
|-------|--------|
| python snippets | 1 fail → 0 |
| node Views | −2 failures |

## Out of scope

- Full public-dataset replica
- View DDL support in seed loader (use duplicate table)
- BigFrames gate (plan 03)

## Done criteria

- [ ] `PublicDataRefsFullySeeded` covers both usa_names tables
- [ ] `test_client_query_total_rows` prints `Got 100 rows.`
- [ ] `tables.get` for `usa_1910_current` returns schema + rows
