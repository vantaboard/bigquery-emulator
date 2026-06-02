---
name: python-bigquery-tests fake-gcs wiring
overview: "Mirror the node-bigquery-tests fake-gcs pattern in the python-bigquery-tests task so GCS-touching snippets stop falling through to real GCS. Adds the fake-gcs-up dep, the STORAGE_EMULATOR_HOST export, and a python-side preflight."
todos:
  - id: tp03_dep
    content: "Add `- task: fake-gcs-up` to python-bigquery-tests deps right after `- task: emulator-up`."
    status: pending
  - id: tp03_env
    content: "Export STORAGE_EMULATOR_HOST with the same http:// normalization used in the node task."
    status: pending
  - id: tp03_preflight
    content: "Reuse scripts/preflight_node_samples_gcs.sh (or add a python-side preflight) before the nox invocation."
    status: pending
  - id: tp03_verify
    content: "Confirm test_extract_table / test_extract_table_json / test_extract_table_compressed no longer get 403 against real GCS."
    status: pending
  - id: tp03_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp03` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 03 — wire python-bigquery-tests through fake-gcs

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 2.1 (now deleted).
- Failing log lines `.logs/thirdparty-20260602-134739.log:1753, 1910, 2068`:
  `test_extract_table`, `test_extract_table_json`,
  `test_extract_table_compressed` fail with
  `403 POST https://storage.googleapis.com/storage/v1/b?project=dev`.

## Prerequisites

- [`thirdparty-02-fake-gcs-readiness.plan.md`](./thirdparty-02-fake-gcs-readiness.plan.md)
  — once `fake-gcs-up` polls for readiness, depending on it from this
  task is race-free.

## Scope

The python task ([`taskfiles/thirdparty.yml:372`](../../taskfiles/thirdparty.yml))
never depends on `fake-gcs-up` and never exports
`STORAGE_EMULATOR_HOST`, so `google-cloud-storage` inside the python
samples falls through to real GCS. Three extract-related snippets
fail with 403. Mirror what `node-bigquery-tests` already does at
[`taskfiles/thirdparty.yml:594-627`](../../taskfiles/thirdparty.yml).

## Implementation

1. **Task dep.** Add `- task: fake-gcs-up` to the
   `python-bigquery-tests:` task dependency list (right after
   `- task: emulator-up`).
2. **Env export.** In the inline step that exports
   `BIGQUERY_EMULATOR_HOST`, also export `STORAGE_EMULATOR_HOST`.
   Reuse the same `http://` normalization expression from the node
   task so the URL shape matches what `google-cloud-storage` expects.
3. **Preflight.** Run `scripts/preflight_node_samples_gcs.sh` (or
   add a `scripts/preflight_python_samples_gcs.sh` if the bucket /
   object list differs) before the `nox` invocation, so a missing
   fake-gcs seed fails fast with a precise message instead of a
   surprise 404 deep inside `pytest`.

## Tests

- `task thirdparty:python-bigquery-tests` runs the three
  `test_extract_table*` snippets against fake-gcs; the 403 against
  `storage.googleapis.com` disappears.
- The three pre-existing `bigquery-public-data` deselects (from
  plan 07) remain unchanged — that work is orthogonal.
- node-bigquery-tests still passes (no regression to the shared
  fake-gcs setup).

## Done criteria

- `python-bigquery-tests` depends on `fake-gcs-up`, exports
  `STORAGE_EMULATOR_HOST`, and gates on a preflight.
- The three extract tests stop failing on the GCS 403.
- `thirdparty-00-completion-index.plan.md` todo `tp03` flipped to
  `completed`.
