---
name: Thirdparty 11 — BigFrames snippet gate
overview: Get the 4 python-bigquery-dataframes-snippet-gate tests passing after harness (01) and upstream emulator features land.
depends_on: [thirdparty-01-harness, thirdparty-02-gateway-query-metadata, thirdparty-04-tp08-load-jobs]
blocks: []
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 2-3 days
isProject: true
todos:
  - id: confirm-harness
    content: Verify plan 01 — exactly 4 tests collected, fake-gcs up, polars fallback works
    status: pending
  - id: test-set-options
    content: Fix test_bigquery_dataframes_set_options failures
    status: pending
  - id: test-load-from-bq
    content: Fix test_bigquery_dataframes_load_data_from_bigquery
    status: pending
  - id: test-pandas-methods
    content: Fix test_bigquery_dataframes_pandas_methods
    status: pending
  - id: test-performance
    content: Fix test_performance_optimizations
    status: pending
  - id: gate-green
    content: task thirdparty:python-bigquery-dataframes-snippet-gate exits 0
    status: pending
---

# Thirdparty 11 — BigFrames snippet gate

## Goal

Green the **default** `task thirdparty` dataframes lane — only 4 non-ML smokes.

## Gate tests (allowlist)

From [`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) `python-bigquery-dataframes-snippet-gate`:

| Test | Sample concern |
|------|----------------|
| `test_bigquery_dataframes_set_options` | Session options / location |
| `test_bigquery_dataframes_load_data_from_bigquery` | `read_gbq` / table load |
| `test_bigquery_dataframes_pandas_methods` | pandas API parity |
| `test_performance_optimizations` | Session caching / optimization flags |

Documented in [`third_party/README.md`](../../third_party/README.md) lines 549–556.

## Prerequisites

| Plan | Why |
|------|-----|
| **01 Harness** | `-k` filter, fake-gcs, polars fixture — without these all 32 tests error at setup |
| **02 Gateway** | Query params, metadata if samples issue parameterized reads |
| **04 tp08 LOAD** | If `load_data_from_bigquery` triggers load-job code paths |

May also need `BIGQUERY_STORAGE_GRPC_ENDPOINT=localhost:9060` if bigframes uses storage reads — verify per test traceback.

## Workflow

### Step 1 — Confirm harness (plan 01 done)

```bash
task thirdparty:python-bigquery-dataframes-snippet-gate 2>&1 | rg 'collected|passed|failed|ERROR'
```

Expect: `4 passed` (or 4 failed with **real** assertion errors, not setup errors).

### Step 2 — Run tests individually

```bash
cd third_party/python-bigquery-dataframes-tests/samples/snippets
DATAFRAME_SAMPLES_PYTEST_ARGS='-k test_bigquery_dataframes_set_options -vv --tb=long' \
  task thirdparty:python-bigquery-dataframes-tests
```

Repeat per test name.

### Step 3 — Fix failures

Triage each failure:

- **Emulator/gateway gap** → fix in appropriate plan (02/04/08) or inline if trivial
- **bigframes client wiring** → [`third_party/python-bigquery-dataframes-tests/samples/snippets/conftest.py`](../../third_party/python-bigquery-dataframes-tests/samples/snippets/conftest.py) emulator client setup
- **Polars execution path** → ensure `bpd` fallback sufficient when Polars compiler unavailable

### Step 4 — Do not expand gate scope

The 15+ ML snippet tests (`*_model_test.py`, bqml, gemini) remain **out of gate** per ENGINE_POLICY. Do not add them to `DATAFRAME_SAMPLES_PYTEST_ARGS` until CREATE MODEL parity exists.

## Verification

```bash
task thirdparty:python-bigquery-dataframes-snippet-gate
task thirdparty   # full aggregator includes this suite
```

## Out of scope

- Full `python-bigquery-dataframes-tests` (32 snippets)
- ML / Gemini / ONNX snippets

## Done when

- [ ] Snippet gate: 4 passed, 0 errors
- [ ] No GCS 403 teardown
- [ ] `third_party/README.md` gate section notes any remaining bigframes-specific env requirements
