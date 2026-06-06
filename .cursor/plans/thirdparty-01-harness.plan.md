---
name: Thirdparty 01 — Harness fixes
overview: Fix thirdparty task orchestration and test-harness bugs so java, dataframes, and node suites report honest verdicts. No emulator feature work.
depends_on: []
blocks: [thirdparty-02-gateway-query-metadata, thirdparty-10-storage-grpc, thirdparty-11-bigframes-gate]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 1-2 days
isProject: true
todos:
  - id: java-allowlist-bash
    content: Extract _java_bq_failures_allowlisted to scripts/java_bq_check_allowlisted_failures.sh; invoke via bash from taskfiles/thirdparty.yml
    status: pending
  - id: dataframes-k-quoting
    content: Fix DATAFRAME_SAMPLES_PYTEST_ARGS word-splitting so -k filter reaches pytest intact
    status: pending
  - id: dataframes-fake-gcs
    content: Default STORAGE_EMULATOR_HOST and call fake-gcs-up in snippet-gate task
    status: pending
  - id: dataframes-polars
    content: Catch Exception in polars_session_or_bpd; align polars pin in nox constraints
    status: pending
  - id: node-mocha-parallel
    content: Disable Mocha --parallel when BIGQUERY_EMULATOR_HOST is set
    status: pending
---

# Thirdparty 01 — Harness fixes

## Goal

Unblock accurate thirdparty signal. Three suites currently fail for **infrastructure reasons**, not missing emulator features.

## 1a. Java allowlist Task shell panic

### Symptom

`task thirdparty:java-bigquery-tests` panics after `bigquerystorage` Maven verify:

```
panic: regexp: Compile(...): missing closing ]: `[^[:space:]\].*)$'`
```

`java-bigquery/samples/snippets` already **BUILD SUCCESS**; panic occurs in `_java_bq_failures_allowlisted` trim logic under Task's mvdan/sh interpreter.

### Root cause

[`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) lines ~858–895 use bash-specific parameter expansion (`${allow_raw//[[:space:],]/}`, `${a%%[![:space:]]*}`) unsafe in mvdan/sh.

### Implementation

1. Create [`scripts/java_bq_check_allowlisted_failures.sh`](../../scripts/java_bq_check_allowlisted_failures.sh):
   - Args: `$1=target_dir`, `$2=allowlist_csv`
   - Parse `target/failsafe-reports/TEST-*.xml` for failing IT short names
   - Exit 0 if all failures ⊆ allowlist; exit 1 otherwise
   - Use POSIX `tr -d '[:space:],'` for allowlist normalization
2. Replace inline function in `thirdparty.yml` java loop with:
   ```bash
   bash "${root}/scripts/java_bq_check_allowlisted_failures.sh" "$target" "${JAVA_BQ_ALLOW_FAILING_ITS:-...}"
   ```

### Verify

```bash
task thirdparty:java-bigquery-tests
```

Expect: 4-module fan-out completes; `java-bigquery` passes; storage/connection/DTS report allowed failures only (not panic).

---

## 1b. Dataframes snippet-gate harness

### Symptoms (log)

- 32 tests collected despite `-k` filter for 4 tests
- GCS teardown 403 on real `storage.googleapis.com`
- `AttributeError: PolarsCompiler` / `polars has no attribute lit`

### Fixes

| Bug | File | Fix |
|-----|------|-----|
| `-k` word-split | [`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) `python-bigquery-dataframes-tests` | Pass pytest args quoted: `bash -c 'uvx ... nox -s py -p "$1" -- "$2"' _ "$py" "$_df_pytest_extra"` or dedicated wrapper script |
| No fake-gcs | `python-bigquery-dataframes-snippet-gate` task | Always `task: fake-gcs-up`; export `STORAGE_EMULATOR_HOST=http://127.0.0.1:4443` (mirror `python-bigquery-tests`) |
| Polars fixture | [`third_party/python-bigquery-dataframes-tests/conftest.py`](../../third_party/python-bigquery-dataframes-tests/conftest.py) | `except Exception:` (not only `ImportError`) → fall back to `bpd` |
| Polars pin | synced `testing/constraints-*.txt` / nox deps | Pin polars version compatible with vendored bigframes |

### Verify

```bash
task thirdparty:python-bigquery-dataframes-snippet-gate
```

Expect: **4 tests** collected/run (not 32); no GCS 403 teardown; no session-setup PolarsCompiler errors.

---

## 1c. Node Mocha `--parallel` breaks emulator skips

### Symptom

Models tests ran and failed (log items 3–8) despite [`third_party/node-bigquery-tests/test/setup.js`](../../third_party/node-bigquery-tests/test/setup.js) intending to skip `models.test.js` via stack inspection.

### Root cause

[`package.json`](../../third_party/node-bigquery-tests/package.json): `"test": "mocha --timeout 200000 --parallel --require ./test/setup.js"`. Parallel workers break stack-based `describe` wrapping.

### Fix (pick one)

- **Preferred:** In `taskfiles/thirdparty.yml` `node-bigquery-tests`, run:
  ```bash
  npx mocha --timeout 200000 --no-parallel --require ./test/setup.js
  ```
  instead of `npm test` when `BIGQUERY_EMULATOR_HOST` is set.
- **Alternative:** Patch `setup.js` to use explicit `describe.skip` in `models.test.js` / `jobs.test.js` headers (less fragile).

### Verify

```bash
task thirdparty:node-bigquery-tests 2>&1 | rg -i 'Models|should retrieve a model'
```

Expect: Models suite skipped (pending), not failing.

---

## Out of scope

- tp08 load/copy/extract implementation
- Gateway query/metadata fixes (plan 02)
- Polars **compiler** feature work beyond pin + fixture fallback

## Done when

- [ ] `task thirdparty:java-bigquery-tests` exits 0 (with allowlisted storage/connection/DTS failures)
- [ ] `task thirdparty:python-bigquery-dataframes-snippet-gate` collects 4 tests, no harness errors
- [ ] Node models suite skipped under emulator
